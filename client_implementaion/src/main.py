import sys
import argparse

from client import *

def main(user_arguments):
   
    client = ourtorrent_client(user_arguments)
    client.connect_tracker()
    
    # initialize the network of peers
    client.start_peer_network()
    
    # download the file from the network
    client.begin_task()

if __name__ == '__main__':
    desc  = 'OurTorrent Client'

    # argument parser for bittorrent
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument(TORRENT_FILE_PATH, help='unix file path of torrent file')
    parser.add_argument("-d", "--" + DOWNLOAD_DIR_PATH, help="unix directory path of downloading file")
    parser.add_argument("-s", "--" + SEEDING_DIR_PATH, help="unix directory path for the seeding file")
    parser.add_argument("-m", "--" + MAX_PEERS, help="maximum peers participating in upload/download of file")
    parser.add_argument("-l", "--" + RATE_LIMIT, help="upload / download limits in Kbps")

    # get the user input option after parsing the command line argument
    options = vars(parser.parse_args(sys.argv[1:]))
    
    if(options[DOWNLOAD_DIR_PATH] is None and options[SEEDING_DIR_PATH] is None):
        print('OutTorrent works with either download or upload arguments, try using --help')
        sys.exit()
    
    if options[MAX_PEERS] and int(options[MAX_PEERS]) > 50:
        print("OutTorrent client doesn't support more than 50 peer connection !")
        sys.exit()
    
    if options[RATE_LIMIT] and int(options[RATE_LIMIT]) <= 0:
        print("OurTorrent client upload / download rate must always greater than 0 Kbps")
        sys.exit()
    
    # call the main function
    main(options)
