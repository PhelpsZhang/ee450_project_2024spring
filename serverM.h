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

enum PORT {
    TCP_PORT = 45089,
    UDP_PORT = 44089,
    REMOTE_S_PORT = 41089,// target UDP S server
    REMOTE_D_PORT = 42089,
    REMOTE_U_PORT = 43089,
};

enum {
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
    GUEST,
    MEMBER
};

enum RequestType {
    AVAILABILITY,
    RESERVATION
};

int recvMessage(int socketFD, std::string &output);

int loadMember(std::unordered_map<std::string, std::string> &result);

AuthCode checkAuth(const std::unordered_map<std::string, std::string> &map, std::string username, std::string password);

std::string authToString(AuthCode code);

int forwardToBackendServer(int udpSocketFD, sockaddr_in &sockaddr, const std::string &roomcode,  const RequestType &reqCode, UserType userType);

int parseRequest(std::string &requestMsg, RequestType &reqCode, std::string &roomcode);

#endif