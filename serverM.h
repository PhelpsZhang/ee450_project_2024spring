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

int serverSocketInitialize();

int recvMessage(int socketFD, std::string &output);

int loadMember();

int forwardToBackendServer(const char*);

#endif