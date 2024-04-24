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
------------------------------------------
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
        std::cerr << "Cannot open file member_unencrypted.ext" << std::endl;
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
            std::cerr << "Error: Line format incorrect, expected username and password separated by a comma." << std::endl;
        }
    }

    file.close();
    
    // for (auto it = credentials.begin(); it != credentials.end(); ++it) {
    //     std::cout << "username:" << it->first << std::endl;
    //     std::cout << "password:" << it->second << std::endl;
    // }
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
        std::cerr << "socket receive error OR connection down" << std::endl;
    }
    delete[] buffer;
    return 0;
}

int parseRequest(const std::string &requestMsg, RequestType &reqCode, std::string &roomcode) {
    if (requestMsg.length() == 0) {
        std::cerr << "requestMsg Error!" << std::endl;
        return ERROR_FLAG;
    }

    std::string opCode;
    std::istringstream iss(requestMsg);
    std::getline(iss, opCode, ':');
    std::getline(iss, roomcode, ':');
    // std::cout << "opCode: " << opCode << " roomcode: " << roomcode << std::endl;

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
            std::cerr << "recv roomstatus from UDP failed." << std::endl;
            break;
        }
        buffer[recvSize] = '\0';
        std::string receivedData(buffer, recvSize);
        char serverTag = receivedData.back();
        std::cout << "The main server has received the room status from Server " << serverTag << " using UDP over port " << UDP_PORT << "." << std::endl;
        forwardTable.insert({receivedData.back(), udpUnknownAddress});
        recvNum += 1;
    }

    // for (auto x : forwardTable) {
    //     std::cout << "code: " << x.first << " sockPort: " << ntohs(x.second.sin_port) << std::endl;
    // }
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
        std::cerr << "UDP Socket bind Failed." << std::endl;
        return ERROR_FLAG;
    }

    // forward table build.
    std::unordered_map<char, sockaddr_in> forwardTable;
    forwardTableBuild(udpSocketFD, forwardTable);
    // manually build a forward table list. A little bit cheat.

    // messege queue setup
    const char* QUEUE_NAME_BASE = "/UDP_RES_";

    pid_t pid = fork();
    if (pid == 0) {
        // child UDP process
        // Be responsible for receiving the Data from backend Server and sending to Child Process(by MQ).
        sockaddr_in recvSocketAddress;
        while (true) {
            char buffer[MAXLINE];
            memset(&recvSocketAddress, 0, sizeof(recvSocketAddress));
            socklen_t recvLen = sizeof(recvSocketAddress);
            int n = recvfrom(udpSocketFD, (char *)buffer, MAXLINE, 0,
                    (struct sockaddr *)&recvSocketAddress, &recvLen);
            buffer[n] = '\0';
            std::string recvMsg(buffer, n);
            // determine to mq_send to which MQ.
            // rescode:roomcode:queueName
            size_t pos = recvMsg.find('/');
            std::string mqName = recvMsg.substr(pos);
            mqd_t mq = mq_open(mqName.data(), O_CREAT | O_RDWR, 0666, NULL);
            if (mq == (mqd_t)-1) {
                std::cerr << "UDP process MQ open fail." << std::endl;
                perror("mq_open");
                mq_close(mq);
                mq_unlink(mqName.data());
                break;
            }
            if (-1 == mq_send(mq, recvMsg.data(), recvMsg.length(), 0)) {
                std::cerr << "UDP process MQ send fail." << std::endl;
                perror("mq_send");
                mq_close(mq);
                mq_unlink(mqName.data());
                break;
            }
            // std::cout << "UDP response sent to MQ successfully." << std::endl;
            // UDP process isnot responsibile for mq_unlink
            mq_close(mq);
        }
        close(udpSocketFD);
        exit(0);
    } else if (pid < 0) {
        perror("Fork Failed");
    }
    // parent (Main Process)
    // close(udpSocketFD);***************************
    // create TCP socket;
    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(ERROR_FLAG == serverSocketFD) {
        std::cerr << "TCP Socket FD creation Failed." << std::endl;
        return ERROR_FLAG;
    }

    // Avoid bind failed. Allow bind() in TIME_WAIT status.
    int yes = 1;
    if (setsockopt(serverSocketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        std::cerr << "Failed to set SO_REUSEADDR." << std::endl;
        close(serverSocketFD);
        return -1;
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
        std::cerr << "TCP Socket bind Failed." << std::endl;
        return ERROR_FLAG;
    }

    // listening to assigned socket
    // add error check
    if (ERROR_FLAG == listen(serverSocketFD, QUEUE_LIMIT)) {
        std::cerr << "TCP Socket listen Failed." << std::endl;
        return ERROR_FLAG;  
    }
    
    sockaddr_in clientSocketAddress;
    socklen_t clientAddrLen = sizeof(clientSocketAddress);
    memset(&clientSocketAddress, 0, clientAddrLen);

    int childNumber = 0;
    while (true) {
        // main accept loop.
        int serverChildSocketFD = accept(serverSocketFD, (struct sockaddr *)&clientSocketAddress, &clientAddrLen);
        // child socket created, receiving data
        if (ERROR_FLAG == serverChildSocketFD) {
            std::cerr << "TCP Socket accept Failed." << std::endl;
            return ERROR_FLAG;       
        }
        childNumber += 1;
        pid_t pid = fork();

        if (pid == 0) {
            // child process.
            // Close parent socket. It doesn't have to keep listening.
            close(serverSocketFD);

            std::string queueName = QUEUE_NAME_BASE + std::to_string(getpid());
            // std::cout << "Pid " << getpid() << "Set queueName: " << queueName << std::endl;

            std::string encryptUsername, encryptPassword;
            UserType userType = GUEST;
            while (true) {
                encryptUsername.clear();
                encryptPassword.clear();
                recvAuthMessage(serverChildSocketFD, encryptUsername);
                recvAuthMessage(serverChildSocketFD, encryptPassword);

                // distinguish the userType
                userType = encryptPassword.length() == 0 ? GUEST : MEMBER;
                // member
                AuthCode resCode = checkAuth(credentials, encryptUsername, encryptPassword);
                std::string responseAuthMsg = authToString(resCode);
                send(serverChildSocketFD, responseAuthMsg.data(), responseAuthMsg.length(), 0);
                // check Authentication and send response. Do with both guest and member.
                if (MEMBER == userType) {
                    std::cout << "The main server received the authentication for " << encryptUsername << " using TCP over port " << TCP_PORT << std::endl;
                    std::cout << "The main server sent the authentication result to the client. " << std::endl;
                    if (resCode == MEMBER_SUCCESS) break;
                } else {
                    std::cout << "The main server received the guest request for " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
                    std::cout << "The main server accepts " << encryptUsername << " as a guest" << std::endl;
                    break;
                }
            }

            char buffer[MAXLINE*8] = {};
            while (true) {
                // Start Request Loop
                memset(&buffer, 0, MAXLINE);
                int reqFlag = recv(serverChildSocketFD, buffer, MAXLINE, 0);
                if (reqFlag <= 0) break;
                // std::cout << "Data received from Client: " << buffer << std::endl;
                std::string opCode;
                std::string roomcode;
                RequestType reqType;
                parseRequest(buffer, reqType, roomcode);
                
                std::string msg2client, data2server, server2M;
                char respMsg[MAX_MSGSIZE];
                sockaddr_in targetSockaddr;
                memset(&targetSockaddr, 0, sizeof(targetSockaddr));
                char rc = roomcode.at(0);
                if (rc != 'S' && rc != 'D' && rc != 'U') {
                    // do not forward to backedn server
                    // define myself message.
                    if (reqType == AVAILABILITY) {
                        std::cout << "The main server has received the availability request on Room " << roomcode << " from " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
                    } else {
                        std::cout << "The main server has received the reservation request on Room " << roomcode << " from " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
                    }
                    std::string res2client = "800";
                    send(serverChildSocketFD, res2client.data(), res2client.length(), 0);
                    if (reqType == RESERVATION) {
                        std::cout << "The main server sent the reservation result to the client." << std::endl;
                    } else {
                        std::cout << "The main server sent the availability information to the client." << std::endl;
                    }
                    continue;
                }
                targetSockaddr = forwardTable[roomcode.at(0)];
                socklen_t socklen = sizeof(targetSockaddr);
                // forward to UDP
                // data: requestType + roomcode
                // ONLY AVAI request and MEMBER's RESERVE request will be forwarded to Backend Server.
                if (reqType == AVAILABILITY) {
                    std::cout << "The main server has received the availability request on Room " << roomcode << " from " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
                    // forward to Backend Server.
                    data2server = "AVAILABILITY:" + roomcode + ":" + queueName;
                    sendto(udpSocketFD, data2server.data(), data2server.length(), 0,
                        (const struct sockaddr *) &targetSockaddr, socklen);
                    std::cout << "The main server sent a request to Server " << roomcode.at(0) << "." << std::endl;

                } else {
                    // reqType == RESERVATION
                    std::cout << "The main server has received the reservation request on Room " << roomcode << " from " << encryptUsername << " using TCP over port " << TCP_PORT << "." << std::endl;
                    if (userType == GUEST) {
                        // permission denied.
                        std::cout << encryptUsername << " cannot make a reservation." << std::endl;
                        std::cout << "The main server sent the error message to the client." << std::endl;
                    } else {
                        // userType == MEMBER
                        data2server = "RESERVATION:" + roomcode + ":" + queueName;
                        ssize_t sentBytes= sendto(udpSocketFD, data2server.data(), data2server.length(), 0,
                        (const struct sockaddr *) &targetSockaddr, socklen);
                        if (sentBytes == -1) {
                            perror("sendto failed");
                        }
                        std::cout << "The main server sent a request to Server " << roomcode.at(0) << "." << std::endl;
                    }
                }

                std::string respCode;
                std::string clientMqId;
                // recv from UDP backend server

                if (!(userType == GUEST && reqType == RESERVATION)) {

                    mqd_t mq = mq_open(queueName.data(), O_CREAT | O_RDWR, 0666, NULL);
                    if (mq == (mqd_t)-1) {
                        std::cerr << "TCP child process MQ open fail." << std::endl;
                        std::cerr << "Failed queueName: " << queueName << std::endl;
                        mq_close(mq);
                        perror("mq_open");
                        mq_unlink(queueName.data());
                        exit(1);
                    }

                    unsigned int priority;
                    ssize_t bytesRead;
                    bytesRead = mq_receive(mq, (char *)respMsg, MAX_MSGSIZE, &priority);
                    if (bytesRead < 0) {
                        perror("mq_receive");
                        mq_close(mq);
                        mq_unlink(queueName.data());
                        continue;
                    }
                    mq_close(mq);
                    mq_unlink(queueName.data());
                    
                    // else
                    std::string respMsgS(respMsg, bytesRead);
                    // roomcode contains the serverTag
                    std::istringstream iss(respMsgS);
                    std::getline(iss, respCode, ':');
                    std::getline(iss, clientMqId, ':');
                    // clientMqId shoud be equal to queueName
                    // std::cout << "recvMqId: " << clientMqId << " queueName: " << queueName << std::endl;
                    if (respCode == "600" && reqType == RESERVATION) {
                        std::cout << "The main server received the response and the updated room status from Server " << roomcode.at(0) << " using UDP over port " << UDP_PORT << "." << std::endl;
                        std::cout << "The room status of Room " << roomcode << " has been updated." << std::endl;
                        // send to client
                    } else {
                        std::cout << "The main server received the response from Server " << roomcode.at(0) << " using UDP over port " << UDP_PORT << "." << std::endl;
                    }

                } else {
                    // when no forward, no receive.
                    respCode = "500";
                    // to be altered
                }
                std::string res2client = respCode;
                // Maybe distinguish the send() and display.
                // send back to the client
                send(serverChildSocketFD, res2client.data(), res2client.length(), 0);
                if (reqType == RESERVATION) {
                    std::cout << "The main server sent the reservation result to the client." << std::endl;
                } else {
                    std::cout << "The main server sent the availability information to the client." << std::endl;
                }
            }
            exit(0);
            // exit the child process
        } else if (pid > 0){
            // parent process
            close(serverChildSocketFD);
        } else {
            perror("fork failed");
            close(serverChildSocketFD);
        }
    }
    close(udpSocketFD);
    close(serverSocketFD);
    return 0;
}