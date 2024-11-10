# torrent_stats.py handles torrent statistics about 
# download rate and no of pieces downloaded of the files being shared

import time
from datetime import timedelta

class torrent_statistics():

    def __init__(self, torrent_metadata):
        self.uploaded = set([])
        self.downloaded = set([])
        self.upload_rate = 0.0
        self.download_rate = 0.0
        self.max_upload_rate = 0.0
        self.max_download_rate = 0.0
        self.total_upload_rate = 0.0
        self.total_download_rate = 0.0
        self.avg_upload_rate = 0.0
        self.avg_download_rate = 0.0
        self.event_start_time = 0
        self.event_end_time = 0
        self.num_pieces_downloaded = 0
        self.num_pieces_uploaded = 0
        self.num_pieces_left = 0
        self.file_size = torrent_metadata.file_size
        self.total_pieces = int(len(torrent_metadata.pieces) / 20)
        self.file_downloading_percentage = 0.0
        self.expected_download_completion_time = 0.0

    def start_time(self):
        self.event_start_time = time.time()
     
    def stop_time(self):
        self.event_end_time = time.time()
    
    def update_start_time(self, time_t):
        self.event_start_time = time_t

    def update_end_time(self, time_t):
        self.event_end_time = time_t


    # for updating the statistics after given piece is downloaded

    def update_download_rate(self, piece_index, piece_size):

        time = (self.event_end_time - self.event_start_time) / 2
        piece_size_kb = piece_size / (2 ** 10)
        self.download_rate = round(piece_size_kb / time, 2)
        self.downloaded.add(piece_index)
        self.num_pieces_downloaded += 1

        self.total_download_rate += self.download_rate
        self.avg_download_rate = self.total_download_rate / self.num_pieces_downloaded
        self.avg_download_rate = round(self.avg_download_rate , 2)

        self.max_download_rate = max(self.max_download_rate, self.download_rate)
        self.file_downloading_percentage = round((len(self.downloaded) * 100)/self.total_pieces, 2)
        time_left = (self.total_pieces - (len(self.downloaded))) * time
        self.expected_download_completion_time = timedelta(seconds=time_left)

    def update_upload_rate(self, piece_index, piece_size):
        time = (self.event_end_time - self.event_start_time) / 2
        piece_size_kb = piece_size / (2 ** 10)
        self.upload_rate = round(piece_size_kb / time, 2)
        self.num_pieces_uploaded += 1
        self.total_upload_rate += self.upload_rate
        self.avg_upload_rate = self.total_upload_rate / self.num_pieces_uploaded
        self.avg_upload_rate = round(self.avg_upload_rate, 2)
        self.max_upload_rate = max(self.max_upload_rate, self.upload_rate)
        
    def get_download_statistics(self):
        download_log  = 'File downloaded : ' + str(self.file_downloading_percentage) + ' % '
        download_log += '(Average downloading rate : ' + str(self.avg_download_rate) + ' Kbps  '
        download_log += 'Time remaining : ' + str(self.expected_download_completion_time) + ')'
        return download_log
    
    def get_upload_statistics(self):
        upload_log  = 'uploaded : [upload rate = ' 
        upload_log += str(self.upload_rate) + ' Kbps'
        upload_log += ', avg uploading rate = '
        upload_log += str(self.avg_upload_rate) + ' Kbps'
        upload_log += ', max uploading rate = ' 
        upload_log += str(self.max_upload_rate) + ' Kbps]'
        return upload_log
