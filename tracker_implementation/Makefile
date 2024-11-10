CXX = g++
CXXFLAGS = -Wall -std=c++20

OBJ = main.o udp_server.o db_manager.o bencode_parser.o peer_manager.o

all: tracker

tracker: $(OBJ)
    $(CXX) $(CXXFLAGS) -o tracker $(OBJ) -lsqlite3

%.o: %.cpp
    $(CXX) $(CXXFLAGS) -c $<

clean:
    rm -f *.o tracker
