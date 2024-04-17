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

#include<cctype>
#include<iostream>
#include<string>
#include<sstream>
#include<iomanip>
#include<openssl/sha.h>
// #include <openssl/aes.h>

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
/*
    PERMISSION_DENIED: "500";
    AVAILABLE: "600";
    UNAVAILABLE: "700";
    NONEXISTENT: "800";
*/

int clientSocketInitialize();

bool checkValid(const std::string& input);

void encrypt(const std::string& input, std::string& output);

void encrypt_SHA256(const std::string &input, const std::string &salt, std::string &output);

bool receiveAuth(const std::string responseMsgCode, const std::string username);

void receiveResp(const std::string responseMsgCode, std::string opCode, int localPort, std::string roomcode);

#endif