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
    GUEST = 1,
    MEMBER = 2
};

int clientSocketInitialize();

bool checkValid(const std::string& input);

void encrypt(const std::string& input, std::string& output);

void displayAuth(const std::string responseMsgCode, const std::string username);

#endif