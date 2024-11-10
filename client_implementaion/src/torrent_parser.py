'''
    torrent_parser.py parses bencoded .torrent files and 
    extracts metadata from it.
'''

import bencodepy # bencodepy module for reading the torrent metadata
import hashlib # hashlib module for generating sha1 hash values

import sys
from collections import OrderedDict 
from rich.table import Table
from rich.console import Console


from torrent_logger import *
from torrent_error import *


# Metadata from the torrent file
class torrent_metadata():
    def __init__(self, trackers_url_list, file_name, file_size, piece_length, pieces, info_hash, files):
        self.trackers_url_list  = trackers_url_list     # list   : URL of trackers
        self.file_name      = file_name                 # string : file name 
        self.file_size      = file_size                 # int    : file size in bytes
        self.piece_length   = piece_length              # int    : piece length in bytes
        self.pieces         = pieces                    # bytes  : sha1 hash concatination of file
        self.info_hash      = info_hash                 # sha1 hash of the info metadata
        self.files          = files                     # list   : [length, path] (multifile torrent)


class torrent_file_parser(torrent_metadata):
    def __init__(self, torrent_file_path):
        self.torrent_file_logger = torrent_logger('torrent file', TORRENT_LOG_FILE, DEBUG)
        try :
            self.torrent_file_raw_extract = bencodepy.decode_from_file(torrent_file_path)
            torrent_read_log = 'Torrent file decoded successfully ' + SUCCESS
            self.torrent_file_logger.log(torrent_read_log)

        except Exception as err:
            torrent_read_log = 'Torrent file decoding failure ! ' + FAILURE + str(err)
            self.torrent_file_logger.log(torrent_read_log)
            sys.exit()
        
        # check if encoding scheme is given in dictionary
        if b'encoding' in self.torrent_file_raw_extract.keys():
            self.encoding = self.torrent_file_raw_extract[b'encoding'].decode()
        else:
            self.encoding = 'UTF-8'
        
        # formatted metadata from the torrent file
        self.torrent_file_extract = self.extract_torrent_metadata(self.torrent_file_raw_extract)
        
        # check if there is list of trackers 
        if 'announce-list' in self.torrent_file_extract.keys():
            trackers_url_list = self.torrent_file_extract['announce-list'] 
        else:
            trackers_url_list = [self.torrent_file_extract['announce']]
        
        file_name    = self.torrent_file_extract['info']['name']
        piece_length = self.torrent_file_extract['info']['piece length']
        pieces       = self.torrent_file_extract['info']['pieces']
        info_hash    = self.generate_info_hash()
            
        # for multifile torrent
        files = None

        if 'files' in self.torrent_file_extract['info'].keys():
            files_dictionary = self.torrent_file_extract['info']['files']
            files = [(file_data['length'], file_data['path']) for file_data in files_dictionary]
            file_size = 0
            for file_length, file_path in files:
                file_size += file_length
        else : 
            file_size = self.torrent_file_extract['info']['length']
       
        super().__init__(trackers_url_list, file_name, file_size, piece_length, pieces, info_hash, files)
            
        self.torrent_file_logger.log(self.__str__())


    # extracts the metadata from the raw data into a dictionary
    def extract_torrent_metadata(self, torrent_file_raw_extract):
        torrent_extract = OrderedDict()
        
        for key, value in torrent_file_raw_extract.items():
            new_key = key.decode(self.encoding)

            # if type of value is of type dictionary then do deep copying
            if type(value) == OrderedDict:
                torrent_extract[new_key] = self.extract_torrent_metadata(value)
            # if the current torrent file could have multiple files with paths
            elif type(value) == list and new_key == 'files':
                torrent_extract[new_key] = list(map(lambda x : self.extract_torrent_metadata(x), value))
            elif type(value) == list and new_key == 'path':
                torrent_extract[new_key] = value[0].decode(self.encoding)
            # url list parameter
            elif type(value) == list and new_key == 'url-list' or new_key == 'collections':
                torrent_extract[new_key] = list(map(lambda x : x.decode(self.encoding), value))
            # if type of value is of type list
            elif type(value) == list :
                try:
                    torrent_extract[new_key] = list(map(lambda x : x[0].decode(self.encoding), value))
                except:
                    torrent_extract[new_key] = value
            # if type of value if of types byte
            elif type(value) == bytes and new_key != 'pieces':
                try:
                    torrent_extract[new_key] = value.decode(self.encoding)
                except:
                    torrent_extract[new_key] = value
            else :
                torrent_extract[new_key] = value

        # torrent extracted metadata
        return torrent_extract

    def generate_info_hash(self):
        sha1_hash = hashlib.sha1()
        raw_info = self.torrent_file_raw_extract[b'info']
        sha1_hash.update(bencodepy.encode(raw_info))
        return sha1_hash.digest()
    
    def get_data(self):
        return torrent_metadata(self.trackers_url_list, self.file_name, 
                                self.file_size,         self.piece_length,      
                                self.pieces,            self.info_hash, self.files)
    
    # provides complete info about the .torrent file as a table on terminal
    def __str__(self):
        console = Console()
        torrent_file_table = Table(title="TORRENT FILE DATA")

        torrent_file_table.add_column("TORRENT FILE DATA", style="cyan", no_wrap=True)
        torrent_file_table.add_column("DATA VALUE", style="green")

        tracker_urls = self.trackers_url_list[0]
        if len(self.trackers_url_list) < 3:
            for tracker_url in self.trackers_url_list[1:]:
                tracker_urls += f'\n{tracker_url}'
        else:
            tracker_urls += f'\n... {len(self.trackers_url_list) - 1} more tracker URLs!'

        torrent_file_table.add_row("Tracker List", tracker_urls)
        torrent_file_table.add_row("File name", str(self.file_name))
        torrent_file_table.add_row("File size", f"{self.file_size} B")
        torrent_file_table.add_row("Piece length", f"{self.piece_length} B")
        torrent_file_table.add_row("Info Hash", "20 Bytes file info hash value")

        file_count = str(len(self.files)) if self.files else "None"
        torrent_file_table.add_row("Files", file_count)

        console.print(torrent_file_table)
        return str(torrent_file_table)
