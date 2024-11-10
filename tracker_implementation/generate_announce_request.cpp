#include <iostream>
#include <cstring>
#include <vector>
#include <arpa/inet.h> // For inet_addr

// Define htonll if not available
#ifndef htonll
uint64_t htonll(uint64_t value) {
    static const int num = 42;
    // Check the endianness
    if (*(const char *)&num == num) {
        const uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
        const uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFLL));
        return (static_cast<uint64_t>(low_part) << 32) | high_part;
    } else {
        return value;
    }
}
#endif
#include <cstdint>     // For fixed-width integer types like uint32_t, uint16_t

// Function to create an announce request
std::vector<uint8_t> createAnnounceRequest() {
    std::vector<uint8_t> request;

    // 8-byte Connection ID (for example, using a known constant)
    uint64_t connection_id = htonll(0x41727101980); // For a real tracker
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&connection_id), 
                   reinterpret_cast<uint8_t*>(&connection_id) + sizeof(connection_id));

    // 4-byte Action (1 for announce)
    uint32_t action = htonl(1);
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&action), 
                   reinterpret_cast<uint8_t*>(&action) + sizeof(action));

    // 4-byte Transaction ID (example value)
    uint32_t transaction_id = htonl(123456); // Arbitrary transaction ID
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&transaction_id), 
                   reinterpret_cast<uint8_t*>(&transaction_id) + sizeof(transaction_id));

    // 20-byte Info Hash (example hash)
    uint8_t info_hash[20] = {0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f, 0x70, 0x81,
                             0x92, 0xa3, 0xb4, 0xc5, 0xd6, 0xe7, 0xf8, 0x09,
                             0x1a, 0x2b, 0x3c, 0x4d};
    request.insert(request.end(), info_hash, info_hash + sizeof(info_hash));

    // 20-byte Peer ID (example peer ID)
    uint8_t peer_id[20] = {'-', 'T', 'e', 's', 't', 'P', 'e', 'e', 'r', '-', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
    request.insert(request.end(), peer_id, peer_id + sizeof(peer_id));

    // 8-byte Downloaded (initially 0)
    uint64_t downloaded = 0;
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&downloaded), 
                   reinterpret_cast<uint8_t*>(&downloaded) + sizeof(downloaded));

    // 8-byte Left (e.g., total size of the torrent in bytes, for example, 1GB)
    uint64_t left = htonll(1073741824); // 1GB in bytes
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&left), 
                   reinterpret_cast<uint8_t*>(&left) + sizeof(left));

    // 8-byte Uploaded (initially 0)
    uint64_t uploaded = 0;
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&uploaded), 
                   reinterpret_cast<uint8_t*>(&uploaded) + sizeof(uploaded));

    // 4-byte Event (0 = none)
    uint32_t event = htonl(0);
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&event), 
                   reinterpret_cast<uint8_t*>(&event) + sizeof(event));

    // 4-byte IP Address (for example, 192.168.0.1)
    uint32_t ip_address = inet_addr("192.168.0.1"); // Convert IP string to binary
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&ip_address), 
                   reinterpret_cast<uint8_t*>(&ip_address) + sizeof(ip_address));

    // 4-byte Key (example value)
    uint32_t key = htonl(0);
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&key), 
                   reinterpret_cast<uint8_t*>(&key) + sizeof(key));

    // 4-byte Num Want (0 for no specific limit)
    uint32_t num_want = htonl(0);
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&num_want), 
                   reinterpret_cast<uint8_t*>(&num_want) + sizeof(num_want));

    // 2-byte Port (e.g., 6881)
    uint16_t port = htons(6881);
    request.insert(request.end(), reinterpret_cast<uint8_t*>(&port), 
                   reinterpret_cast<uint8_t*>(&port) + sizeof(port));

    return request;
}

// Example usage
int main() {
    std::vector<uint8_t> announce_request = createAnnounceRequest();

    // For testing: print the byte values in hex
    for (const auto& byte : announce_request) {
        printf("%02x ", byte);
    }
    std::cout << std::endl;

    return 0;
}
