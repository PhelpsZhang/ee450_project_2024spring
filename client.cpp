#include "client.h"
#include <iostream>

#define FOREIGN_PORT 8080
#define FOREIGN_IP "127.0.0.1"

/*
Client - TCP Socket
Create Socket - socket()
Connect - connect()
---
Data Transfer - myHandleFunction()
Handle Response - myHandleFunction()
Destory Socket - close()
*/

void clientSocketInitialize(){

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverSocketAddress;

    memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));

    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_port = htons(FOREIGN_PORT);
    inet_pton(AF_INET, FOREIGN_IP, &(serverSocketAddress.sin_addr));
    int connectFlag = connect(clientSocket, (struct sockaddr*)&serverSocketAddress, sizeof(serverSocketAddress));

    if(connectFlag) {
        std::cout << "connect fail" << std::endl;
    }
    // send data
    //const char* roomcode = "S102";
    //send(clientSocket, roomcode, strlen(roomcode), 0);

    std::string message;
    while(1) {
        std::cout << "Enter a message: ";
        std::getline(std::cin, message);
        //if(message.empty() || std::cin.eof()) break;
        if(message == "quit") {break;};
        std::cout << "Final Send Message: " << message << std::endl;
        if (send(clientSocket, message.data(), message.size(), 0) == -1) {
            std::cout<<"xxx" <<std::endl;
            perror("send failed");
            break;
        }
    }

    close(clientSocket);
    
}

int main(){

    clientSocketInitialize();
    return 0;
}