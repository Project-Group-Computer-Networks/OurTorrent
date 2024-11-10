#include "socket_util.h"
#include "peer_manager.h"
using namespace std;

void startUDPServer()
{
    srand(time(0));
    cout << "Server" << endl;
    int listenSocket=createUDPIPv4Socket();
    if (listenSocket < 0) {
        cout << "Socket creation failed" << endl;
        return ;
    }

    sockaddr_in trackeraddr=createIPv4Address("0.0.0.0",PORT);

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&trackeraddr), sizeof(trackeraddr)) < 0) {
        cout << "Bind error" << endl;
        close(listenSocket);
        return;
    }

        
        char buffer[4096];
        sockaddr_in clientAddr;
        socklen_t addrLen=sizeof(clientAddr);
        while (true) {
        int bytesRcvd = recvfrom(listenSocket, buffer, sizeof(buffer), 0,(sockaddr*)&clientAddr,&addrLen);

        cout << "Message from client: " << endl;
        for(size_t i=0; i<bytesRcvd;i++)
        {
            printf("%02x",(unsigned char)buffer[i]);
        }
        cout<<endl;

        cout << "Client connected" << endl;
        if (bytesRcvd <= 0) {
            cout << "Client disconnected" << endl;
            break;
        }

        if (bytesRcvd >= 12) {
            uint32_t action;
            memcpy(&action, buffer + 8, sizeof(uint32_t));
            action = ntohl(action);

        if (action == ACTION_CONNECT)
        {
            handleConnectRequest(buffer, bytesRcvd, listenSocket, clientAddr, addrLen);
        }
        else if(action==ACTION_ANNOUNCE)
        {
            processAnnounceRequest(buffer, bytesRcvd, listenSocket,clientAddr);
        }
    }

        }
    close(listenSocket);
    return;
}