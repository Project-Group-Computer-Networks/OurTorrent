#include "request.h"

using namespace std;

uint64_t htonll(uint64_t hostlonglong) {
    return ((uint64_t)htonl(hostlonglong & 0xFFFFFFFF) << 32) | htonl(hostlonglong >> 32);
}


// Convert from network byte order
uint64_t ntohll(uint64_t netlonglong) {
    return ((uint64_t)ntohl(netlonglong & 0xFFFFFFFF) << 32) | ntohl(netlonglong >> 32);
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


// Function to send a connect request to the tracker
int send_connect_request(int sockfd, struct addrinfo *p) {
    uint8_t buf[16];
    uint64_t protocol_id = 0x41727101980; // magic constant for the connect request
    uint32_t action = 0; // connect action
    
    transaction_id=rand();
    std::cout<<transaction_id<<endl;
    // Pack the request
    memcpy(buf, &protocol_id, sizeof(protocol_id));
    memcpy(buf + 8, &action, sizeof(action));
    memcpy(buf + 12, &transaction_id, sizeof(transaction_id));

    // Send the connect request
    int numbytes = sendto(sockfd, buf, sizeof(buf), 0, p->ai_addr, p->ai_addrlen);
    if (numbytes == -1) {
        perror("sendto");
        return -1;
    }

    printf("Sent connect request to tracker.\n");
    return transaction_id; // Return the transaction_id to validate the response later
}
