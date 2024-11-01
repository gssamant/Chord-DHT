#include <iostream>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "headers.h"
#include "M.h"
#include "functions.h"
#include "helperClass.h"

#pragma comment(lib, "Ws2_32.lib") // Link with the Winsock library

typedef long long int lli;

using namespace std;

HelperFunctions help = HelperFunctions();

/* Function to initialize Winsock */
void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }
}

/* Function to clean up Winsock */
void cleanupWinsock() {
    WSACleanup();
}

/* put the entered key to the proper node */
void put(string key, string value, NodeInformation &nodeInfo) {
    if (key == "" || value == "") {
        cout << "Key or value field empty\n";
        return;
    } else {
        lli keyHash = help.getHash(key);
        cout << "Key is " << key << " and hash: " << keyHash << endl;

        pair<pair<string, int>, lli> node = nodeInfo.findSuccessor(keyHash);
        help.sendKeyToNode(node, keyHash, value);
        cout << "Key entered successfully\n";
    }
}

/* get key from the desired node */
void get(string key, NodeInformation nodeInfo) {
    if (key == "") {
        cout << "Key field empty\n";
        return;
    } else {
        lli keyHash = help.getHash(key);
        pair<pair<string, int>, lli> node = nodeInfo.findSuccessor(keyHash);
        string val = help.getKeyFromNode(node, to_string(keyHash));

        if (val == "")
            cout << "Key Not found\n";
        else
            cout << "Found " << key << ": " << val << endl;
    }
}

/* create a new ring */
void create(NodeInformation &nodeInfo) {
    string ip = nodeInfo.sp.getIpAddress();
    int port = nodeInfo.sp.getPortNumber();
    string key = ip + ":" + to_string(port);   
    lli hash = help.getHash(key);

    nodeInfo.setId(hash);
    nodeInfo.setSuccessor(ip, port, hash);
    nodeInfo.setSuccessorList(ip, port, hash);
    nodeInfo.setPredecessor("", -1, -1);
    nodeInfo.setFingerTable(ip, port, hash);
    nodeInfo.setStatus();

    thread second(listenTo, ref(nodeInfo));
    second.detach();

    thread fifth(doStabilize, ref(nodeInfo));
    fifth.detach();
}

/* join in a DHT ring */
void join(NodeInformation &nodeInfo, string ip, string port) {
    if (help.isNodeAlive(ip, atoi(port.c_str())) == false) {
        cout << "Sorry but no node is active on this ip or port\n";
        return;
    }

    struct sockaddr_in server;
    socklen_t l = sizeof(server);
    help.setServerDetails(server, ip, stoi(port));

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    string currIp = nodeInfo.sp.getIpAddress();
    string currPort = to_string(nodeInfo.sp.getPortNumber()); 
    lli nodeId = help.getHash(currIp + ":" + currPort);    

    char charNodeId[41];
    strcpy(charNodeId, to_string(nodeId).c_str());

    if (sendto(sock, charNodeId, strlen(charNodeId), 0, (struct sockaddr*)&server, l) == SOCKET_ERROR) {
        cerr << "Sendto failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        exit(EXIT_FAILURE);
    }

    char ipAndPort[40];
    int len;
    if ((len = recvfrom(sock, ipAndPort, 1024, 0, (struct sockaddr*)&server, &l)) == SOCKET_ERROR) {
        cerr << "Recvfrom failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        exit(EXIT_FAILURE);
    }
    ipAndPort[len] = '\0';

    closesocket(sock);
    cout << "Successfully joined the ring\n";

    string key = ipAndPort;
    lli hash = help.getHash(key);
    pair<string, int> ipAndPortPair = help.getIpAndPort(key);

    nodeInfo.setId(nodeId);
    nodeInfo.setSuccessor(ipAndPortPair.first, ipAndPortPair.second, hash);
    nodeInfo.setSuccessorList(ipAndPortPair.first, ipAndPortPair.second, hash);
    nodeInfo.setPredecessor("", -1, -1);
    nodeInfo.setFingerTable(ipAndPortPair.first, ipAndPortPair.second, hash);
    nodeInfo.setStatus();

    help.getKeysFromSuccessor(nodeInfo, ipAndPortPair.first, ipAndPortPair.second);

    thread fourth(listenTo, ref(nodeInfo));
    fourth.detach();

    thread third(doStabilize, ref(nodeInfo));
    third.detach();
}

/* print successor, predecessor, successor list and finger table of node */
void printState(NodeInformation nodeInfo) {
    string ip = nodeInfo.sp.getIpAddress();
    lli id = nodeInfo.getId();
    int port = nodeInfo.sp.getPortNumber();
    vector<pair<pair<string, int>, lli>> fingerTable = nodeInfo.getFingerTable();
    cout << "Self " << ip << " " << port << " " << id << endl;

    pair<pair<string, int>, lli> succ = nodeInfo.getSuccessor();
    pair<pair<string, int>, lli> pre = nodeInfo.getPredecessor();
    vector<pair<pair<string, int>, lli>> succList = nodeInfo.getSuccessorList();

    cout << "Succ " << succ.first.first << " " << succ.first.second << " " << succ.second << endl;
    cout << "Pred " << pre.first.first << " " << pre.first.second << " " << pre.second << endl;

    for (int i = 1; i <= M; i++) {
        ip = fingerTable[i].first.first;
        port = fingerTable[i].first.second;
        id = fingerTable[i].second;
        cout << "Finger[" << i << "] " << id << " " << ip << " " << port << endl;
    }
    for (int i = 1; i <= R; i++) {
        ip = succList[i].first.first;
        port = succList[i].first.second;
        id = succList[i].second;
        cout << "Successor[" << i << "] " << id << " " << ip << " " << port << endl;
    }
}

/* node leaves the DHT ring */
void leave(NodeInformation &nodeInfo) {
    pair<pair<string, int>, lli> succ = nodeInfo.getSuccessor();
    lli id = nodeInfo.getId();

    if (id == succ.second)
        return;

    vector<pair<lli, string>> keysAndValuesVector = nodeInfo.getAllKeysForSuccessor();
    if (keysAndValuesVector.size() == 0)
        return;

    string keysAndValues = "";
    for (int i = 0; i < keysAndValuesVector.size(); i++) {
        keysAndValues += to_string(keysAndValuesVector[i].first) + ":" + keysAndValuesVector[i].second;
        keysAndValues += ";";
    }
    keysAndValues += "storeKeys";

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);
    help.setServerDetails(serverToConnectTo, succ.first.first, succ.first.second);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    char keysAndValuesChar[2000];
    strcpy(keysAndValuesChar, keysAndValues.c_str());
    sendto(sock, keysAndValuesChar, strlen(keysAndValuesChar), 0, (struct sockaddr*)&serverToConnectTo, l);
    closesocket(sock);

    nodeInfo.clearAllKeys();
}

/* function to listen to the requests coming to node */
void listenTo(NodeInformation &nodeInfo) {
    struct sockaddr_in server;
    socklen_t l = sizeof(server);
    
    // Set server details
    help.setServerDetails(server, nodeInfo.sp.getIpAddress(), nodeInfo.sp.getPortNumber());

    // Create a UDP socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation failed: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the address
    if (bind(sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        cerr << "Bind failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        exit(EXIT_FAILURE);
    }

    while (true) {
        char incomingMessage[1024];
        int len = recvfrom(sock, incomingMessage, sizeof(incomingMessage) - 1, 0, (struct sockaddr*)&server, &l);
        if (len == SOCKET_ERROR) {
            cerr << "Recvfrom failed: " << WSAGetLastError() << endl;
            continue; // Optionally, you could break or exit here based on your logic
        }

        // Null-terminate the incoming message
        incomingMessage[len] = '\0';

        // Create a string from the incoming message
        string messageString(incomingMessage);

        // Call doTask with the correct parameters
        doTask(nodeInfo, sock, server, messageString); // Ensure server is used correctly
    }

    // Close the socket after exiting the loop
    closesocket(sock);
}


/* main function to start the DHT application */
// int main(int argc, char* argv[]) {
//     // Initialize Winsock
//     initializeWinsock();

//     NodeInformation nodeInfo;

//     if (argc < 3) {
//         cout << "Please provide the required parameters.\n";
//         cleanupWinsock();
//         return 1;
//     }

//     string command = argv[1];
//     string ipAddress = argv[2];
//     int portNumber = stoi(argv[3]);

//     nodeInfo.sp.setIpAddress(ipAddress);
//     nodeInfo.sp.setPortNumber(portNumber);

//     if (command == "create") {
//         create(nodeInfo);
//     } else if (command == "join") {
//         join(nodeInfo, ipAddress, to_string(portNumber));
//     } else {
//         cout << "Invalid command. Use 'create' or 'join'.\n";
//         cleanupWinsock();
//         return 1;
//     }

//     // Wait for user input before exiting
//     cout << "Press Enter to exit...\n";
//     cin.get();

//     // Clean up Winsock
//     cleanupWinsock();
//     return 0;
// }
