#include "serverM.h"
#define LOCAL_HOST "127.0.0.1"


int loadMember() {
    // load members from DB to data structure (memory).
    return 0;
}

int recvAuthMessage(int socketFD, std::string &output) {
    // receive the message including length and data.
    uint32_t len;
    recv(socketFD, &len, sizeof(len), 0);
    len = ntohl(len);

    char *buffer = new char[len+1];
    int readBytesLen = recv(socketFD, buffer, len, 0);
    buffer[len] = '\0';
    if (readBytesLen > 0) {
        output = std::string(buffer, readBytesLen);
    } else {
        std::cout << "socket receive error OR connection down" << std::endl;
    }
    delete[] buffer;
    return 0;
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
    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(ERROR_FLAG == serverSocketFD) {
        std::cout << "Socket FD Failed." << std::endl;
        return ERROR_FLAG;
    }

    // prepare socket address struct.
    sockaddr_in serverSocketAddress;
    memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_port = htons(TCP_PORT);
    // serverAddress.sin_addr.s_addr = LOCAL_HOST;
    inet_pton(AF_INET, LOCAL_HOST, &(serverSocketAddress.sin_addr));

    // bind socket to local (socket, address(IP+portNumber), length)
    // add some error check 
    if (ERROR_FLAG == bind(serverSocketFD, (struct sockaddr*)&serverSocketAddress, sizeof(serverSocketAddress))) {
        std::cout << "Socket bind Failed." << std::endl;
        return ERROR_FLAG;
    }

    // listening to assigned socket
    // add error check
    if (ERROR_FLAG == listen(serverSocketFD, QUEUE_LIMIT)) {
        std::cout << "Socket listen Failed." << std::endl;
        return ERROR_FLAG;  
    }
    
    sockaddr_in clientSocketAddress;
    socklen_t clientAddrLen = sizeof(clientSocketAddress);
    memset(&clientSocketAddress, 0, clientAddrLen);
    // accepting connection request
    int serverChildSocketFD = accept(serverSocketFD, (struct sockaddr *)&clientSocketAddress, &clientAddrLen);
    // child socket created, receiving data
    if (ERROR_FLAG == serverChildSocketFD) {
        std::cout << "Socket accept Failed." << std::endl;
        return ERROR_FLAG;       
    }

    // recv(), -1, error; 0, connect down; x, bytes length
    std::string encryptUsername, encryptPassword;
    recvAuthMessage(serverChildSocketFD, encryptUsername);
    recvAuthMessage(serverChildSocketFD, encryptPassword);

    std::cout << "read username: " << encryptUsername << std::endl;
    std::cout << "read password: " << encryptPassword << std::endl;

    char buffer[1024] = {};
    
    while(1){
        memset(&buffer, 0, sizeof(buffer));
        int recvFlag = recv(serverChildSocketFD, buffer, sizeof(buffer), 0);
        if (recvFlag <= 0) break;
        std::cout << "Data received from Client: " << buffer << std::endl;
        // add control logic to Which Server
        forwardToBackendServer(buffer);
    }

    // while(1){
    //     memset(&buffer, 0, sizeof(buffer));
    //     int recvFlag = recv(serverChildSocket, buffer, sizeof(buffer), 0);
    //     if (recvFlag <= 0) break;
    //     std::cout << "Data received from Client: " << buffer << std::endl;
    //     // add control logic to Which Server
    //     forwardToBackendServer(buffer);
    // }
    close(serverChildSocketFD);
    close(serverSocketFD);
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
    serverAddress.sin_port = htons(TARGET_UDP_PORT);
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
    std::cout << "The main server is up and running." << std::endl;
    serverSocketInitialize();
    return 0;
}