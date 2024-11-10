#ifndef TORRENT_FILE_H
#define TORRENT_FILE_H
#include <bits/stdc++.h>
#include <fstream>

#include <arpa/inet.h>
// #include <openssl/sha.h> // For generating info hash
#include <sys/socket.h>  // For socket functions
#include <unistd.h>      // For close




extern uint64_t downloaded,uploaded,left_length;
extern uint32_t ip_address,event,key;
extern uint16_t port,num_want; // Listening port (convert to network byte order)
extern uint8_t peer_id[20],info_hash[20];  // 16-byte buffer to store truncated peer ID

extern std::string peer_id_str;

extern uint64_t CONNECTION_ID; // Example connection ID
extern uint32_t TRANSACTION_ID; // Generate a random transaction ID
extern uint32_t ACTION_ANNOUNCE;



#endif