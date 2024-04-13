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
            // iss.get();
            iss >> std::ws;
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

int parseRequest(const std::string &requestMsg, RequestType &reqCode, std::string &roomcode) {
    if (requestMsg.length() == 0) {
        std::cout << "requestMsg Error!" << std::endl;
        return ERROR_FLAG;
    }

    std::string opCode;
    std::istringstream iss(requestMsg);
    std::getline(iss, opCode, ':');
    std::getline(iss, roomcode, ':');
    std::cout << "opCode: " << opCode << " roomcode: " << roomcode << std::endl;

    if (opCode == "Availability") {
        reqCode = AVAILABILITY;
    } else {
        reqCode = RESERVATION;
    }

    return 0;
}

int forwardTableBuild(int udpSocketFD, std::unordered_map<char, sockaddr_in> &forwardTable) {
    sockaddr_in udpUnknownAddress;
    socklen_t uua_len; 
    std::string serverTag = "";
    char buffer[MAXLINE];
    int recvNum = 0;
    while (recvNum < ROOM_TYPE_NUM) {
        memset(&udpUnknownAddress, 0, sizeof(udpUnknownAddress));
        memset(buffer, 0, sizeof(buffer));
        uua_len = sizeof(udpUnknownAddress);
        int recvSize = recvfrom(udpSocketFD, (char*) buffer, MAXLINE, 0, (struct sockaddr *) &udpUnknownAddress, &uua_len);
        if (recvSize <= 0) {
            std::cout << "recv error" << std::endl;
            break;
        }
        buffer[recvSize] = '\0';
        std::string receivedData(buffer, recvSize);
        char serverTag = receivedData.back();
        std::cout << "The main server has received the room status from Server " << serverTag << " using UDP over port " << UDP_PORT << "." << std::endl;
        forwardTable.insert({receivedData.back(), udpUnknownAddress});
        recvNum += 1;
    }

    for (auto x : forwardTable) {
        std::cout << "code: " << x.first << " sockPort: " << ntohs(x.second.sin_port) << std::endl;
    }
    return 0;
}

int main() {
    std::cout << "The main server is up and running." << std::endl;

    // load the member data into memory.
    std::unordered_map<std::string, std::string> credentials;
    loadMember(credentials);

    // create udp Socket
    int udpSocketFD = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udpSocketAddress;
    memset(&udpSocketAddress, 0, sizeof(udpSocketAddress));
    udpSocketAddress.sin_family = AF_INET;
    udpSocketAddress.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, LOCAL_HOST, &(udpSocketAddress.sin_addr));

    // bind UDP socket
    if (ERROR_FLAG == bind(udpSocketFD, (const struct sockaddr *)&udpSocketAddress, sizeof(udpSocketAddress))) {
        std::cout << "UDP Socket bind Failed." << std::endl;
        return ERROR_FLAG;
    }

    // forward table build.
    std::unordered_map<char, sockaddr_in> forwardTable;
    forwardTableBuild(udpSocketFD, forwardTable);

    // manually build a forward table list. A little cheat.
    

    // create TCP socket;
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

    // distinguish the userType
    UserType userType = encryptPassword.length() == 0 ? GUEST : MEMBER;
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

    char buffer[1024] = {};
    
    if (resCode == WRONG_USER || resCode == WRONG_PASS) {
        std::cout << "Wrong Auth. maybe abnormal shut down." << std::endl;
    }

    while (true) {
        memset(&buffer, 0, sizeof(buffer));
        int reqFlag = recv(serverChildSocketFD, buffer, sizeof(buffer), 0);
        if (reqFlag <= 0) break;
        std::cout << "Data received from Client: " << buffer << std::endl;
        std::string opCode;
        std::string roomcode;
        RequestType reqType;
        parseRequest(buffer, reqType, roomcode);
        
        std::string msg2client, data2server, server2M;
        char respMsg[MAXLINE];
        sockaddr_in targetSockaddr;
        memset(&targetSockaddr, 0, sizeof(targetSockaddr));
        targetSockaddr = forwardTable[roomcode.at(0)];
        socklen_t socklen = sizeof(targetSockaddr);
        int byteLen;
        // forward to UDP
        // data: requestType + roomcode
        // ONLY AVAI request and MEMBER's RESERVE request will be forwarded to Backend Server.
        if (reqType == AVAILABILITY) {
            std::cout << "The main server has received the availability request on Room " << roomcode << " from " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
            // forward to Backend Server.
            data2server = "AVAILABILITY:" + roomcode;
            sendto(udpSocketFD, data2server.data(), data2server.length(), 0,
                   (const struct sockaddr *) &targetSockaddr, socklen);
            std::cout << "The main server sent a request to Server " << roomcode.at(0) << "." << std::endl;

        } else {
            // reqType == RESERVATION
            std::cout << "The main server has received the reservation request on Room " << roomcode << " from " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
            if (userType == GUEST) {
                // permission denied.
                std::cout << encryptUsername << " cannot make a reservation." << std::endl;
                //msg2client = "Permission denied: Guest cannot make a reservation.";
                
                // await to be altered.
                //send(serverChildSocketFD, msg2client.data(), msg2client.length(), 0);

                std::cout << "The main server sent the error message to the client." << std::endl;
            } else {
                // userType == MEMBER
                data2server = "RESERVATION:" + roomcode;
                sendto(udpSocketFD, data2server.data(), data2server.length(), 0,
                   (const struct sockaddr *) &targetSockaddr, socklen);
                std::cout << "The main server sent a request to Server " << roomcode.at(0) << "." << std::endl;
            }
        }

        char updateFlag;
        std::string respCode;
        std::string opRoomCode;
        // recv from UDP backend server
        if (!(userType == GUEST && reqType == RESERVATION)) {
            
            byteLen = recvfrom(udpSocketFD, (char *)respMsg, MAXLINE, 0,
                   (struct sockaddr*)&targetSockaddr, &socklen);
            respMsg[byteLen] = '\0';
            std::string respMsgS(respMsg, byteLen);
            // roomcode contains the serverTag
            updateFlag = respMsgS.at(0);
            respCode = respMsgS.substr(1, 3);
            opRoomCode = respMsgS.substr(4);
            if (updateFlag == '0') {
                std::cout << "The main server received the response from Server " << opRoomCode.at(0) << " using UDP over port " << UDP_PORT << "." << std::endl;
            } else {
                std::cout << "The main server received the response and the updated room status from Server " << opRoomCode.at(0) << " using UDP over port" << UDP_PORT << "." << std::endl;
                std::cout << "The room status of Room " << opRoomCode << " has been updated." << std::endl;
                // send to client
            }
        } else {
            // when no forward, no receive.
            updateFlag = '0';
            respCode = "500";
            opRoomCode = roomcode;
            // to be altered
        }
        std::string res2client = updateFlag + respCode + opRoomCode;
        // Maybe distinguish the send() and display.
        // send back to the client
        send(serverChildSocketFD, res2client.data(), res2client.length(), 0);
        if (reqType == RESERVATION) {
            std::cout << "The main server sent the reservation result to the client." << std::endl;
        } else {
            std::cout << "The main server sent the availability information to the client." << std::endl;
        }
        
        // add control logic to Which Server
        // forwardToBackendServer(roomcode, reqCode, userType,);
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

    // initializa UDP remote sockaddr
    // sockaddr_in udpSAddress, udpDAddress, udpUAddress;
    // memset(&udpSAddress, 0, sizeof(udpSAddress));
    // memset(&udpDAddress, 0, sizeof(udpDAddress));
    // memset(&udpUAddress, 0, sizeof(udpUAddress));
    // udpSAddress.sin_family = AF_INET;
    // udpSAddress.sin_port = htons(REMOTE_S_PORT);
    // inet_pton(AF_INET, LOCAL_HOST, &(udpSAddress.sin_addr));
    // udpDAddress.sin_family = AF_INET;
    // udpDAddress.sin_port = htons(REMOTE_D_PORT);
    // inet_pton(AF_INET, LOCAL_HOST, &(udpDAddress.sin_addr));
    // udpUAddress.sin_family = AF_INET;
    // udpUAddress.sin_port = htons(REMOTE_U_PORT);
    // inet_pton(AF_INET, LOCAL_HOST, &(udpUAddress.sin_addr));