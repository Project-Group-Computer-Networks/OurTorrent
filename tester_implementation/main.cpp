#include <vector>
#include <cstdlib> // For rand()
#include "peer_decoder.h"
#include "torrent_file.h"
#include "request.h"
#include "custom_bencoder.h"
#include <openssl/sha.h> // Include OpenSSL SHA header
using namespace std;

// Define global variables
uint64_t connection_id;
uint32_t transaction_id;
uint64_t downloaded = 0, uploaded = 0, left_length = 0;
uint32_t ip_address = 0, event = 0, key = rand(); // Random key generation
uint16_t port = htons(6881);                      // Set port to 6881 and convert to network byte order
uint16_t num_want = 0;
uint8_t peer_id[20];                              // Buffer for 20-byte peer ID
uint8_t info_hash[20];                            // Buffer for the info hash
std::string peer_id_str = "0MjRoxR3KuzXecjOVAF2"; // Example peer ID
uint64_t CONNECTION_ID = 0x41727101980LL;         // Example connection ID
uint32_t TRANSACTION_ID = rand();                 // Generate a random transaction ID
uint32_t ACTION_ANNOUNCE = 1;
const char *ip_addr;
std::string hostname;

void decode_torrent_file()
{

    ifstream file("puppy.torrent", ios::binary);
    if (!file)
    {
        cerr << "Error opening .torrent file!" << endl;
        return;
    }

    // Read the entire torrent file
    string bencode((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    bencoding::string_subs sa(bencode);
    bencoding::bencode_dict torrent_dict;
    torrent_dict.decode(sa.str, sa.citer);

    // Extract info dictionary
    bencoding::bencode_dict file_info_dict;
    bencoding::string_subs snew(torrent_dict["info"]->encode());
    file_info_dict.decode(snew.str, snew.citer);

    // Generate SHA-1 hash of the "info" dictionary for the info_hash
    SHA1(reinterpret_cast<const unsigned char *>(snew.str.data()), snew.str.size(), info_hash);

    // Prepare announce request parameters
    downloaded = htonll(0);

    left_length = htonll(stoull(file_info_dict["length"]->get_as_str()));

    uploaded = htonll(0);
    struct in_addr addr;

    std::string url_string = torrent_dict["announce"]->get_as_str();

    hostname = "0.0.0.0";    
    ip_addr = hostname.c_str(); // Now ip_addr contains only the hostname

    inet_aton(ip_addr, &addr); // Default (0) to let the tracker detect
    ip_address = htonl(addr.s_addr);
    event = htonl(0);
    key = htonl(rand());
    num_want = htons(0);

    port = htons(6881); // Listening port (convert to network byte order)

    peer_id_str = "0MjRoxR3KuzXecjOVAF2"; // 20-byte peer ID

    // Truncate the peer_id_str to fit into the 16-byte peer_id array
    memcpy(peer_id, peer_id_str.data(), 16);
}

int main()
{
    decode_torrent_file();

    // Initialize socket and address info
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
    hints.ai_family = AF_INET;      // Use IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    if ((rv = getaddrinfo(ip_addr, TRACKER_PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop through all results and create socket
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "failed to create socket\n");
        return 2;
    }

    // Send connect request to the tracker
    int connect_transaction_id = send_connect_request(sockfd, p);
    if (connect_transaction_id == -1)
    {
        fprintf(stderr, "failed to send connect request\n");
        return 3;
    }

    addr_len = sizeof their_addr;

    // Receive response from the tracker
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    printf("Got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    printf("Packet is %d bytes long\n", numbytes);

    uint32_t response_action, response_transaction_id;
    memcpy(&response_action, buf, 4);
    memcpy(&response_transaction_id, buf + 4, 4);

    // Validate the transaction ID matches
    if (response_transaction_id != transaction_id)
    {
        fprintf(stderr, "Transaction ID mismatch\n");
        return 5;
    }

    if (response_action == 0)
    { // Connect response
        memcpy(&connection_id, buf + 8, 8);
        connection_id = ntohll(connection_id); // Receive in host order

        printf("Received valid connection ID: %llu\n", (unsigned long long)connection_id);
    }
    else
    {
        fprintf(stderr, "Received unexpected action: %u\n", response_action);
        return 6;
    }

    // Announce request
    int announce_transaction_id = send_announce_request(sockfd, p);
    if (announce_transaction_id == -1)
    {
        fprintf(stderr, "failed to send announce request\n");
        return 7;
    }

    addr_len = sizeof their_addr;

    // Receive announce response from the tracker
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    printf("Got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    printf("Packet is %d bytes long\n", numbytes);

    // Decode the response to extract peer information
    std::vector<uint8_t> response(buf, buf + numbytes);
    std::vector<Peer> peers = decodeResponse(response);

    // Print the decoded peer information
    printPeers(peers);

    // Clean up
    freeaddrinfo(servinfo);
    if (close(sockfd) == -1)
    {
        perror("close");
    }

    return 0;
}