#include "request.h"
#include <vector>
// Function to decode the response into a vector of Peer
std::vector<Peer> decodeResponse(const std::vector<uint8_t> &response) {
    std::vector<Peer> peers;

    // Announce response header is 20 bytes (skip it):
    // 4 bytes action, 4 bytes transaction_id, 4 bytes interval, 4 bytes leechers, 4 bytes seeders
    size_t offset = 20;

    // Each peer entry is 6 bytes: 4 bytes for IP and 2 bytes for port
    while (offset + 6 <= response.size()) {
        Peer peer;

        // Extract IP (4 bytes)
        uint32_t ip_net;
        memcpy(&ip_net, &response[offset], 4);
        offset += 4;

        // Convert IP from network byte order to human-readable format
        struct in_addr ip_addr;
        ip_addr.s_addr = ip_net;
        peer.ip = inet_ntoa(ip_addr); // Convert to string

        // Extract Port (2 bytes)
        uint16_t port_net;
        memcpy(&port_net, &response[offset], 2);
        offset += 2;

        // Convert port from network byte order to host byte order
        peer.port = ntohs(port_net);

        // Add peer to the list (peer_id is not part of the response, so leave it empty)
        peer.peer_id = "";  // peer_id not present in the response
        peers.push_back(peer);
    }

    return peers;
}

// Debug function to print the decoded peers
void printPeers(const std::vector<Peer> &peers) {
    for (const auto &peer : peers) {
        printf("IP: %s, Port: %d\n", peer.ip.c_str(), peer.port);
    }
}
