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

### Note: Use Ubuntu systems since the project uses POSIX library, unavailable on Windows

## Installation

1. **Clone and navigate to the repository**:
   ```bash
   git clone https://github.com/Project-Group-Computer-Networks/OurTorrent.git
   cd OurTorrent
   ```

2. **For running Tracker**:
    - Navigate to Tracker_Implementation
    - Install the following dependencies:
    ```bash
    sudo apt update
    sudo apt install sqlite3
    ```
    - Compile the executable file:
    ```bash
    g++ main.cpp db_manager.cpp peer_manager.cpp udp_server.cpp socket_util.cpp -l sqlite3 -o bittorrent_tracker
    ```
    - The file is ready for use; to run:
    ```bash
    ./bittorrent_tracker
    ```

3. **For running Client**:
    - Navigate to client_bittorent
    - Download all the pre-requisites present in /requirements.txt 
    ```bash
    pip install -r requirements.txt
    ```
    - Change directory to src and run the help command
    ```
    cd src
    python3 main.py --help
    ```
    - To test the code, run the client with a torrent file present in examples folder
    ```
    mkdir ../downloads
    python3 main.py ../examples/ubuntu.torrent -d ../downloads/
    ```

4. **For running Tester**:
    - Navigate to Tester_implementation
    - Install OpenSSL libraries in Linux :
    ```bash
    sudo apt-get install libssl-dev
    ```
    - To run the directory code:
    ```bash
    g++ -o client main.cpp announce_request.cpp connect_request.cpp peer_decoder.cpp -std=c++20 -lssl -lcrypto
    ./client
    ```
## Video Demo Link:


## Contributors:
- Meet Sindhav  
- Mohammed Haaziq
- Aditya Mundada
- Nayan Kakade
- Sarvesh Baheti
- Roopam Taneja 

## Made with ❤️ by team "OurTorrent"
