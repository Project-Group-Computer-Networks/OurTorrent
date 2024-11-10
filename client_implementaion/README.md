# OurTorrent

OurTorrent is our attempt at developing a bittorrent client, which enables users to download files over internet in a P2P manner.

## Installation and Build :hammer_and_wrench:

* Clone this repository on an Ubuntu machine and move to the working directory

```bash
    $ git clone https://github.com/Project-Group-Computer-Networks/client_bittorent.git
    $ cd ourtorrent_client
```

* Donwload all the pre-requisites present in requirements.txt 

```bash
    $ pip install -r requirements.txt
```

## Run :computer:

* Change directory to src and run the help command

```bash
    $ cd src
    $ python3 main.py --help
```

* To test the code, run the client with torrent source file path and destination path for downloaded file.

    As an example, we are using a torrent file from examples folder and creating a new folder as destination.

```bash
    $ mkdir ../downloads
    $ python3 main.py ../examples/ubuntu.torrent -d ../downloads/
```

## Directory Structure
```
-  OurTorrent_client
    -  examples
        -  ubuntu.torrent
    -  src
        -  logs
        -  client.py
        -  main.py
        -  peer.py
        -  peer_manager.py
        -  peer_network.py
        -  peer_socket.py
        -  peer_state.py
        -  shared_file_handler.py
        -  torrent_error.py
        -  torrent_logger.py
        -  torrent_parser.py
        -  torrent_sharing.py
        -  torrent_stats.py
        -  tracker_communication.py
    -  readme.md
    -  requirements.txt
```
