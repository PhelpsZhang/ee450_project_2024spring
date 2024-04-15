#ifndef SERVER_D_H
#define SERVER_D_H

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
#include<iostream>
#include<string>

enum {
    UDP_PORT = 42089,
    REMOTE_UDP_PORT = 44089,
    MAXLINE = 1024,
    ERROR_FLAG = -1
};

enum Response {
    AVAILABLE,
    UNAVAILABLE,
    NONEXISTENT
};

std::string ResToString(Response resCode);

int loadRoomInfo(std::unordered_map<std::string, int> &roomInfoMap);

#endif