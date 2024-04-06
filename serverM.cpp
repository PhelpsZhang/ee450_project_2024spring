#include "serverM.h"
#include <iostream>

#define LOCAL_HOST "127.0.0.1"
#define TCP_PORT 8080
#define UDP_PORT 8888 // target UDP S server
#define QUEUE_LIMIT 5
#define MAXLINE 1024

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
    char buffer[1024];

    while(1){
        memset(&buffer, 0, sizeof(buffer));
        int recvFlag = recv(serverChildSocket, buffer, sizeof(buffer), 0);
        if (recvFlag <= 0) break;
        std::cout << "Data received from Client: " << buffer << std::endl;
        // add control logic to Which Server
        forwardToBackendServer(buffer);
    }
    close(serverParentSocket);
    return 0;
}


/*
Client - UDP Socket
Create Socket - socket()
Connect - connect()
---
Data Transfer - myHandleFunction()
Handle Response - myHandleFunction()
Destory Socket - close()
*/

int forwardToBackendServer(const char* roomcode){

    char buffer[MAXLINE];
    // const char *hello = "Hello from serverM by UDP";
    sockaddr_in serverAddress;

    int serverUdpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, LOCAL_HOST, &(serverAddress.sin_addr));

    socklen_t len;

    // sendto(serverUdpSocket, (const char*)hello, strlen(hello), MSG_CONFIRM,
    //    (const struct sockaddr *) &serverAddress, sizeof(serverAddress));
    // std::cout << "Hello Message sent " << std::endl;

    //if(roomcode[0] == 'S') {
    if(1) {
        memset(&buffer, 0, sizeof(buffer));
        sendto(serverUdpSocket, roomcode, strlen(roomcode), MSG_CONFIRM,
        (const struct sockaddr *) &serverAddress, sizeof(serverAddress));
        std::cout << "S roomcode sent." << std::endl;

        int n = recvfrom(serverUdpSocket, (char*)buffer, MAXLINE,
                MSG_WAITALL, (struct sockaddr *) &serverAddress, &len);
        buffer[n] = '\0';
        std::cout << "serverS: " << buffer << std::endl; 
    }

    close(serverUdpSocket);

    return 0;
}


int main() {
    std::cout << "Hello World!" << std::endl;
    testFunction(3);
    serverSocketInitialize();
    return 0;
}