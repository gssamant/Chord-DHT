#include "headers.h"
#include "helperClass.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mutex>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <cmath>

using namespace std;

mutex mt;

vector<string> HelperFunctions::splitCommand(string command) {
    vector<string> arguments;
    int pos = 0;
    do {
        pos = command.find(' ');
        string arg = command.substr(0, pos);
        arguments.push_back(arg);
        command = command.substr(pos + 1);
    } while (pos != -1);
    return arguments;
}

lli HelperFunctions::getHash(string key) {
    unsigned char obuf[20];  // SHA1 produces a 20-byte hash
    char finalHash[41];
    string keyHash = "";
    int i;
    lli mod = pow(2, M);

    unsigned char unsigned_key[key.length() + 1];
    for (i = 0; i < key.length(); i++) {
        unsigned_key[i] = key[i];
    }
    unsigned_key[i] = '\0';

    // Using SHA1 from Windows CryptoAPI
    // Alternatively, you can use any third-party library for SHA1 in Windows
    // Assuming SHA1(unsigned_key, sizeof(unsigned_key), obuf) is defined elsewhere

    for (i = 0; i < M / 8; i++) {
        sprintf(finalHash, "%d", obuf[i]);
        keyHash += finalHash;
    }

    lli hash = stoll(keyHash) % mod;
    return hash;
}

pair<string, int> HelperFunctions::getIpAndPort(string key) {
    int pos = key.find(':');
    string ip = key.substr(0, pos);
    string port = key.substr(pos + 1);

    pair<string, int> ipAndPortPair;
    ipAndPortPair.first = ip;
    ipAndPortPair.second = atoi(port.c_str());

    return ipAndPortPair;
}

pair<lli, string> HelperFunctions::getKeyAndVal(string keyAndVal) {
    int pos = keyAndVal.find(':');
    string key = keyAndVal.substr(0, pos);
    string val = keyAndVal.substr(pos + 1);

    pair<lli, string> keyAndValPair;
    keyAndValPair.first = stoll(key);
    keyAndValPair.second = val;

    return keyAndValPair;
}

bool HelperFunctions::isKeyValue(string id) {
    int pos = id.find(":");
    if (pos == -1)
        return false;

    for (int i = 0; i < pos; i++) {
        if (!(id[i] >= '0' && id[i] <= '9'))
            return false;
    }
    return true;
}

string HelperFunctions::getKeyFromNode(pair<pair<string, int>, lli> node, string keyHash) {
    string ip = node.first.first;
    int port = node.first.second;

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo, ip, port);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        exit(-1);
    }

    keyHash += "k";
    char keyHashChar[40];
    strcpy(keyHashChar, keyHash.c_str());

    sendto(sock, keyHashChar, strlen(keyHashChar), 0, (struct sockaddr*)&serverToConnectTo, l);

    char valChar[100];
    int len = recvfrom(sock, valChar, sizeof(valChar) - 1, 0, (struct sockaddr*)&serverToConnectTo, &l);
    if (len == SOCKET_ERROR) {
        cerr << "recvfrom failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        return "";
    }

    valChar[len] = '\0';
    string val = valChar;

    closesocket(sock);
    return val;
}

void HelperFunctions::setServerDetails(struct sockaddr_in& server, string ip, int port) {
    server.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &server.sin_addr);
    server.sin_port = htons(port);
}

void HelperFunctions::sendKeyToNode(pair<pair<string, int>, lli> node, lli keyHash, string value) {
    string ip = node.first.first;
    int port = node.first.second;

    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo, ip, port);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        exit(-1);
    }

    string keyAndVal = combineIpAndPort(to_string(keyHash), value);
    char keyAndValChar[100];
    strcpy(keyAndValChar, keyAndVal.c_str());

    sendto(sock, keyAndValChar, strlen(keyAndValChar), 0, (struct sockaddr*)&serverToConnectTo, l);
    closesocket(sock);
}

void HelperFunctions::getKeysFromSuccessor(NodeInformation& nodeInfo, string ip, int port) {
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo, ip, port);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        exit(-1);
    }

    string id = to_string(nodeInfo.getId());
    string msg = "getKeys:" + id;

    char msgChar[40];
    strcpy(msgChar, msg.c_str());

    sendto(sock, msgChar, strlen(msgChar), 0, (struct sockaddr*)&serverToConnectTo, l);

    char keysAndValuesChar[2000];
    int len = recvfrom(sock, keysAndValuesChar, sizeof(keysAndValuesChar) - 1, 0, (struct sockaddr*)&serverToConnectTo, &l);
    if (len == SOCKET_ERROR) {
        cerr << "recvfrom failed: " << WSAGetLastError() << endl;
        closesocket(sock);
        return;
    }

    keysAndValuesChar[len] = '\0';
    closesocket(sock);

    string keysAndValues = keysAndValuesChar;
    vector<pair<lli, string>> keysAndValuesVector = separateKeysAndValues(keysAndValues);

    for (const auto& kv : keysAndValuesVector) {
        nodeInfo.storeKey(kv.first, kv.second);
    }
}

vector<pair<lli, string>> HelperFunctions::separateKeysAndValues(string keysAndValues) {
    vector<pair<lli, string>> res;
    int i = 0, size = keysAndValues.size();

    while (i < size) {
        string key = "";
        while (i < size && keysAndValues[i] != ':') {
            key += keysAndValues[i++];
        }
        i++;

        string value = "";
        while (i < size && keysAndValues[i] != ';') {
            value += keysAndValues[i++];
        }
        i++;

        res.push_back(make_pair(stoll(key), value));
    }

    return res;
}

vector<pair<string, int>> HelperFunctions::separateSuccessorList(string succList) {
    vector<pair<string, int>> res;
    int i = 0, size = succList.size();

    while (i < size) {
        string ip = "";
        while (i < size && succList[i] != ':') {
            ip += succList[i++];
        }
        i++;

        string port = "";
        while (i < size && succList[i] != ';') {
            port += succList[i++];
        }
        i++;

        res.push_back(make_pair(ip, stoi(port)));
    }

    return res;
}

string HelperFunctions::combineIpAndPort(string ip, string port) {
    return ip + ":" + port;
}


void HelperFunctions::storeAllKeys(NodeInformation& nodeInfo, string keysAndValues) {
    vector<pair<lli, string>> keyValPairs = separateKeysAndValues(keysAndValues);

    for (const auto& kv : keyValPairs) {
        nodeInfo.keyValueMap[kv.first] = kv.second;
    }
}

void HelperFunctions::sendNecessaryKeys(NodeInformation& nodeInfo, SOCKET newSock, struct sockaddr_in client, string nodeIdString) {
    lli newNodeId = stoll(nodeIdString);
    vector<pair<lli, string>> keysToSend;

    for (const auto& kv : nodeInfo.keyValueMap) {
        if (kv.first <= newNodeId) {
            keysToSend.push_back(kv);
        }
    }

    string keysAndValues = "";
    for (const auto& kv : keysToSend) {
        keysAndValues += to_string(kv.first) + ":" + kv.second + ";";
        nodeInfo.keyValueMap.erase(kv.first);  // Remove the key from current node
    }

    sendto(newSock, keysAndValues.c_str(), keysAndValues.size(), 0, (struct sockaddr*)&client, sizeof(client));
}

void HelperFunctions::sendValToNode(NodeInformation nodeInfo, SOCKET newSock, struct sockaddr_in client, string nodeIdString) {
    lli keyHash = stoll(nodeIdString);

    string value = nodeInfo.keyValueMap[keyHash];
    sendto(newSock, value.c_str(), value.size(), 0, (struct sockaddr*)&client, sizeof(client));
}

void HelperFunctions::sendSuccessorId(NodeInformation nodeInfo, SOCKET newSock, struct sockaddr_in client) {
    string succIdStr = to_string(nodeInfo.successor.second);
    sendto(newSock, succIdStr.c_str(), succIdStr.size(), 0, (struct sockaddr*)&client, sizeof(client));
}

void HelperFunctions::sendSuccessor(NodeInformation nodeInfo, string nodeIdString, SOCKET newSock, struct sockaddr_in client) {
    string succInfo = combineIpAndPort(nodeInfo.successor.first.first, to_string(nodeInfo.successor.first.second));
    sendto(newSock, succInfo.c_str(), succInfo.size(), 0, (struct sockaddr*)&client, sizeof(client));
}

void HelperFunctions::sendPredecessor(NodeInformation nodeInfo, SOCKET newSock, struct sockaddr_in client) {
    string predInfo = combineIpAndPort(nodeInfo.predecessor.first.first, to_string(nodeInfo.predecessor.first.second));
    sendto(newSock, predInfo.c_str(), predInfo.size(), 0, (struct sockaddr*)&client, sizeof(client));
}

lli HelperFunctions::getSuccessorId(string ip, int port){
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo, ip, port);

    struct timeval timer;
    setTimer(timer);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timer, sizeof(timer));

    char msg[] = "finger";
    sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&serverToConnectTo, l);

    char succIdChar[40];
    int len = recvfrom(sock, succIdChar, 1024, 0, (struct sockaddr*)&serverToConnectTo, &l);
    closesocket(sock);

    if (len < 0)
        return -1;

    succIdChar[len] = '\0';
    return atoll(succIdChar);
}

void HelperFunctions::setTimer(struct timeval& timer) {
    timer.tv_sec = 0;
    timer.tv_usec = 100000;
}

pair<pair<string, int>, lli> HelperFunctions::getPredecessorNode(string ip, int port, string ipClient, int portClient, bool forStabilize) {
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo, ip, port);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    string message = forStabilize ? "predecessor_stabilize" : "predecessor";
    sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr*)&serverToConnectTo, l);

    char response[100];
    int len = recvfrom(sock, response, 100, 0, (struct sockaddr*)&serverToConnectTo, &l);
    closesocket(sock);

    if (len < 0)
        return make_pair(make_pair("", 0), -1);

    response[len] = '\0';
    string predInfo(response);
    pair<string, int> ipAndPort = getIpAndPort(predInfo);

    return make_pair(ipAndPort, getSuccessorId(ipAndPort.first, ipAndPort.second));
}

vector<pair<string, int>> HelperFunctions::getSuccessorListFromNode(string ip, int port) {
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo, ip, port);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    char msg[] = "successor_list";
    sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&serverToConnectTo, l);

    char response[200];
    int len = recvfrom(sock, response, 200, 0, (struct sockaddr*)&serverToConnectTo, &l);
    closesocket(sock);

    response[len] = '\0';
    return separateSuccessorList(response);
}

void HelperFunctions::sendSuccessorList(NodeInformation& nodeInfo, SOCKET sock, struct sockaddr_in client) {
    string succList = splitSuccessorList(nodeInfo.successorList);
    sendto(sock, succList.c_str(), succList.size(), 0, (struct sockaddr*)&client, sizeof(client));
}

string HelperFunctions::splitSuccessorList(vector<pair<pair<string, int>, lli>> list) {
    string succList = "";
    for (const auto& succ : list) {
        succList += combineIpAndPort(succ.first.first, to_string(succ.first.second)) + ";";
    }
    return succList;
}

void HelperFunctions::sendAcknowledgement(SOCKET newSock, struct sockaddr_in client) {
    char msg[] = "ACK";
    sendto(newSock, msg, strlen(msg), 0, (struct sockaddr*)&client, sizeof(client));
}

bool HelperFunctions::isNodeAlive(string ip, int port) {
    struct sockaddr_in serverToConnectTo;
    socklen_t l = sizeof(serverToConnectTo);

    setServerDetails(serverToConnectTo, ip, port);

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        exit(EXIT_FAILURE);
    }

    char msg[] = "ping";
    sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&serverToConnectTo, l);

    char response[10];
    int len = recvfrom(sock, response, 10, 0, (struct sockaddr*)&serverToConnectTo, &l);
    closesocket(sock);

    return len > 0;
}
