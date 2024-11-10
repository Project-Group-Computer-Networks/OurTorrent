#include "socket_util.h"

int createUDPIPv4Socket()
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    return s;
}
int createTCPIPv4Socket()
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    return s;
}
sockaddr_in createIPv4Address(const char *ip, int port)
{
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (strlen(ip) == 0)
    {
        address.sin_addr.s_addr = INADDR_ANY;
    }
    else
    {
        inet_pton(AF_INET, ip, &address.sin_addr);
    }

    return address;
}

/* uint64_t ntohll(uint64_t value)
{
    if (htonl(1) != 1)
    { // Check if the system is big-endian
        return value;
    }
    return ((uint64_t)ntohl(value & 0xFFFFFFFF) << 32) | ntohl(value >> 32);
} */
uint64_t ntohll(uint64_t value)
{
    // If the system is little-endian, swap byte order
    // If the system is big-endian, do nothing (network order is already big-endian)
    static const int num = 42;
    if (*(const char *)&num == 42)
    {
        // System is little-endian, swap bytes
        return ((uint64_t)ntohl(value & 0xFFFFFFFF) << 32) | ntohl(value >> 32);
    }
    else
    {
        // System is big-endian, no need to swap
        return value;
    }
}
uint64_t htonll(uint64_t value)
{
    static const int num = 42;
    // Check the endianness
    if (*(const char *)&num == num)
    {
        const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
        const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));
        return (static_cast<uint64_t>(low_part) << 32) | high_part;
    }
    else
    {
        return value;
    }
}