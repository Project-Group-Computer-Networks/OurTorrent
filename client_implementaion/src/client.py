# client.py interacts with the tracker server and downloads
# the files from other peers using different user-defined modules

import sys

from torrent_parser import torrent_file_parser
from tracker_communication import tracker_connection
from torrent_sharing import *
from peer_network import swarm
from shared_file_handler import torrent_shared_file_handler
from torrent_logger import *

TORRENT_FILE_PATH = 'torrent_file_path'
DOWNLOAD_DIR_PATH = 'download_directory_path'
SEEDING_DIR_PATH  = 'seeding_directory_path'
MAX_PEERS         = 'max_peers'
RATE_LIMIT        = 'rate_limit'

# initialize the client with torrent file and user arguments
class ourtorrent_client():
    def __init__(self, user_arguments):
        
        torrent_file_path = user_arguments[TORRENT_FILE_PATH]
        self.ourtorrent_logger = torrent_logger('ourtorrent', OURTORRENT_LOG_FILE, DEBUG)
        self.ourtorrent_logger.set_console_logging()
        
        self.ourtorrent_logger.log('Reading ' + torrent_file_path + ' file ...')

        # read metadata 
        self.torrent_info = torrent_file_parser(torrent_file_path)
        
        # decide whether the user want to download or seed the torrent
        self.client_request = {'seeding' : None,               'downloading': None,
                               'uploading rate' : sys.maxsize,  'downloading rate' : sys.maxsize,
                               'max peers' : 4, 'AWS' : False}
        
        if user_arguments[DOWNLOAD_DIR_PATH]:
            self.client_request['downloading'] = user_arguments[DOWNLOAD_DIR_PATH]
            if user_arguments[RATE_LIMIT]:
                self.client_request['downloading rate'] = int(user_arguments[RATE_LIMIT])
        elif user_arguments[SEEDING_DIR_PATH]:
            self.client_request['seeding'] = user_arguments[SEEDING_DIR_PATH]
            if user_arguments[RATE_LIMIT]:
                self.client_request['uploading rate'] = int(user_arguments[RATE_LIMIT])
        
        if user_arguments[MAX_PEERS]:
            self.client_request['max peers'] = int(user_arguments[MAX_PEERS])
        
        self.torrent = torrent_sharing(self.torrent_info.get_data(), self.client_request)
        self.ourtorrent_logger.log(str(self.torrent))
        

    # for connecting to trackers to get list of peers
    def connect_tracker(self):

        self.ourtorrent_logger.log('Connecting to Trackers ...')
        self.trackers_list = tracker_connection(self.torrent)
        self.active_tracker = self.trackers_list.request_connection()
        self.ourtorrent_logger.log(str(self.active_tracker))

    # to initialize network of peers based on list from tracker
    def start_peer_network(self):

        self.ourtorrent_logger.log('Initializing the swarm of peers ...')
        peers_data = self.active_tracker.get_peers_data()
            
        if self.client_request['downloading'] != None:
            self.swarm = swarm(peers_data, self.torrent)
        
        if self.client_request['seeding'] != None:
            peers_data['peers'] = []
            # create swarm instance for seeding 
            self.swarm = swarm(peers_data, self.torrent)

    
    # to allow client to become a seeder
    def seed(self):

        self.ourtorrent_logger.log('Client started seeding ... ')
        upload_file_path = self.client_request['seeding'] 
        file_handler = torrent_shared_file_handler(upload_file_path, self.torrent)
        self.swarm.add_shared_file_handler(file_handler)
        self.swarm.seed_file()

    # for downloading torrent file from peer network
    def download(self):

        download_file_path = self.client_request['downloading'] + self.torrent.torrent_metadata.file_name
        self.ourtorrent_logger.log('Initializing the file handler for peers in swarm ... ')

        file_handler = torrent_shared_file_handler(download_file_path, self.torrent)
        file_handler.initialize_for_download()
        self.swarm.add_shared_file_handler(file_handler)
        
        self.ourtorrent_logger.log('Client started downloading (check torrent statistics) ... ')
        self.swarm.download_file() 

    # to start client's desired task of downloading or seeding
    def begin_task(self):
        if self.client_request['downloading'] is not None:
            self.download()
        if self.client_request['seeding'] is not None:
            self.seed()
