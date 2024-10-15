#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <winsock2.h> // Include for Windows socket API
#include <ws2tcpip.h> // Include for other socket-related functions

#include "init.h"
#include "port.h"
#include "functions.h"
#include "helperClass.h"
#include "nodeInformation.h"

using namespace std;

void initialize(){
    
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock. Error Code: " << WSAGetLastError() << endl;
        return;
    }

    NodeInformation nodeInfo = NodeInformation();

    /* open a socket to listen to other nodes */
    nodeInfo.sp.specifyPortServer();

    cout << "Now listening at port number " << nodeInfo.sp.getPortNumber() << endl;
    cout << "Type help to know more\n";

    string command;

    while(1){
        cout << "> ";
        getline(cin, command);

        /* find space in command and separate arguments */
        HelperFunctions help = HelperFunctions();
        vector<string> arguments = help.splitCommand(command);

        string arg = arguments[0];
        if (arguments.size() == 1) {

            /* creates */
            if (arg == "create") {
                if (nodeInfo.getStatus() == true) {
                    cout << "Sorry but this node is already on the ring\n";
                }
                else {
                    thread first(create, ref(nodeInfo));
                    first.detach();
                }
            }

            /* prints */
            else if (arg == "printstate") {
                if (nodeInfo.getStatus() == false) {
                    cout << "Sorry this node is not in the ring\n";
                }
                else
                    printState(nodeInfo);
            }

            /* leaves */
            else if (arg == "leave") {
                leave(nodeInfo);
                nodeInfo.sp.closeSocket();
                WSACleanup(); // Clean up Winsock
                return;
            }

            /* print current port number */
            else if (arg == "port") {
                cout << nodeInfo.sp.getPortNumber() << endl;
            }

            /* print keys present in this node */
            else if (arg == "print") {
                if (nodeInfo.getStatus() == false) {
                    cout << "Sorry this node is not in the ring\n";
                }
                else
                    nodeInfo.printKeys();
            }

            else if (arg == "help") {
                showHelp();
            }

            else {
                cout << "Invalid Command\n";
            }
        }

        else if (arguments.size() == 2) {

            /* */
            if (arg == "port") {
                if (nodeInfo.getStatus() == true) {
                    cout << "Sorry you can't change port number now\n";
                }
                else {
                    int newPortNo = atoi(arguments[1].c_str());
                    nodeInfo.sp.changePortNumber(newPortNo);
                }
            }

            /* */
            else if (arg == "get") {
                if (nodeInfo.getStatus() == false) {
                    cout << "Sorry this node is not in the ring\n";
                }
                else
                    get(arguments[1], nodeInfo);
            }

            else {
                cout << "Invalid Command\n";
            }
        }

        else if (arguments.size() == 3) {

            /* */
            if (arg == "join") {
                if (nodeInfo.getStatus() == true) {
                    cout << "Sorry but this node is already on the ring\n";
                }
                else
                    join(nodeInfo, arguments[1], arguments[2]);
            }

            /* puts the entered key and its value to the necessary node */
            else if (arg == "put") {
                if (nodeInfo.getStatus() == false) {
                    cout << "Sorry this node is not in the ring\n";
                }
                else
                    put(arguments[1], arguments[2], nodeInfo);
            }

            else {
                cout << "Invalid Command\n";
            }
        }

        else {
            cout << "Invalid Command\n";
        }
    }

    // Clean up Winsock
    WSACleanup();
}
