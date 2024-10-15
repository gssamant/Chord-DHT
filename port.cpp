#include <iostream>
#include <cstdlib>
#include <ctime> // For time()
#include <cstring> // For memset
#include "headers.h"
#include "port.h"

using namespace std;

/* generate a port number to run on */
void SocketAndPort::specifyPortServer() {

    /* generating a port number between 1024 and 65535 */
    srand(static_cast<unsigned int>(time(0)));
    portNoServer = rand() % 65536;
    if (portNoServer < 1024)
        portNoServer += 1024;

    int len = sizeof(current);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    current.sin_family = AF_INET;
    current.sin_port = htons(portNoServer);
    current.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(sock, (struct sockaddr*)&current, len) == SOCKET_ERROR) {
        cerr << "Error: " << WSAGetLastError() << endl;
        exit(-1);
    }
}

/* change Port Number */
void SocketAndPort::changePortNumber(int newPortNumber) {
    if (newPortNumber < 1024 || newPortNumber > 65535) {
        cout << "Please enter a valid port number\n";
    }
    else {
        if (portInUse(newPortNumber)) {
            cout << "Sorry but port number is already in use\n";
        }
        else {
            closesocket(sock); // Use closesocket() for Windows
            int len = sizeof(current);
            sock = socket(AF_INET, SOCK_DGRAM, 0);
            current.sin_port = htons(newPortNumber);
            if (bind(sock, (struct sockaddr*)&current, len) == SOCKET_ERROR) {
                cerr << "Error: " << WSAGetLastError() << endl;
                current.sin_port = htons(portNoServer);
            }
            else {
                portNoServer = newPortNumber;
                cout << "Port number changed to: " << portNoServer << endl;
            }
        }
    }
}

/* check if a port number is already in use */
bool SocketAndPort::portInUse(int portNo) {
    SOCKET newSock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in newCurr;
    int len = sizeof(newCurr);
    newCurr.sin_port = htons(portNo);
    newCurr.sin_family = AF_INET;
    newCurr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(newSock, (struct sockaddr*)&newCurr, len) == SOCKET_ERROR) {
        cerr << "Error: " << WSAGetLastError() << endl;
        return true;
    }
    else {
        closesocket(newSock); // Use closesocket() for Windows
        return false;
    }
}

/* get IP Address */
string SocketAndPort::getIpAddress() {
    return inet_ntoa(current.sin_addr);
}

/* get port number on which it is listening */
int SocketAndPort::getPortNumber() {
    return portNoServer;
}

/* */
SOCKET SocketAndPort::getSocketFd() { // Change return type to SOCKET
    return sock;
}

/* close socket */
void SocketAndPort::closeSocket() {
    closesocket(sock); // Use closesocket() for Windows
}
