#include "peer_manager.h"
#include "db_manager.h"
#include <sqlite3.h>

sqlite3 *db = nullptr;

std::vector<Peer> retrievePeersFromDatabase(const std::string &infoHash)
{
    std::vector<Peer> peers;

    const char *selectSQL = "SELECT peer_id, ip, port FROM peers WHERE info_hash = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return peers;
    }

    sqlite3_bind_text(stmt, 1, infoHash.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Peer peer;
        peer.peer_id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        peer.ip = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        peer.port = sqlite3_column_int(stmt, 2);
        peers.push_back(peer);
    }

    sqlite3_finalize(stmt);
    return peers;
}

void initializeDatabase()
{
    if (sqlite3_open("tracker.db", &db))
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        exit(1);
    }

    const char *createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS peers (
            info_hash TEXT NOT NULL,
            peer_id TEXT NOT NULL,
            ip TEXT NOT NULL,
            port INTEGER NOT NULL,
            uploaded INTEGER,
            downloaded INTEGER,
            left INTEGER
        );
    )";

    char *errMsg = 0;
    if (sqlite3_exec(db, createTableSQL, NULL, 0, &errMsg))
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

void countLeechersAndSeeders(const std::string &infoHash, int &leechers, int &seeders)
{
    const char *countSQL = "SELECT CASE WHEN left = 0 THEN 'seeder' ELSE 'leecher' END, COUNT(*) FROM peers WHERE info_hash = ? GROUP BY CASE WHEN left = 0 THEN 'seeder' ELSE 'leecher' END";
    sqlite3_stmt *stmt;

    leechers = 0;
    seeders = 0;

    if (sqlite3_prepare_v2(db, countSQL, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, infoHash.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const unsigned char *type = sqlite3_column_text(stmt, 0);
        int count = sqlite3_column_int(stmt, 1);

        if (strcmp(reinterpret_cast<const char *>(type), "seeder") == 0)
        {
            seeders = count;
        }
        else
        {
            leechers = count;
        }
    }

    sqlite3_finalize(stmt);
}
void storePeerInDatabase(const std::string &infoHash, const std::string &peerId, const std::string &ip, int port, int uploaded, int downloaded, int left)
{
    const char *insertSQL = "INSERT INTO peers (info_hash, peer_id, ip, port, uploaded, downloaded, left) VALUES (?, ?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;

    // Insert data into the database
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    sqlite3_bind_text(stmt, 1, infoHash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, peerId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, ip.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, port);
    sqlite3_bind_int(stmt, 5, uploaded);
    sqlite3_bind_int(stmt, 6, downloaded);
    sqlite3_bind_int(stmt, 7, left);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        std::cerr << "Failed to insert data: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    // Now print all rows for debugging
    const char *selectSQL = "SELECT * FROM peers";
    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare SELECT statement: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::cout << "Database content after insertion:" << std::endl;
    std::cout << "info_hash\tpeer_id\t\tip\t\tport\tuploaded\tdownloaded\tleft" << std::endl;

    // Loop through all the rows in the table
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string info_hash = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        std::string peer_id = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        std::string ip = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        int port = sqlite3_column_int(stmt, 3);
        int uploaded = sqlite3_column_int(stmt, 4);
        int downloaded = sqlite3_column_int(stmt, 5);
        int left = sqlite3_column_int(stmt, 6);

        std::cout << info_hash << "\t" << peer_id << "\t" << ip << "\t" << port << "\t"
                  << uploaded << "\t" << downloaded << "\t" << left << std::endl;
    }

    sqlite3_finalize(stmt);
}
