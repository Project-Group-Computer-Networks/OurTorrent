#ifndef PEER_DECODER_H
#define PEER_DECODER_H

#include "request.h"
#include <vector>

std::vector<Peer> decodeResponse(const std::vector<uint8_t> &response);
void printPeers(const std::vector<Peer> &peers);

#endif