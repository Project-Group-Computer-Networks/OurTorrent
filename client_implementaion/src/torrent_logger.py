# handles logging detailed info of the whole torrenting process

import logging
import sys

TORRENT_LOG_DIR = './logs/'

# log file names
TRACKER_LOG         = 'tracker.log'
TORRENT_LOG         = 'torrent_parsing.log'
PEER_LOG            = 'peer.log'
SWARM_LOG           = 'connections.log'
TORRENT_STATS_LOG   = 'stats.log'
OURTORRENT_LOG      = 'ourtorrent.log'

# files paths required by the logger
TRACKER_LOG_FILE        = TORRENT_LOG_DIR + TRACKER_LOG
TORRENT_LOG_FILE        = TORRENT_LOG_DIR + TORRENT_LOG
PEER_LOG_FILE           = TORRENT_LOG_DIR + PEER_LOG
SWARM_LOG_FILE          = TORRENT_LOG_DIR + SWARM_LOG
TORRENT_STATS_LOG_FILE  = TORRENT_LOG_DIR + TORRENT_STATS_LOG
OURTORRENT_LOG_FILE     = TORRENT_LOG_DIR + OURTORRENT_LOG

DEBUG       = logging.DEBUG
INFO        = logging.INFO
WARNING     = logging.WARNING
ERROR       = logging.ERROR
CRITICAL    = logging.CRITICAL


class torrent_logger():

    def __init__(self, logger_name, file_name, verbosity_level = logging.DEBUG):
        self.logger_name    = logger_name
        self.file_name      = file_name
        self.verbosity_level  = verbosity_level
        
        open(self.file_name, "w").close()
        
        self.logger = logging.getLogger(self.logger_name)
        
        verbose_string  = '%(threadName)s - '
        verbose_string += '%(levelname)s - '
        verbose_string += '%(name)s \n'
        verbose_string += '%(message)s'
        
        self.verbose_formatter = logging.Formatter(verbose_string)

        file_handler = logging.FileHandler(self.file_name)
        file_handler.setFormatter(self.verbose_formatter)
        self.logger.addHandler(file_handler)
        
        self.logger.setLevel(self.verbosity_level)
    

    def set_console_logging(self):
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setFormatter(self.verbose_formatter)
        self.logger.addHandler(console_handler)

    def log(self, message):
        message = message + '\n'
        if self.verbosity_level == logging.DEBUG:
            self.logger.debug(message)
        elif self.verbosity_level == logging.INFO:
            self.logger.info(message)
        elif self.verbosity_level == logging.WARNING:
            self.logger.warning(message)
        elif self.verbosity_level == logging.ERROR:
            self.logger.error(message)
        elif self.verbosity_level == logging.CRITICAL:
            self.logger.critical(message)
