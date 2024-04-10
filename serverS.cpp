#include "serverS.h"

int loadRoomInfo(std::unordered_map<std::string, int> &roomInfoMap) {
    // load members from DB to data structure (memory).
    std::ifstream file("single.txt");
    std::string line;

    if (!file) {
        std::cout << "cannot open file single.txt" << std::endl;
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
    std::cout << "serverS bootup" << std::endl;

    std::unordered_map<std::string, int> roomInfoMap;
    loadRoomInfo(roomInfoMap);
    
    char buffer[MAXLINE];
    //const char *prompt = "Hello from server S";
    sockaddr_in serverAddress, clientAddress;

    int serverSSocket = socket(AF_INET, SOCK_DGRAM, 0);
    // clear possible random garbage data.
    memset(&serverAddress, 0, sizeof(serverAddress));
    memset(&clientAddress, 0, sizeof(clientAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(UDP_PORT);

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
    return 0;
}