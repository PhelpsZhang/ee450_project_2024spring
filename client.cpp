#include "client.h"

#define FOREIGN_IP "127.0.0.1"


bool checkValid(const std::string& input){
    // check if the format of username and password is valid.
    return true;
}

void encrypt(const std::string& input, std::string& output){
    // make sure it is empty. more robotic.
    output.clear();
    // implementation of encrpytion
    for (std::string::const_iterator it = input.begin(); it != input.end(); ++it) {
        char c = *it;
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c = (c - base + 3) % 26 + base;
        } else if (isdigit(c)) {
            c = (c - '0' + 3) % 10 + '0';
        }
        output += c;
    }
}


bool receiveAuth(const std::string responseMsgCode, const std::string username) {
    std::string displayMsg;
    if (responseMsgCode == "100") {
        displayMsg = "Welcome guest " + username;
        std::cout << displayMsg << std::endl;
        return true;
    } else if (responseMsgCode == "200") {
        displayMsg = "Failed login: Username does not exist.";
    } else if (responseMsgCode == "300") {
        displayMsg = "Failed login: Password does not match.";
    } else if (responseMsgCode == "400") {
        displayMsg = "Welcome member " + username;
        std::cout << displayMsg << std::endl;
        return  true;
    }
    std::cout << displayMsg << std::endl;
    return false;
}


void receiveResp(const std::string responseMsgCode, std::string opCode, int localPort, std::string roomcode) {
    std::string displayMsg;
    if (responseMsgCode == "500") {
        displayMsg = "Permission denied: Guest cannot make a reservation.";
    } else if (responseMsgCode == "600") {
        // success. avai or reserva?
        if (opCode == "Availability") 
            displayMsg = "The client received the response from the main server using TCP over port " + std::to_string(localPort) +
                        "\nThe requested room is available.\n\n" +
                        "-----Start a new request-----";
        else 
            displayMsg = "The client received the response from the main server using TCP over port " + std::to_string(localPort) + 
                        "\nCongratulation! The reservation for Room " + roomcode + " has been made.\n\n" +
                        "-----Start a new request-----";
    } else if (responseMsgCode == "700") {
        if (opCode == "Availability")
        displayMsg = "The client received the response from the main server using TCP over port " + std::to_string(localPort) + 
                        "\nThe requested room is not available.\n\n" +
                        "-----Start a new request-----";
        else 
            displayMsg = "The client received the response from the main server using TCP over port " + std::to_string(localPort) + 
                        "\nSorry! The requested room is not available.\n\n" +
                        "-----Start a new request-----";
    } else if (responseMsgCode == "800") {
        if (opCode == "Availability")
            displayMsg = "The client received the response from the main server using TCP over port " + std::to_string(localPort) + 
                        "\nNot able to find the room layout.\n\n" +
                        "-----Start a new request-----";
        else
            displayMsg = "The client received the response from the main server using TCP over port " + std::to_string(localPort) +  
                        "\nOops! Not able to find the room.\n\n" +
                        "-----Start a new request-----";
    }
    std::cout << displayMsg << std::endl;
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

int clientSocketInitialize(){

    // create TCP socket
    int clientSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(ERROR_FLAG == clientSocketFD) {
        std::cout << "Socket FD Failed." << std::endl;
        return ERROR_FLAG;
    }

    // prepare socket address structure
    sockaddr_in serverSocketAddress;
    memset(&serverSocketAddress, 0, sizeof(serverSocketAddress));
    serverSocketAddress.sin_family = AF_INET;
    serverSocketAddress.sin_port = htons(FOREIGN_PORT);
    inet_pton(AF_INET, FOREIGN_IP, &(serverSocketAddress.sin_addr));
    
    // connect() to serverM
    if (ERROR_FLAG == connect(clientSocketFD, (struct sockaddr*)&serverSocketAddress, sizeof(serverSocketAddress))) {
        std::cout << "Connect Failed." << std::endl;
        return ERROR_FLAG;
    }
    
    // type in the username and password
    std::string username, password;
    UserType userType = GUEST;
    while (true) {
        std::cout << "Please enter the username: ";
        std::getline(std::cin, username);
        if (!username.empty()) break;
        // add format check.
        std::cout << " Username cannot be empty. Please try again. To be removed." << std::endl;
    }
    
    std::cout << "Please enter the password (Press “Enter” to skip for guest): ";
    std::getline(std::cin, password);
    
    // get local Socket Address (PORT)
    sockaddr_in localSocketAddress;
    socklen_t socketLen = sizeof(localSocketAddress);
    memset(&localSocketAddress, 0, sizeof(localSocketAddress));
    if(ERROR_FLAG == getsockname(clientSocketFD, (struct sockaddr *)&localSocketAddress, &socketLen)) {
        std::cout << "getsockname failed. To be removed" << std::endl;
        return ERROR_FLAG;
    }
    int localPort = ntohs(localSocketAddress.sin_port);

    std::string encryptUsername, encryptPassword;
    encrypt(username, encryptUsername);
    encrypt(password, encryptPassword);
    // std::cout << "after encrypting username:" << encryptUsername << std::endl;
    // std::cout << "after encrypting password:" << encryptPassword << std::endl;

    // Solving Problem about 'TCP no Boundary'.
    // Potential Another way: The first n characters are designated as markers.
    uint32_t dataLen = htonl(encryptUsername.length());
    send(clientSocketFD, &dataLen, sizeof(dataLen), 0);    
    send(clientSocketFD, encryptUsername.data(), encryptUsername.length(), 0);
    dataLen = htonl(encryptPassword.length());
    send(clientSocketFD, &dataLen, sizeof(dataLen), 0);
    send(clientSocketFD, encryptPassword.data(), encryptPassword.length(), 0);

    if(password.empty()) {
        userType = GUEST; 
        std::cout << username <<" sent a guest request to the main server using TCP over port " << localPort << std::endl;
    } else {
        userType = MEMBER;
        std::cout << username <<" sent an authentication request to the main server." << std::endl;
    }

    // receive authentication Msg
    char buffer[1024] = {};
    std::string res;
    recv(clientSocketFD, buffer, 1024, 0);
    // std::cout << buffer << " buffer size:" << strlen(buffer) << std::endl;
    bool authRes = receiveAuth(buffer, username);
    
    if (!authRes) {
        std::cout << "Wrong auth, maybe abnormal shutdown." << std::endl;
    }

    std::string roomCode;
    std::string opCode = "Availability";

    while (true) {
        std::cout << "Please enter the room code:";
        std::getline(std::cin, roomCode);
        std::cout << "Would you like to search for the availability or make a reservation?(Enter \"Availability\" to search for the availability or Enter \"Reservation\" to make a reservation):";
        std::getline(std::cin, opCode);

        if (opCode.length() == 0 || roomCode.length() == 0)
            std::cout << "invalid Input" << std::endl;

        std::string RequestMsg = opCode + ":" + roomCode;
        if (opCode == "Availability") {
            // send an availability
            send(clientSocketFD, RequestMsg.data(), RequestMsg.length(), 0);
            std::cout << username << " sent an availability request to the main server." << std::endl;

        } else if (opCode == "Reservation") {

            if (userType == GUEST) {
                // Actually, we don't have to send request.
                std::cout << username << " sent a reservation request to the main server." << std::endl;
                // std::cout << "Perssion denied: Guest cannot make a reservation." << std::endl;
                send(clientSocketFD, RequestMsg.data(), RequestMsg.length(), 0);
            } else {
                // send an reservation
                send(clientSocketFD, RequestMsg.data(), RequestMsg.length(), 0);
                std::cout << username << " sent a reservation request to the main server." << std::endl;
            }
        }
        // send request
        char recvRes[1024] = {};
        int byteLen = recv(clientSocketFD, recvRes, 1024, 0);
        recvRes[byteLen] = '\0';
        std::string responseMsgCode(recvRes, byteLen);
        receiveResp(responseMsgCode.substr(0,3), opCode, localPort, roomCode);

    }

    // send data
    //const char* roomcode = "S102";
    //send(clientSocket, roomcode, strlen(roomcode), 0);

    // if (password.empty())
    // send(clientSocketFD, username)


    // std::string message;
    // while(1) {
    //     std::cout << "Enter a message: ";
    //     std::getline(std::cin, message);
    //     //if(message.empty() || std::cin.eof()) break;
    //     if(message == "quit") {break;};
    //     std::cout << "Final Send Message: " << message << std::endl;
    //     if (send(clientSocket, message.data(), message.size(), 0) == -1) {
    //         std::cout<<"xxx" <<std::endl;
    //         perror("send failed");
    //         break;
    //     }
    // }

    close(clientSocketFD);
    return 0;
}

int main(){
    std::cout << "Client is up and running." << std::endl;
    clientSocketInitialize();
    return 0;
}