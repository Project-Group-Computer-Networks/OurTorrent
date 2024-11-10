#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdint.h>

using namespace std;

#define TRACKER_PORT "6969"  // port of the UDP tracker
#define MAXBUFLEN 100
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
    uint64_t connection_id = 0x41727101980; // magic constant for the connect request
    uint32_t action = 0; // connect action
    uint32_t transaction_id = rand(); // random transaction ID

    // Pack the request
    memcpy(buf, &connection_id, sizeof(connection_id));
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

int main(void) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    // Set up the hints for the tracker address
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // Use IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    const char* hostname = "localhost"; // hostname
    const char* port = "6969"; // port
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop through all results and create socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "failed to create socket\n");
        return 2;
    }

    // Send connect request to the tracker
    int transaction_id = send_connect_request(sockfd, p);
    if (transaction_id == -1) {
        fprintf(stderr, "failed to send connect request\n");
        return 3;
    }

    addr_len = sizeof their_addr;

    // Receive response from the tracker
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    printf("Got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    printf("Packet is %d bytes long\n", numbytes);

    // Unpack response
    if (numbytes < 16) {
        fprintf(stderr, "Invalid response length\n");
        return 4;
    }

    uint32_t response_action, response_transaction_id;
    memcpy(&response_action, buf, 4);
    memcpy(&response_transaction_id, buf + 4, 4);

    // Validate the transaction ID matches
    if (response_transaction_id != transaction_id) {
        fprintf(stderr, "Transaction ID mismatch\n");
        return 5;
    }

    if (response_action == 0) { // Connect response
        uint64_t connection_id;
        memcpy(&connection_id, buf + 8, 8);
        connection_id = ntohll(connection_id); // Receive in host order

        // printf("Received valid connection ID: %lx\n", connection_id);
        printf("Received valid connection ID: %llu\n", (unsigned long long)connection_id);

    } else {
        fprintf(stderr, "Received unexpected action: %u\n", response_action);
        return 6;
    }

    freeaddrinfo(servinfo);
    if (close(sockfd) == -1) {
        perror("close");
    }

    return 0;
}