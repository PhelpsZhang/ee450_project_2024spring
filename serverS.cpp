#include "serverS.h"
#include <iostream>

#define PORT 8888
#define MAXLINE 1024

void handleRequest(){
    char buffer[MAXLINE];
    //const char *prompt = "Hello from server S";
    sockaddr_in serverAddress, clientAddress;

    int serverSSocket = socket(AF_INET, SOCK_DGRAM, 0);
    // clear possible random garbage data.
    memset(&serverAddress, 0, sizeof(serverAddress));
    memset(&clientAddress, 0, sizeof(clientAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    // bind
    bind(serverSSocket, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));

    int n;
    socklen_t len;
    len = sizeof(clientAddress);
    char res[20];
    while(1){
        n = recvfrom(serverSSocket, (char *)buffer, MAXLINE, MSG_WAITALL,
                 (struct sockaddr *) &clientAddress, &len);
        buffer[n] = '\0';
        std::cout << "Client: " << buffer << std::endl;
        if(buffer[0] == 'S') {
            strncpy(res, "Available", sizeof(res) - 1);
            res[sizeof(res) - 1] = '\0';
        } else {
            strncpy(res, "UnAvailable", sizeof(res) - 1);
            res[sizeof(res) - 1] = '\0';       
        }
        sendto(serverSSocket, (const char *)res, strlen(res), MSG_CONFIRM,
                (const struct sockaddr *) &clientAddress, len);
        std::cout << "result message sent:" << res << std::endl;
    }

}

int main(){
    std::cout << "serverS bootup" << std::endl;
    handleRequest();
    return 0;
}