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


void displayAuth(const std::string responseMsgCode, const std::string username) {
    std::string displayMsg;
    if (responseMsgCode == "100") {
        displayMsg = "Welcome guest " + username;
    } else if (responseMsgCode == "200") {
        displayMsg = "Username does not exist.";
    } else if (responseMsgCode == "300") {
        displayMsg = "Password does not match.";
    } else if (responseMsgCode == "400") {
        displayMsg = "Welcome member " + username;
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
    int userType = GUEST;
    while (true) {
        std::cout << "Please enter the username: ";
        std::getline(std::cin, username);
        if (!username.empty()) break;
        // add format check.
        std::cout << " Username cannot be empty. Please try again. To be removed." << std::endl;
    }
    
    std::cout << "Please enter the password (Press “Enter” to skip for guest): ";
    std::getline(std::cin, password);
    if(password.empty())
        userType = GUEST; 
    else
        userType = MEMBER;
    

    // get local Socket Address (PORT)
    sockaddr_in localSocketAddress;
    socklen_t socketLen = sizeof(localSocketAddress);
    memset(&localSocketAddress, 0, sizeof(localSocketAddress));
    if(ERROR_FLAG == getsockname(clientSocketFD, (struct sockaddr *)&localSocketAddress, &socketLen)) {
        std::cout << "getsockname failed. To be removed" << std::endl;
        return ERROR_FLAG;
    }
    int localPort = ntohs(localSocketAddress.sin_port);

    if (userType == GUEST) {
        std::cout << username <<" sent a guest request to the main server using TCP over port " << localPort << std::endl;
    } else if (userType == MEMBER) {
        std::cout << username <<" sent an authentication request to the main server." << std::endl;
    }

    std::string encryptUsername, encryptPassword;
    encrypt(username, encryptUsername);
    encrypt(password, encryptPassword);
    // std::cout << "after encrypting username:" << encryptUsername << std::endl;
    // std::cout << "after encrypting password:" << encryptPassword << std::endl;

    // Solving Problem about 'TCP no Boundary'.
    uint32_t dataLen = htonl(encryptUsername.length());
    send(clientSocketFD, &dataLen, sizeof(dataLen), 0);    
    send(clientSocketFD, encryptUsername.data(), encryptUsername.length(), 0);
    dataLen = htonl(encryptPassword.length());
    send(clientSocketFD, &dataLen, sizeof(dataLen), 0);
    send(clientSocketFD, encryptPassword.data(), encryptPassword.length(), 0);


    // receive authentication Msg
    char buffer[1024] = {};
    std::string res;
    recv(clientSocketFD, buffer, 1024, 0);
    // std::cout << buffer << " buffer size:" << strlen(buffer) << std::endl;
    displayAuth(buffer, username);
    
    
    while (true) {
        // send request
        
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