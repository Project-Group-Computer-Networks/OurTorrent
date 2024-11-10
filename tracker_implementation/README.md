# Tracker Implementation

The implementation of a UDP tracker for a P2P file sharing system.

It is a UDP tracker implemented according to this BEP:  http://bittorrent.org/beps/bep_0015.html.

To run use :

1) g++ main.cpp db_manager.cpp peer_manager.cpp udp_server.cpp socket_util.cpp -l sqlite3 -o bittorrent_tracker

2) ./bittorrent_tracker

