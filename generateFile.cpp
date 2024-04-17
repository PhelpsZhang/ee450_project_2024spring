
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <openssl/sha.h>
#include <algorithm> // For std::remove_if

std::string trim_right(const std::string& str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    return std::string(str.begin(), it.base());
}

void encrypt(const std::string& input, std::string& output){
    output.clear();
    // implementation of encrpytion
    for (std::string::const_iterator it = input.begin(); it != input.end(); ++it) {
        char c = *it;
        if (isalpha(c)) {
            char base = isupper(c) ? 'A' : 'a';
            c = (c - base + 3) % 26 + base;
        } else if (isdigit(c)) {
            c = (c - '0' + 3) % 10 + '0';
        }
        output += c;
    }
}

void encrypt_SHA256(const std::string &input, const std::string &salt, std::string &output) {
    // output store, 32 bytes
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    // Update sha256, 
    SHA256_Update(&sha256, input.data(), input.length());
    // with Salt
    SHA256_Update(&sha256, salt.data(), salt.length());
    // store to hash[]
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    output = ss.str();
}

int main() {
    std::ifstream inFile("member_unencrypted.txt");
    std::ofstream outFile("member.txt");

    if (!inFile || !outFile) {
        std::cout << "cannot" << std::endl;
        return 1;
    }

    std::string line, prevLine;
    if (std::getline(inFile, prevLine)) {
        while (std::getline(inFile, line)) {
            std::istringstream iss(prevLine);
            std::string username, password;
            std::string eusername, epassword;
            std::string salt;
            if (std::getline(iss, username, ',')) {
                // iss.get();
                iss >> std::ws;
                std::getline(iss, password);
                password = trim_right(password);
            }
            encrypt(username, salt);
            encrypt_SHA256(username, salt, eusername);
            encrypt_SHA256(password, salt, epassword);
            outFile << eusername << ", " << epassword << std::endl;
            prevLine = line;
        }
        std::istringstream iss(prevLine);
        std::string username, password;
        std::string eusername, epassword;
        std::string salt;
        if (std::getline(iss, username, ',')) {
            // iss.get();
            iss >> std::ws;
            std::getline(iss, password);
            password = trim_right(password);
        }
        encrypt(username, salt);
        encrypt_SHA256(username, salt, eusername);
        encrypt_SHA256(password, salt, epassword);
        outFile << eusername << ", " << epassword;
    }


    inFile.close();
    outFile.close();

    return 0;
}