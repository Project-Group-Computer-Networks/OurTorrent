#include "peer_manager.h"
#include "db_manager.h"

using namespace std;
set<uint64_t> valid_connection_ids;

uint64_t generateConnectionID()
{
    return ((uint64_t)rand() << 32) | rand();
}
void handleConnectRequest(char *buffer, int bytesRcvd, int listenSocket, sockaddr_in &clientAddr, socklen_t addrLen)
{
    if (bytesRcvd >= sizeof(ConnectRequest))
    {
        ConnectRequest *request = reinterpret_cast<ConnectRequest *>(buffer);

        if (request->protocol_id == PROTOCOL_ID && request->action == ACTION_CONNECT)
        {
            cout << "Received a valid connect request" << endl;

            uint64_t connection_id = generateConnectionID();
            valid_connection_ids.insert(connection_id);

            printf("Generated connnection id: %llu\n", (unsigned long long)connection_id);

            // cout << "Generated connection ID: " << connection_id << endl;

            ConnectResponse response;
            response.action = ACTION_RESPONSE_CONNECT;
            response.transaction_id = request->transaction_id;
            response.connection_id = htonll(connection_id); // Convert to network byte order

            int bytesSent = sendto(listenSocket, reinterpret_cast<char *>(&response), sizeof(response), 0, (sockaddr *)&clientAddr, addrLen);
            if (bytesSent <= 0)
            {
                cout << "Failed to send response" << endl;
            }
            else
            {
                cout << "Sent connect response to client" << endl;
            }
        }
        else
        {
            cout << "Invalid protocol ID or action in request" << endl;
        }
    }
    else
    {
        cout << "Invalid connect request received" << endl;
    }
}
void processAnnounceRequest(char *buffer, int length, int sockfd, sockaddr_in &clientAddr)
{
    try
    {
        cout << length << endl;
        cout << sizeof(AnnounceRequest) << endl;
        // Check if buffer length matches the expected size
        if (length < sizeof(AnnounceRequest))
        {
            throw std::runtime_error("Invalid announce request length");
        }

        AnnounceRequest req;
        memcpy(&req, buffer, sizeof(AnnounceRequest));

        /*************************************************************/
        // sample Annnouncement:<8-byte Connection ID><4-byte Action (1 for announce)><4-byte Transaction ID><20-byte Info_hash><20-byte Peer ID><8-byte Downloaded><8-byte Left><8-byte Uploaded><4-byte Event><4-byte IP address><4-byte Key><4-byte Num Want><2-byte Port>
        //  Convert network byte order to host byte order where necessary
        /*************************************************************/

        // req.connection_id = ntohll(req.connection_id);

        if (valid_connection_ids.find(req.connection_id) == valid_connection_ids.end())
        {
            cout << "Invalid connection ID\n";
            return;
        }
        uint32_t value_4;
        uint64_t value_8;

        req.action = 1;

        memcpy(&value_4, &buffer[12], sizeof(uint32_t));

        req.transaction_id = value_4;

        memcpy(&value_8, &buffer[56], sizeof(uint64_t));
        req.downloaded = ntohll(value_8); // Assuming you have a ntohll for 64-bit

        memcpy(&value_8, &buffer[64], sizeof(uint64_t));
        cout << value_8 << endl;
        req.left = ntohll(value_8);
        cout << ntohll(value_8) << endl;

        memcpy(&value_8, &buffer[72], sizeof(uint64_t));
        req.uploaded = ntohll(value_8);

        memcpy(&value_4, &buffer[80], sizeof(uint32_t));
        req.event = ntohl(value_4);

        memcpy(&value_4, &buffer[84], sizeof(uint32_t));
        req.ip_address = ntohl(value_4);
        struct in_addr addr;
        addr.s_addr = req.ip_address;
        cout << inet_ntoa(addr) << endl;

        memcpy(&value_4, &buffer[88], sizeof(uint32_t));
        req.key = ntohl(value_4);

        memcpy(&value_4, &buffer[92], sizeof(uint32_t));
        req.num_want = ntohl(value_4);

        uint16_t value_2;
        memcpy(&value_2, &buffer[96], sizeof(uint16_t));
        req.port = ntohs(value_2);

        cout << req.action << " " << req.transaction_id << " " << req.downloaded << " " << req.left << " " << req.uploaded << " " << req.event << " " << inet_ntoa(addr) << " " << req.key << " " << req.num_want << " " << req.port << endl;
        // cout<<value<<endl;
        // Extract info_hash and peer_id
        std::string info_hash(reinterpret_cast<char *>(req.info_hash), sizeof(req.info_hash));
        std::string peer_id(reinterpret_cast<char *>(req.peer_id), sizeof(req.peer_id));

        // Now use info_hash and peer_id for processing
        std::vector<Peer> peers = retrievePeersFromDatabase(info_hash);

        // Convert IP address from uint32_t to string format (e.g., "192.168.0.1")
        struct in_addr ip_addr;
        ip_addr.s_addr = htonl(req.ip_address);  // Convert back to network byte order for string conversion
        std::string ip_str = inet_ntoa(ip_addr); // Converts to "xxx.xxx.xxx.xxx"

        // now have to store the peer in database
        storePeerInDatabase(info_hash, peer_id, ip_str, req.port, req.uploaded, req.downloaded, req.left);

        /*************************************************************/
        // sample response:<4-byte Action (1 for announce)><4-byte Transaction ID><Interval><Leechers><Seeders><Peer List (IP:Port)>
        /*************************************************************/

        //  Create a binary response
        std::vector<uint8_t> response;

        int leechers_net = 0;
        int seeders_net = 0;
        countLeechersAndSeeders(info_hash, leechers_net, seeders_net);

        // cout << "Leechers: " << leechers_net << " Seeders: " << seeders_net << endl;

        // Add the 4-byte action field
        uint32_t action_response = htonl(1); // 1 = announce response
        // std::cout << "action_response type: " << action_response << " " << typeid(action_response).name() << std::endl; // Ignore debugging statement
        response.insert(response.end(), reinterpret_cast<uint8_t *>(&action_response), reinterpret_cast<uint8_t *>(&action_response) + sizeof(action_response));

        // Add the 4-byte transaction_id field
        uint32_t tran = htonl(req.transaction_id);
        response.insert(response.end(), reinterpret_cast<uint8_t *>(&tran), reinterpret_cast<uint8_t *>(&tran) + sizeof(tran));
        // std::cout << "transaction_id type: " << tran << " " << typeid(tran).name() << std::endl; // Ignore debugging statement
        // Add Interval, Leechers, Seeders (dummy values here)
        uint32_t interval = htonl(1800); // 1800 seconds for example
        uint32_t leechers = htonl(leechers_net);
        uint32_t seeders = htonl(seeders_net);
        // std::cout << "interval type: " << interval << " " << typeid(interval).name() << std::endl; // Ignore debugging statement
        response.insert(response.end(), reinterpret_cast<uint8_t *>(&interval), reinterpret_cast<uint8_t *>(&interval) + sizeof(interval));
        response.insert(response.end(), reinterpret_cast<uint8_t *>(&leechers), reinterpret_cast<uint8_t *>(&leechers) + sizeof(leechers));
        response.insert(response.end(), reinterpret_cast<uint8_t *>(&seeders), reinterpret_cast<uint8_t *>(&seeders) + sizeof(seeders));

        // Add peers in binary format (IP + Port)
        for (const auto &peer : peers)
        {
            uint32_t peer_ip = inet_addr(peer.ip.c_str()); // Convert IP to binary
            uint16_t peer_port = htons(peer.port);

            response.insert(response.end(), reinterpret_cast<uint8_t *>(&peer_ip), reinterpret_cast<uint8_t *>(&peer_ip) + sizeof(peer_ip));
            response.insert(response.end(), reinterpret_cast<uint8_t *>(&peer_port), reinterpret_cast<uint8_t *>(&peer_port) + sizeof(peer_port));
        }
        // debugging response
        for (size_t i = 0; i < response.size(); ++i)
        {
            // printf("%02x ", (unsigned char)response[i]);
            cout << response[i] << "";
        }
        printf("\n");
        // Send the response as a char array
        sendto(sockfd, reinterpret_cast<char *>(response.data()), response.size(), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error processing announce request: " << e.what() << std::endl;
    }
}