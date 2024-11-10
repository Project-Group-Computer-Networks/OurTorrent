To install OpenSSL libraries in Linux :

sudo apt-get install libssl-dev

To run the directory code:

1) g++ -o client main.cpp announce_request.cpp connect_request.cpp peer_decoder.cpp -std=c++20 -lssl -lcrypto

2) ./client
