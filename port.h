#ifndef port_h
#define port_h

#include <iostream>
#include <winsock2.h> // For Windows socket API
#include <ws2tcpip.h> // For additional socket-related functions
using namespace std;

class SocketAndPort {
private:
    int portNoServer;
    SOCKET sock; // Change int to SOCKET for Windows
    struct sockaddr_in current;

public:
    void specifyPortServer();
    void changePortNumber(int portNo);
    void closeSocket();
    bool portInUse(int portNo);
    string getIpAddress();
    int getPortNumber();
    SOCKET getSocketFd(); // Change int to SOCKET
};

#endif
