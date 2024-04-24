#include "client.h"

#define FOREIGN_IP "127.0.0.1"

void encrypt(const std::string& input, std::string& output){
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

void encrypt_SHA256(const std::string &input, const std::string &salt, std::string &output) {
    // output store, 32 bytes
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    // Update sha256, 
    SHA256_Update(&sha256, input.data(), input.length());
    // with Salt
    SHA256_Update(&sha256, salt.data(), salt.length());
    // store to hash[]
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    output = ss.str();
}

bool receiveAuth(const std::string responseMsgCode, const std::string username) {
    std::string displayMsg;
    if (responseMsgCode == "100") {
        displayMsg = "Welcome guest " + username + "!";
        std::cout << displayMsg << std::endl;
        return true;
    } else if (responseMsgCode == "200") {
        displayMsg = "Failed login: Username does not exist.";
    } else if (responseMsgCode == "300") {
        displayMsg = "Failed login: Password does not match.";
    } else if (responseMsgCode == "400") {
        displayMsg = "Welcome member " + username + "!";
        std::cout << displayMsg << std::endl;
        return  true;
    }
    std::cout << displayMsg << std::endl;
    return false;
}


void receiveResp(const std::string responseMsgCode, std::string opCode, int localPort, std::string roomcode) {
    std::string displayMsg;
    if (responseMsgCode == "500") {
        displayMsg = "Permission denied: Guest cannot make a reservation.\n\n-----Start a new request-----";
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
        std::cerr << "Socket FD Failed." << std::endl;
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
        std::cerr << "Connect Failed." << std::endl;
        return ERROR_FLAG;
    }
    
    // get local Socket Address (PORT)
    sockaddr_in localSocketAddress;
    socklen_t socketLen = sizeof(localSocketAddress);
    memset(&localSocketAddress, 0, sizeof(localSocketAddress));
    if(ERROR_FLAG == getsockname(clientSocketFD, (struct sockaddr *)&localSocketAddress, &socketLen)) {
        std::cerr << "Getsockname Failed." << std::endl;
        return ERROR_FLAG;
    }
    int localPort = ntohs(localSocketAddress.sin_port);

    // type in the username and password
    std::string username, password;
    UserType userType = GUEST;
    while (true) {
        std::cout << "Please enter the username: ";
        std::getline(std::cin, username);
        std::cout << "Please enter the password (Press “Enter” to skip for guest): ";
        std::getline(std::cin, password);
        // Check input constraint. Robust.
        if (username.length() < 5 || username.length() > 50) {
            std::cerr << "Username length should be between 5 and 50 characters." << std::endl;
            continue;
        }
        bool isLowerFlag = false;
        for (char c : username) {
            if (!std::islower(c)) {
                isLowerFlag = true;
            }
        }
        if (isLowerFlag) {
            std::cerr << "Username string include non-lower case characters." << std::endl;
            continue;
        }
        if (password.length() != 0 && (password.length() < 5 || password.length() > 50)) {
            std::cerr << "Password length should be between 5 and 50 characters OR you can skip it." << std::endl;
            continue;
        }

        /* Encrypt
        * Salt should be randomly generated and stored in the database along with user information.
        * However, for simplicity here,
        * Using the original shif-by-3 encrypt() to 'pretend' to generate a salt value.
        */
        std::string encryptUsername, encryptPassword;
        std::string salt;
        encrypt(username, salt);
        encrypt_SHA256(username, salt, encryptUsername);
        encrypt_SHA256(password, salt, encryptPassword);

        // Solving Problem about 'TCP no Boundary'.
        // Potential Another way: The first n characters are designated as markers.
        uint32_t dataLen = htonl(encryptUsername.length());
        send(clientSocketFD, &dataLen, sizeof(dataLen), 0);    
        send(clientSocketFD, encryptUsername.data(), encryptUsername.length(), 0);

        if(password.empty()) {
            dataLen = htonl(password.length());
            send(clientSocketFD, &dataLen, sizeof(dataLen), 0);
            userType = GUEST; 
            std::cout << username <<" sent a guest request to the main server using TCP over port " << localPort << std::endl;
        } else {
            dataLen = htonl(encryptPassword.length());
            send(clientSocketFD, &dataLen, sizeof(dataLen), 0);
            send(clientSocketFD, encryptPassword.data(), encryptPassword.length(), 0);
            userType = MEMBER;
            std::cout << username <<" sent an authentication request to the main server." << std::endl;
        }

        // receive authentication Msg
        char buffer[1024] = {};
        std::string res;
        recv(clientSocketFD, buffer, 1024, 0);
        // std::cout << buffer << " buffer size:" << strlen(buffer) << std::endl;
        bool authRes = receiveAuth(buffer, username);

        if (authRes) {
            break;
        }
    }
    

    std::string roomCode;
    std::string opCode;

    while (true) {
        std::cout << "Please enter the room code:";
        std::getline(std::cin, roomCode);

        if (roomCode.length() == 0) {
            std::cerr << "Warning: Invalid roomCode!" << std::endl;
            continue;
        } else if (std::islower(roomCode.at(0))){
            std::cerr << "Warning: Invalid roomCode!" << std::endl;
            continue;
        }

        std::cout << "Would you like to search for the availability or make a reservation?(Enter \"Availability\" to search for the availability or Enter \"Reservation\" to make a reservation):";
        std::getline(std::cin, opCode);

        if (opCode.length() == 0) {
            std::cerr << "Warning: Invalid Request Input!" << std::endl;
            continue;  
        } else if (opCode != "Availability" && opCode != "Reservation"){
            std::cerr << "Warning: Invalid Request Input!" << std::endl;
            continue;  
        }

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
        if (byteLen < 0) {
            std::cerr << "recv Error" << std::endl;
        }
        recvRes[byteLen] = '\0';
        std::string responseMsgCode(recvRes, byteLen);
        receiveResp(responseMsgCode, opCode, localPort, roomCode);

    }

    close(clientSocketFD);
    return 0;
}

int main(){
    std::cout << "Client is up and running." << std::endl;
    clientSocketInitialize();
    return 0;
}