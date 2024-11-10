#include "request.h"
#include "torrent_file.h"

using namespace std;

int send_announce_request(int sockfd, struct addrinfo *p){
    uint8_t announce_buffer[104];

    uint32_t announce_action = htonl(1); // connect action
    
    // Pack the request
    memcpy(announce_buffer, &connection_id, sizeof(connection_id));
    memcpy(announce_buffer + 8, &announce_action, sizeof(announce_action));
    memcpy(announce_buffer + 12, &transaction_id, sizeof(transaction_id));
    memcpy(announce_buffer + 16, &info_hash, sizeof(info_hash));
    memcpy(announce_buffer + 36, &peer_id, sizeof(peer_id));
    memcpy(announce_buffer + 56, &downloaded, sizeof(downloaded));
    memcpy(announce_buffer + 64, &left_length, sizeof(left_length));
    memcpy(announce_buffer + 72, &uploaded, sizeof(uploaded));
    memcpy(announce_buffer + 80, &event, sizeof(event));
    memcpy(announce_buffer + 84, &ip_address, sizeof(ip_address));
    memcpy(announce_buffer + 88, &key, sizeof(key));
    memcpy(announce_buffer + 92, &num_want, sizeof(num_want));
    memcpy(announce_buffer + 96, &port, sizeof(port));
    


    // Send the connect request
    int numbytes = sendto(sockfd, announce_buffer, sizeof(announce_buffer), 0, p->ai_addr, p->ai_addrlen);
    if (numbytes == -1) {
        perror("sendto in announce request");
        return -1;
    }

    printf("Sent announce request to tracker.\n");
    return numbytes; // Return the transaction_id to validate the response later
}