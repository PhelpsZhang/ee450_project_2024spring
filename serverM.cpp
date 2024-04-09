#include "serverM.h"
#define LOCAL_HOST "127.0.0.1"

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


/*
Client - UDP Socket
Create Socket - socket()
Connect - connect()
---
Data Transfer - myHandleFunction()
Handle Response - myHandleFunction()
Destory Socket - close()
*/

int loadMember(std::unordered_map<std::string, std::string> &credentials) {
    // load members from DB to data structure (memory).
    std::ifstream file("member.txt");
    std::string line;

    if (!file) {
        std::cout << "cannot open file member_unencrypted.ext" << std::endl;
        return ERROR_FLAG;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username, password;
        if (std::getline(iss, username, ',')) {
            iss.get();
            std::getline(iss, password);
            credentials[username] = password;
        } else {
            std::cout << "format wrong OR file end." << std::endl;
        }
    }

    file.close();

    
    for (auto it = credentials.begin(); it != credentials.end(); ++it) {
        std::cout << "username:" << it->first << it->second << std::endl;
    }
    return 0;
}

AuthCode checkAuth(const std::unordered_map<std::string, std::string> &map, std::string username, std::string password) {
    if (0 == password.length()) return GUEST_SUCCESS;
    auto it = map.find(username);
    if (it == map.end()) {
        std::cout << "Username does not exist." << std::endl;
        return WRONG_USER;
    } else if (password != it->second) {
        std::cout << "Password does not match." << std::endl;
        return WRONG_PASS;
    }
    return MEMBER_SUCCESS;
}

std::string authToString(AuthCode code) {
    switch (code) {
        case GUEST_SUCCESS: return "100";
        case WRONG_USER: return "200";
        case WRONG_PASS: return "300";
        case MEMBER_SUCCESS: return "400";
        default: return "error";
    }
}

int recvAuthMessage(int socketFD, std::string &output) {
    // receive the message including length and data.
    uint32_t len;
    recv(socketFD, &len, sizeof(len), 0);
    len = ntohl(len);

    // if password.length() == 0, we should not recv more, otherwise it will stall and wait.
    if (len == 0) {
        output = "";
        return 0;
    }

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

int parseRequest(const std::string &requestMsg, std::string &opCode, std::string &roomcode) {
    if (requestMsg.length() == 0) {
        std::cout << "requestMsg Error!" << std::endl;
        return ERROR_FLAG;
    }

    std::istringstream iss(requestMsg);
    std::getline(iss, opCode, ':');
    std::getline(iss, roomcode, ':');
    std::cout << "opCode: " << opCode << " roomcode: " << roomcode << std::endl;
    return 0;
}

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

    // load the member data into memory.
    std::unordered_map<std::string, std::string> credentials;
    loadMember(credentials);

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
    // distinguish the userType
    UserType userType = encryptPassword.length() == 0 ? GUEST : MEMBER;

    recvAuthMessage(serverChildSocketFD, encryptUsername);
    recvAuthMessage(serverChildSocketFD, encryptPassword);
    // member!!! need if
    // need distinguish UserType and Request Type
    if (MEMBER == userType) 
        std::cout << "The main server received the authentication for " << encryptUsername << " using TCP over port " << TCP_PORT << std::endl;
    else {
        std::cout << "The main server received the guest request for " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
        std::cout << "The main server accepts " << encryptUsername << " as a guest" << std::endl;
    }
        
    // std::cout << "read username: " << encryptUsername << std::endl;
    // std::cout << "read password: " << encryptPassword << std::endl;

    // check Authentication and send response. Do with both guest and member.
    AuthCode resCode = checkAuth(credentials, encryptUsername, encryptPassword);
    std::string responseAuthMsg = authToString(resCode);
    // std::cout << "responseAuthMsg " << responseAuthMsg << std::endl;
    send(serverChildSocketFD, responseAuthMsg.data(), responseAuthMsg.length(), 0);
    std::cout << "The main server sent the authentication result to the client. " << std::endl;

    // only when one passes authentication, continue to execute the code.
    // Maybe change to while.
    if (resCode == WRONG_USER || resCode == WRONG_PASS) return 0;

    char buffer[1024] = {};
    
    while (true) {
        memset(&buffer, 0, sizeof(buffer));
        int recvFlag = recv(serverChildSocketFD, buffer, sizeof(buffer), 0);
        if (recvFlag <= 0) break;
        std::cout << "Data received from Client: " << buffer << std::endl;
        std::string opCode;
        std::string roomcode;
        parseRequest(buffer, opCode, roomcode);

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