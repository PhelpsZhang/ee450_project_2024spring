#include "client.h"
#include <iostream>

#define FOREIGN_PORT 8080
#define FOREIGN_IP "127.0.0.1"

void clientSocketInitialize(){

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverSocketAddress;
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_port = htons(FOREIGN_PORT);
    inet_pton(AF_INET, FOREIGN_IP, &(serverSocketAddress.sin_addr));
    connect(clientSocket, (struct sockaddr*)&serverSocketAddress, sizeof(serverSocketAddress));

    // send data
    const char* message = "hello, serverM!";
    send(clientSocket, message, strlen(message), 0);

    close(clientSocket);
}

int main(){

    clientSocketInitialize();
    return 0;
}