#ifndef REQUEST_H
#define REQUEST_H

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
#include <iostream>

#include <string>

#define TRACKER_PORT "6969"  // port of the UDP tracker
#define MAXBUFLEN 1024

extern uint64_t connection_id;
extern uint32_t transaction_id; // random transaction ID

int send_announce_request(int sockfd, struct addrinfo *p);
int send_connect_request(int sockfd, struct addrinfo *p);
uint64_t htonll(uint64_t hostlonglong);
uint64_t ntohll(uint64_t netlonglong);
void *get_in_addr(struct sockaddr *sa);

struct Peer
{
    std::string ip;      // Peer IP address
    int port;            // Peer port number
    std::string peer_id; // Peer ID
};

#endif