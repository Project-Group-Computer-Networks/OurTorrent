#include "udp_server.h"
#include "db_manager.h"
#include "peer_manager.h"
#include "socket_util.h"

int main()
{
    initializeDatabase();
    startUDPServer();
    return 0;
}