# OurTorrent

OurTorrent is a peer-to-peer (P2P) file-sharing system inspired by the BitTorrent protocol. This decentralized system allows users to download files more quickly by connecting with multiple uploaders, rather than relying on a single central server. OurTorrent aims to simulate the peer-to-peer file-sharing experience, including a custom client and a lightweight UDP tracker for efficient, low-overhead communication.

## Key Features

1. **UDP Tracker**
2. **Asynchronous Socket Handling**
3. **Bencoding Serialization Library**
4. **Error Handling**
5. **Security**

## Project Phases

1. **Phase 1**: 
   - Built a client compatible with the BitTorrent protocol that can download files using torrent files.
   - Our client connects with general internet-based trackers to join peer groups and download any specified file.

2. **Phase 2**:
   - Developed our own UDP tracker for a deeper understanding of the protocol.
   - The tracker manages essential attributes for each peer, including peer IDs, seeders, and leechers, and shares this information with clients upon request.

## Tech Stack and Libraries

1. **C++ for Tracker**
2. **Python for Peer**

## Tools and Technologies Used
   - Sqlite3 for database
   - POSIX socket libraries
   - OpenSSL for SHA1 hash
   - Rich package for table formatting
   - urrlib3 library for HTTP connections

## Installation

1. **Clone the repository**:
   ```bash
   git clone https://github.com/Project-Group-Computer-Networks/OurTorrent.git
   cd OurTorrent
   ```
   
