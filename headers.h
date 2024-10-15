#ifndef functions_h
#define functions_h

#include <iostream>
#include <string> // Include for std::string

#include "port.h"
#include "nodeInformation.h"

using namespace std;

typedef long long int lli;

// Function declarations
void put(string key, string value, NodeInformation &nodeInfo);
void get(string key, NodeInformation nodeInfo);
void create(NodeInformation &nodeInfo);
void join(NodeInformation &nodeInfo, string ip, string port);
void printState(NodeInformation nodeInfo);
void listenTo(NodeInformation &nodeInfo);
void doStabilize(NodeInformation &nodeInfo);
void callNotify(NodeInformation &nodeInfo, string ipAndPort);
void callFixFingers(NodeInformation &nodeInfo);
void doTask(NodeInformation &nodeInfo, SOCKET newSock, struct sockaddr_in client, string nodeIdString); // Change newSock to SOCKET
void leave(NodeInformation &nodeInfo);
void showHelp();

#endif
