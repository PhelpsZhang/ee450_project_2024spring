#include "serverD.h"
#define LOCAL_HOST "127.0.0.1"

int loadRoomInfo(std::unordered_map<std::string, int> &roomInfoMap) {
    // load members from DB to data structure (memory).
    std::ifstream file("double.txt");
    std::string line;

    if (!file) {
        std::cout << "cannot open file double.txt" << std::endl;
        return ERROR_FLAG;
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string roomcode, countStr;
        if (std::getline(iss, roomcode, ',')) {
            iss >> std::ws;
            std::getline(iss, countStr);
            try {
                int count = std::stoi(countStr);
                roomInfoMap[roomcode] = count;
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid number: " << countStr << std::endl;
            } catch (const std::out_of_range& e) {
                std::cerr << "Number out of range: " << countStr << std::endl;
            }
        } else {
            std::cout << "format wrong OR file end." << std::endl;
        }
    }

    file.close();

    
    for (auto it = roomInfoMap.begin(); it != roomInfoMap.end(); ++it) {
        std::cout << "roomcode:" << it->first << " count:" << it->second << std::endl;
    }
    return 0;
}

int main(){
    std::cout << "serverD bootup" << std::endl;

    std::unordered_map<std::string, int> roomInfoMap;
    loadRoomInfo(roomInfoMap);

    int udpSocketFD = socket(AF_INET, SOCK_DGRAM, 0);

    // build sockaddr structure.
    sockaddr_in serverAddress, serverMAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, LOCAL_HOST, &(serverAddress.sin_addr));

    // bind
    bind(udpSocketFD, (const struct sockaddr *)&serverAddress, sizeof(serverAddress));

    memset(&serverMAddress, 0, sizeof(serverMAddress));
    serverMAddress.sin_family = AF_INET;
    serverMAddress.sin_port = htons(REMOTE_UDP_PORT);
    inet_pton(AF_INET, LOCAL_HOST, &(serverMAddress.sin_addr));
    socklen_t socklen = sizeof(serverMAddress);

    std::string roomStatus = "roomD";
    // send message to serverM
    sendto(udpSocketFD, roomStatus.data(), roomStatus.length(), 0,
             (const struct sockaddr *) &serverMAddress, socklen);

    // int n;
    // socklen_t len;
    // len = sizeof(clientAddress);
    // char res[20];
    // while(1){
    //     n = recvfrom(serverSSocket, (char *)buffer, MAXLINE, 0,
    //              (struct sockaddr *) &clientAddress, &len);
    //     buffer[n] = '\0';
    //     std::cout << "Client: " << buffer << std::endl;
    //     if(buffer[0] == 'S') {
    //         strncpy(res, "Available", sizeof(res) - 1);
    //         res[sizeof(res) - 1] = '\0';
    //     } else {
    //         strncpy(res, "UnAvailable", sizeof(res) - 1);
    //         res[sizeof(res) - 1] = '\0';       
    //     }
    //     sendto(serverSSocket, (const char *)res, strlen(res), 0,
    //             (const struct sockaddr *) &clientAddress, len);
    //     std::cout << "result message sent:" << res << std::endl;
    // }

    close(udpSocketFD);
    return 0;
}