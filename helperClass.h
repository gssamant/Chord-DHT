#ifndef helper_h
#define helper_h

#include <iostream>
#include <vector>
#include <string> // Include for std::string
#include <utility> // Include for std::pair

#include "nodeInformation.h"

using namespace std;

typedef long long int lli;

class HelperFunctions {

public:
    vector<string> splitCommand(string command);
    string combineIpAndPort(string ip, string port);
    vector<pair<lli, string>> separateKeysAndValues(string keysAndValues);
    vector<pair<string, int>> separateSuccessorList(string succList);
    string splitSuccessorList(vector<pair<pair<string, int>, lli>> list);

    lli getHash(string key);
    pair<string, int> getIpAndPort(string key);

    bool isKeyValue(string id);

    bool isNodeAlive(string ip, int port);

    void setServerDetails(struct sockaddr_in& server, string ip, int port);
    void setTimer(struct timeval& timer);

    void sendNecessaryKeys(NodeInformation& nodeInfo, SOCKET newSock, struct sockaddr_in client, string nodeIdString); // Change newSock to SOCKET
    void sendKeyToNode(pair<pair<string, int>, lli> node, lli keyHash, string value);
    void sendValToNode(NodeInformation nodeInfo, SOCKET newSock, struct sockaddr_in client, string nodeIdString); // Change newSock to SOCKET
    string getKeyFromNode(pair<pair<string, int>, lli> node, string keyHash);
    pair<lli, string> getKeyAndVal(string keyAndVal);
    void getKeysFromSuccessor(NodeInformation& nodeInfo, string ip, int port);
    void storeAllKeys(NodeInformation& nodeInfo, string keysAndValues);

    pair<pair<string, int>, lli> getPredecessorNode(string ip, int port, string ipClient, int ipPort, bool forStabilize);
    lli getSuccessorId(string ip, int port);

    void sendPredecessor(NodeInformation nodeInfo, SOCKET newSock, struct sockaddr_in client); // Change newSock to SOCKET
    void sendSuccessor(NodeInformation nodeInfo, string nodeIdString, SOCKET newSock, struct sockaddr_in client); // Change newSock to SOCKET
    void sendSuccessorId(NodeInformation nodeInfo, SOCKET newSock, struct sockaddr_in client); // Change newSock to SOCKET
    void sendAcknowledgement(SOCKET newSock, struct sockaddr_in client); // Change newSock to SOCKET

    vector<pair<string, int>> getSuccessorListFromNode(string ip, int port);
    void sendSuccessorList(NodeInformation& nodeInfo, SOCKET sock, struct sockaddr_in client); // Change sock to SOCKET
};

#endif
