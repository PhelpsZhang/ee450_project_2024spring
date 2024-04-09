#ifndef SERVER_M_H
#define SERVER_M_H

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

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <iostream>

enum {
    TCP_PORT = 45089,
    UDP_PORT = 44089,
    TARGET_UDP_PORT = 41089,// target UDP S server
    QUEUE_LIMIT = 5,
    MAXLINE = 1024,
    ERROR_FLAG = -1
};

enum AuthCode {
    GUEST_SUCCESS,     // GUEST 
    WRONG_USER, // username doesnot exist
    WRONG_PASS,  // password doesnot match
    MEMBER_SUCCESS    // MEMBER login success
};

enum UserType {
    MEMBER,
    GUEST
};

enum RequestType {
    AVAILABILITY,
    RESERVATION
};

int recvMessage(int socketFD, std::string &output);

int loadMember(std::unordered_map<std::string, std::string> &result);

AuthCode checkAuth(const std::unordered_map<std::string, std::string> &map, std::string username, std::string password);

std::string authToString(AuthCode code);

int forwardToBackendServer(const char*);

int parseRequest(std::string &requestMsg, std::string &opCode, std::string &roomcode);

#endif