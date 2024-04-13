#include "serverU.h"
#define LOCAL_HOST "127.0.0.1"

std::string ResToString(Response resCode) {
    switch (resCode) {
        case AVAILABLE: return "600";
        case UNAVAILABLE: return "700";
        case NONEXISTENT: return "800";
        default: return "error";
    }
}

int loadRoomInfo(std::unordered_map<std::string, int> &roomInfoMap) {
    // load members from DB to data structure (memory).
    std::ifstream file("suite.txt");
    std::string line;

    if (!file) {
        std::cout << "cannot open file suite.txt" << std::endl;
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
    std::cout << "serverU bootup" << std::endl;

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

    std::string roomStatus = "roomU";
    // send message to serverM
    sendto(udpSocketFD, roomStatus.data(), roomStatus.length(), 0,
             (const struct sockaddr *) &serverMAddress, socklen);


    char buffer[MAXLINE];
    while (true) {
        int byteLen = recvfrom(udpSocketFD, (char*) buffer, MAXLINE, 0,
                (struct sockaddr*)&serverMAddress, &socklen);
        buffer[byteLen] = '\0';
        std::string req(buffer, byteLen);
        size_t pos = req.find(':');
        std::string opCode = req.substr(0, pos);
        std::string roomcode = req.substr(pos+1);
        std::string responseMsg = "0";  // 0 represent for roomstatus No Updated
        Response resCode;
        auto it = roomInfoMap.find(roomcode);
        int count = 0;
        if (it == roomInfoMap.end()) {
            resCode = NONEXISTENT;
        } else {
            count = it->second;
            if (count == 0) {
                resCode = UNAVAILABLE;
            } else {
                // count > 0
                resCode = AVAILABLE;
            }
        }
        if (opCode == "AVAILABILITY") {
            std::cout << "The Server U received an availability request from the main server." << std::endl;
            if (resCode == NONEXISTENT) {
                std::cout << "Not able to find the room layout." << std::endl;
            } else if (resCode == UNAVAILABLE) {
                std::cout << "Room " << roomcode << "is not available." << std::endl;
            } else {
                std::cout << "Room " << roomcode << " is available." << std::endl;
            }
        } else {
            std::cout << "The Server U received an reservation request from the main server." << std::endl;
            if (resCode == NONEXISTENT) {
                std::cout << "Cannot make a reservation. Not able to find the room layout." << std::endl;
            } else if (resCode == UNAVAILABLE) {
                std::cout << "Cannot make a reservation. Room " << roomcode << " is not available." << std::endl;
            } else {
                it->second -= 1;
                count -= 1;
                responseMsg = "1";
                std::cout << "Successful reservation. The count of Room "<< roomcode << " is now " << count << "." << std::endl;
            }
        }
        responseMsg = responseMsg + ResToString(resCode) + roomcode;
        // updatedOrNot + responseMsg + roomcode
        sendto(udpSocketFD, responseMsg.data(), responseMsg.length(), 0,
                (const struct sockaddr *)&serverMAddress, socklen);
        if (responseMsg.at(0) == '0')
            std::cout << "The Server U finished sending the response to the main server." << std::endl;
        else
            std::cout << "The Server U finished sending the response and the updated room status to the main server." << std::endl;
    }

    close(udpSocketFD);
    return 0;
}