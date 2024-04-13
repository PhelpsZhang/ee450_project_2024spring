#ifndef CLIENT_H
#define CLIENT_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/wait.h>

#include <iostream>
#include <string>

enum {
    FOREIGN_PORT = 45089,
    ERROR_FLAG = -1,
};

enum UserType {
    GUEST,
    MEMBER
};

enum RequestType {
    AVAILABILITY,
    RESERVATION
};

enum Res2Client {
    REFUSE, // guest cannot make a reservation
    AVAILABLE,
    UNAVAILABLE,
    NONEXISTENT
};

int clientSocketInitialize();

bool checkValid(const std::string& input);

void encrypt(const std::string& input, std::string& output);

bool receiveAuth(const std::string responseMsgCode, const std::string username);

void receiveResp(const std::string responseMsgCode, std::string opCode, int localPort, std::string roomcode);

#endif