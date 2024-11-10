'''
    tracker_communication.py stores request parameters needed by client
    for communication with tracker.
    It has provisions to establish HTTP as well as UDP connections
    one of which it chooses based on the tracker URL.
'''

import requests
import random as rd
import struct
from rich.table import Table
from rich.console import Console

import bencodepy
from socket import *

from torrent_logger import *
from torrent_error import *

class tracker_request_data():
    def __init__(self, torrent):
        self.compact = 1
        self.request_parameters = {
            'info_hash' : torrent.torrent_metadata.info_hash,
            'peer_id'   : torrent.peer_id,
            'port'      : torrent.client_port,
            'uploaded'  : torrent.statistics.num_pieces_uploaded,
            'downloaded': torrent.statistics.num_pieces_downloaded,
            'left'      : torrent.statistics.num_pieces_left,
            'compact'   : self.compact
        }
        self.interval   = None
        self.complete   = None
        self.incomplete = None
        self.peers_list = [] 


# Allows establishing HTTP connection to communicate with any HTTP tracker
class http_tracker(tracker_request_data):
    
    def __init__(self, torrent, tracker_url):
        super().__init__(torrent)
        self.tracker_url = tracker_url
        self.tracker_logger = torrent_logger(self.tracker_url, TRACKER_LOG_FILE, DEBUG)

    # returns true if conncetion is established false otherwise
    def request_torrent_information(self):
        try:
            # the reponse from HTTP tracker is an bencoded dictionary 
            bencoded_response = requests.get(self.tracker_url, self.request_parameters, timeout=5)
            raw_response_dict = bencodepy.decode(bencoded_response.content)
            self.parse_http_tracker_response(raw_response_dict)
            return True
        except Exception as error_msg:
            self.tracker_logger.log(self.tracker_url + ' connection failed !' + FAILURE) 
            return False

    # extract info from raw HTTP response
    def parse_http_tracker_response(self, raw_response_dict):
        
        if b'interval' in raw_response_dict:
            self.interval = raw_response_dict[b'interval']
        if b'peers' in raw_response_dict:
            self.peers_list = []
            raw_peers_data = raw_response_dict[b'peers']
            raw_peers_list = [raw_peers_data[i : 6 + i] for i in range(0, len(raw_peers_data), 6)]
            for raw_peer_data in raw_peers_list:
                peer_IP = ".".join(str(int(a)) for a in raw_peer_data[0:4])
                peer_port = raw_peer_data[4] * 256 + raw_peer_data[5]
                self.peers_list.append((peer_IP, peer_port))
        if b'tracker id' in raw_response_dict:
            self.tracker_id = raw_response_dict[b'tracker id']
            
        # number of seeders
        if b'complete' in raw_response_dict:
            self.complete = raw_response_dict[b'complete']

        # number of leechers
        if b'incomplete' in raw_response_dict:
            self.incomplete = raw_response_dict[b'incomplete']
        

    def get_peers_data(self):
        peer_data = {'interval' : self.interval, 'peers' : self.peers_list,
                     'leechers' : self.incomplete, 'seeders'  : self.complete}
        return peer_data

    # logs the information obtained by the HTTP tracker on the terminal
    def __str__(self):
        console = Console()
        tracker_table = Table(title="HTTP TRACKER RESPONSE DATA")

        tracker_table.add_column("HTTP TRACKER RESPONSE DATA", style="cyan", no_wrap=True)
        tracker_table.add_column("DATA VALUE", style="green")

        tracker_table.add_row("HTTP tracker URL", self.tracker_url)
        tracker_table.add_row("Interval", str(self.interval))
        tracker_table.add_row("Number of leechers", str(self.incomplete))
        tracker_table.add_row("Number of seeders", str(self.complete))

        peer_data = f"({self.peers_list[0][0]} : {self.peers_list[0][1]})\n"
        peer_data += f"... {len(self.peers_list) - 1} more peers"
        tracker_table.add_row("Peers in swarm", peer_data)
        console.print(tracker_table)
        return str(tracker_table)


# Allows to communicate with any UDP tracker as per BEP 0015
class udp_torrent_tracker(tracker_request_data):
    def __init__(self, torrent, tracker_url):
        super().__init__(torrent)
        self.tracker_url, self.tracker_port = self.parse_udp_tracker_url(tracker_url)
        self.tracker_logger = torrent_logger(self.tracker_url, TRACKER_LOG_FILE, DEBUG)
        
        # connection id
        self.connection_id = 0x41727101980                       
        # action
        self.action = 0x0                                            
        # transaction id
        self.transaction_id = int(rd.randrange(0, 255))          
        
    def parse_udp_tracker_url(self, tracker_url):
        domain_url = tracker_url[6:].split(':')
        udp_tracker_url = domain_url[0]
        udp_tracker_port = int(domain_url[1].split('/')[0])
        return (udp_tracker_url, udp_tracker_port)


    # connect to UDP tracker returns true if 
    # connection is established else false
    def request_torrent_information(self):

        # create a socket for sending request and recieving responses
        self.tracker_sock = socket(AF_INET, SOCK_DGRAM) 
        self.tracker_sock.settimeout(5)

        # data for UDP tracker connection request
        connection_data = self.build_connection_payload()
        
        try:
            self.connection_id = self.udp_connection_request(connection_data)
            
            # data for announce request
            announce_data = self.build_announce_payload()
            self.raw_announce_reponse = self.udp_announce_request(announce_data)
            
            self.parse_udp_tracker_response(self.raw_announce_reponse)

            self.tracker_sock.close()
            
            if self.peers_list and len(self.peers_list) != 0:
                return True
            else:
                return False
            
        except Exception as error_msg:
            self.tracker_logger.log(self.tracker_url + str(error_msg) + FAILURE)
            self.tracker_sock.close()
            return False
            

    # creates the connection payload for the UDP tracker 
    def build_connection_payload(self):
        req_buffer  = struct.pack("!q", self.connection_id)     # first 8 bytes : connection_id
        req_buffer += struct.pack("!i", self.action)            # next 4 bytes  : action
        req_buffer += struct.pack("!i", self.transaction_id)    # next 4 bytes  : transaction_id
        return req_buffer


    # recieves the connection reponse from the tracker
    def udp_connection_request(self, connection_payload):
        self.tracker_sock.sendto(connection_payload, (self.tracker_url, self.tracker_port))
        try:
            raw_connection_data, conn = self.tracker_sock.recvfrom(2048)
        except :
            raise torrent_error('UDP tracker connection request failed')
        
        return self.parse_connection_response(raw_connection_data)


    def parse_connection_response(self, raw_connection_data):
        if(len(raw_connection_data) < 16):
            raise torrent_error('UDP tracker wrong reponse length of connection ID !')
        
        # extract the reponse action : first 4 bytes
        response_action = struct.unpack_from("!i", raw_connection_data)[0]       
        # error reponse from tracker 
        if response_action == 0x3:
            error_msg = struct.unpack_from("!s", raw_connection_data, 8)
            raise torrent_error('UDP tracker reponse error : ' + error_msg)
        
        # extract the reponse transaction id : next 4 bytes
        response_transaction_id = struct.unpack_from("!i", raw_connection_data, 4)[0]
        # compare the request and response transaction id
        if(response_transaction_id != self.transaction_id):
            raise torrent_error('UDP tracker wrong response transaction ID !')
        
        # extract the response connection id : next 8 bytes
        reponse_connection_id = struct.unpack_from("!q", raw_connection_data, 8)[0]
        return reponse_connection_id


    # returns the annouce request payload
    def build_announce_payload(self):
        # action = 1 (annouce)
        self.action = 0x1            
        # first 8 bytes connection_id
        announce_payload =  struct.pack("!q", self.connection_id)    
        # next 4 bytes is action
        announce_payload += struct.pack("!i", self.action)  
        # next 4 bytes is transaction id
        announce_payload += struct.pack("!i", self.transaction_id)  
        # next 20 bytes the info hash string of the torrent 
        announce_payload += struct.pack("!20s", self.request_parameters['info_hash'])
        # next 20 bytes the peer_id 
        announce_payload += struct.pack("!20s", self.request_parameters['peer_id'])         
        # next 8 bytes the number of bytes downloaded
        announce_payload += struct.pack("!q", self.request_parameters['downloaded'])
        # next 8 bytes the left bytes
        announce_payload += struct.pack("!q", self.request_parameters['left'])
        # next 8 bytes the number of bytes uploaded 
        announce_payload += struct.pack("!q", self.request_parameters['uploaded']) 
        # event 2 denotes start of downloading
        announce_payload += struct.pack("!i", 0x2) 
        # your IP address, set this to 0 if you want the tracker to use the sender
        announce_payload += struct.pack("!i", 0x0) 
        # some random key
        announce_payload += struct.pack("!i", int(rd.randrange(0, 255)))
        # number of peers require, set this to -1 by defualt
        announce_payload += struct.pack("!i", -1)                   
        # port on which response will be sent 
        announce_payload += struct.pack("!H", self.request_parameters['port'])   
        # extension is by default 0x2 which is request string
        # announce_payload += struct.pack("!H", 0x2)
        return announce_payload

    def udp_announce_request(self, announce_payload):
        raw_announce_data = None
        trails = 0
        while(trails < 8):
            try:
                self.tracker_sock.sendto(announce_payload, (self.tracker_url, self.tracker_port))    
                raw_announce_data, conn = self.tracker_sock.recvfrom(2048)
                break
            except:
                error_log = self.tracker_url + ' failed announce request attempt ' + str(trails + 1)
                self.tracker_logger.log(error_log + FAILURE)
            trails = trails + 1
        return raw_announce_data

    
    def parse_udp_tracker_response(self, raw_announce_reponse):
        if(len(raw_announce_reponse) < 20):
            raise torrent_error('Invalid response length in announcing!')
        
        # first 4 bytes is action
        response_action = struct.unpack_from("!i", raw_announce_reponse)[0]     
        # next 4 bytes is transaction id
        response_transaction_id = struct.unpack_from("!i", raw_announce_reponse, 4)[0]
        # compare for the transaction id
        if response_transaction_id != self.transaction_id:
            raise torrent_error('The transaction id in announce response do not match')
        
        # check if the response contains any error message
        if response_action != 0x1:
            error_msg = struct.unpack_from("!s", raw_announce_reponse, 8)
            raise torrent_error("Error while annoucing: %s" % error_msg)

        offset = 8
        self.interval = struct.unpack_from("!i", raw_announce_reponse, offset)[0]
        
        offset = offset + 4
        self.leechers = struct.unpack_from("!i", raw_announce_reponse, offset)[0] 
        
        offset = offset + 4
        self.seeders = struct.unpack_from("!i", raw_announce_reponse, offset)[0] 
        
        offset = offset + 4
        self.peers_list = []
        while(offset != len(raw_announce_reponse)):
            raw_peer_data = raw_announce_reponse[offset : offset + 6]    
            peer_IP = ".".join(str(int(a)) for a in raw_peer_data[0:4])
            peer_port = raw_peer_data[4] * 256 + raw_peer_data[5]
            self.peers_list.append((peer_IP, peer_port))
            offset = offset + 6


    def get_peers_data(self):
        peer_data = {'interval' : self.interval, 'peers'    : self.peers_list,
                     'leechers' : self.leechers, 'seeders'  : self.seeders}
        return peer_data
       
    
    def __exit__(self):
        self.tracker_sock.close()

    
    # logs the information obtained from UDP tracker
    def __str__(self):
        console = Console()
        tracker_table = Table(title="UDP TRACKER RESPONSE DATA")

        tracker_table.add_column("UDP TRACKER RESPONSE DATA", style="cyan", no_wrap=True)
        tracker_table.add_column("DATA VALUE", style="green")

        tracker_table.add_row("UDP tracker URL", self.tracker_url)
        tracker_table.add_row("Interval", str(self.interval))
        tracker_table.add_row("Number of leechers", str(self.leechers))
        tracker_table.add_row("Number of seeders", str(self.seeders))

        peer_data = f"({self.peers_list[0][0]} : {self.peers_list[0][1]})\n"
        peer_data += f"... {len(self.peers_list) - 1} more peers"
        tracker_table.add_row("Peers in swarm", peer_data)

        console.print(tracker_table)
        return str(tracker_table)



# Identifies tracker as HTTP or UDP and establishes reqd connection

class tracker_connection():

    # contructors initializes a torernt tracker connection given 
    # the tracker urls from the torrent metadata file
    def __init__(self, torrent):
        self.client_tracker = None
        
        self.connection_success         = 1 
        self.connection_failure         = 2
        self.connection_not_attempted   = 3
        
        self.trackers_logger = torrent_logger('trackers', TRACKER_LOG_FILE, DEBUG)
        self.trackers_list = []
        self.trackers_connection_status = []

        for tracker_url in torrent.torrent_metadata.trackers_url_list:
            # classify HTTP and UDP torrent trackers
            if 'http' in tracker_url[:4]:
                tracker = http_tracker(torrent, tracker_url)
            if 'udp' in tracker_url[:4]:
                tracker = udp_torrent_tracker(torrent, tracker_url)
            self.trackers_list.append(tracker)
            self.trackers_connection_status.append(self.connection_not_attempted)

    # attempts to connect to any tracker in given list
    def request_connection(self):
        for i, tracker in enumerate(self.trackers_list):
            if(tracker.request_torrent_information()):
                self.trackers_connection_status[i] = self.connection_success
                self.client_tracker = tracker
                break
            else:
                self.trackers_connection_status[i] = self.connection_failure
        
        self.trackers_logger.log(str(self))
        
        # returns tracker for which successful connection was established
        return self.client_tracker
        
    def __str__(self):

        console = Console()
        trackers_table = Table(title="TRACKERS LIST")

        trackers_table.add_column("TRACKERS LIST", style="cyan", no_wrap=True)
        trackers_table.add_column("CONNECTION STATUS", style="green")

        successful_tracker_url = None
        unsuccessful_tracker_url = None
        unsuccessful_tracker_url_count = 0
        not_attempted_tracker_url = None
        not_attempted_tracker_url_count = 0

        # Trackers and corresponding connection status
        for i, status in enumerate(self.trackers_connection_status):
            if status == self.connection_success:
                successful_tracker_url = self.trackers_list[i].tracker_url
            elif status == self.connection_failure:
                unsuccessful_tracker_url = self.trackers_list[i].tracker_url
                unsuccessful_tracker_url_count += 1
            else:
                not_attempted_tracker_url = self.trackers_list[i].tracker_url
                not_attempted_tracker_url_count += 1

        if successful_tracker_url:
            trackers_table.add_row(successful_tracker_url, f"successful connection {SUCCESS}")

        if unsuccessful_tracker_url:
            unsuccessful_log = unsuccessful_tracker_url
            if unsuccessful_tracker_url_count > 1:
                unsuccessful_log += f"\n... {unsuccessful_tracker_url_count} connections"
            trackers_table.add_row(unsuccessful_log, f"failed connection {FAILURE}")

        if not_attempted_tracker_url:
            not_attempted_log = not_attempted_tracker_url
            if not_attempted_tracker_url_count > 1:
                not_attempted_log += f"\n... {not_attempted_tracker_url_count} connections"
            trackers_table.add_row(not_attempted_log, "not attempted connection")

        console.print(trackers_table)
        return str(trackers_table)
