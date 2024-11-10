#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H
#define PORT 6969
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdint>   
#include <cstdlib>  
#include <string.h>
#include <set>
#include <time.h>
sockaddr_in createIPv4Address(const char* ip,int port);
int createUDPIPv4Socket();
int createTCPIPv4Socket();
uint64_t htonll(uint64_t value) ;
uint64_t ntohll(uint64_t value);
#endif