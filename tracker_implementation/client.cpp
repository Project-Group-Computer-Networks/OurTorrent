#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <string>

using namespace std;

bool Initialize()
{
    return true; // No WSAStartup needed in Linux
}

void SendMsg(int s, sockaddr_in servaddr)
{
    cout << "Chat Name: " << endl;
    string name;
    getline(cin, name);
    string message;

    while (true)
    {
        getline(cin, message);
        string msg = name + ": " + message;
        int b = sendto(s, msg.c_str(), msg.length(), 0, (sockaddr *)&servaddr,
                       sizeof(servaddr));
        if (b < 0)
        {
            cout << "Send failed" << endl;
            break;
        }
        if (message == "quit")
        {
            cout << "Stopping application" << endl;
            break;
        }
    }
    close(s); // Use `close()` in Linux
}

void ReceiveMsg(int s)
{
    char buffer[4096];

    while (true)
    {
        int b = recvfrom(s, buffer, sizeof(buffer), 0, NULL, NULL);
        if (b <= 0)
        {
            cout << "Disconnected" << endl;
            break;
        }
        for (int i = 0; i < b; ++i)
        {
            printf("%02x ", (unsigned char)buffer[i]);
        }
        printf("\n");
        string message(buffer, b);
        cout << "Message from server: " << message << endl;
    }
    close(s); // Close socket
}

int main()
{
    if (!Initialize())
    {
        cout << "Initialization failed" << endl;
        return 0;
    }

    cout << "Client" << endl;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
    {
        cout << "Socket creation failed" << endl;
        return 0;
    }

    string serveraddress = "127.0.0.1";
    int port = 6969;
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));

    if (connect(s, reinterpret_cast<sockaddr *>(&serveraddr), sizeof(serveraddr)) < 0)
    {
        cout << "Connection failed" << endl;
        close(s);
        return 0;
    }

    cout << "Connected" << endl;
    thread senderThread(SendMsg, s, serveraddr);
    thread receiverThread(ReceiveMsg, s);
    senderThread.join();
    receiverThread.join();

    cout << "Client finished" << endl;
    return 0;
}
