#include "serverM.h"
#include <iostream>

#define LOCAL_HOST "127.0.0.1"
#define TCP_PORT 8080
#define QUEUE_LIMIT 5

void testFunction(int a){
    std::cout << a << std::endl;
}

// Socket initialization

/*
Server - TCP Socket
Create Socket - socket()
Bind Socket - bind()
Listen - listen()
Accept - accept()
---
Data Transfer - myHandleFunction()
Handle Request - myHandleFunction()
    - Create Child Socket - Fork()?
Destory Socket - close()
*/
int serverSocketInitialize() {
    // create socket;
    int serverParentSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    // prepare socket address struct.
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(TCP_PORT);
    // serverAddress.sin_addr.s_addr = LOCAL_HOST;
    inet_pton(AF_INET, LOCAL_HOST, &(serverAddress.sin_addr));

    // bind socket to local (socket, address(IP+portNumber), length)
    // add some error check 
    bind(serverParentSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    // listening to assigned socket
    // add error check
    listen(serverParentSocket, QUEUE_LIMIT);
    std::cout << "Parent Socket Listenning.." << std::endl;

    // sockaddr_in clientAddress;
    // accepting connection request
    int serverChildSocket = accept(serverParentSocket, NULL, NULL);
    // child socket created, receiving data
    char buffer[1024] = {0};
    recv(serverChildSocket, buffer, sizeof(buffer), 0);
    std::cout << "Data received from Client: " << buffer << std::endl;

    close(serverParentSocket);
    return 0;
}

/*
Client - TCP Socket
Create Socket - socket()
Connect - connect()
---
Data Transfer - myHandleFunction()
Handle Response - myHandleFunction()
Destory Socket - close()
*/


int main() {
    std::cout << "Hello World!" << std::endl;
    testFunction(3);
    serverSocketInitialize();
    return 0;
}