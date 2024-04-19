# ee450_project_2024spring

## Student Information
Author: Penghao Zhang

USC Student ID: #1119537089

Email: penghaoz@usc.edu

## What I've done.

I fully met the project basic requirements.

I also completed the **Optional Part**: an upgrade to the encryption mechanism as an additional bonus feature. Using the SHA-256 hash function, usernames and passwords are encrypted in a one-way, irreversible manner.

Some "**Optional**" parts can be ignored, they are just for personal git repository explanation.

## Code File Description

### Basic file

- client.h / client.cpp 
  - Implements an interactive client that includes user login, information encryption, request sending, and receiving.

- serverM.h / serverM.cpp
  - Implemented the functionality of serverM. Briefly, it receives login requests from the multiple clients and performs user verification; it also receives subscription and query requests from the client and forwards them to the backend servers S/D/U, returning the request results to the client.

- serverS.h / serverS.cpp
  - Implemented the functionality of the backend server. It receives requests from serverM and carries out specific business processes for subscribing to/querying rooms.

- serverD.h / serverD.cpp
  - Refer to the above serverS.h/serverS.cpp.

- serverU.h / serverU.cpp
  - Refer to the above serverS.h/serverS.cpp.

- Makefile
  - Configured the program's compilation and linking. Supports compiling each program individually using `make xxx`, as well as using `make` to compile all programs at once. Supports `make clean` to remove compilation results and intermediate files.

- member.txt
  - Stored user data encrypted using the **SHA256 hashing algorithm**.
  - Uses the SHA256 hashing algorithm to encrypt the original user authentication data in the file `member_unencrypted.txt`, and saves it in the same format in this file member.txt.

- member_unencrypted.txt
  - Unencrypted original user data file. 
  - I personally changed the first letter to lowercase as required, and the data saved in `member.txt` is also the encrypted data after being changed to lowercase.

### Additional (Optional): 

- single.txt / double.txt / suite.txt
  - Includes the status information of the room for the backend servers to read.

- generateFile.cpp
  - Just for personal use!
  - Reads the original user data from member_encrypted.txt, encrypts it, and writes it back to member.txt in the same format.

- run_test.sh
  - Optional. Just for personal test script. 
  - To check if the preset TCP/UDP ports are occupied and to close the occupying programs to ensure the program runs smoothly. 
  - Then it sequentially starts serverM, serverS, serverD, serverU, client, and client, each named respectively, with the two client programs being divided into clientA and clientB.


## Extra Credit - Encryption

A **salt** is an additional piece of data used to enhance the complexity of a password, thereby increasing its security.

In the project, the original `encrypt()` function is used to perform simple encryption on the username to obtain a salt. Then, the **SHA256** hashing algorithm uses this salt to encrypt both the username and password. This results in a one-way, irreversible hash value suitable for password storage.

```cpp
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
```

For example.

There is a user information in `member_unencrypted.txt`:
```txt
james, SODids392
```

After encrypting, it becomes:
```txt
c44b7599b01449d9f3d326917efa26ca911fb46d42532b5852bda1386d0b0c0f, bc2e0007d1a5be031e2bf5f866facec2020bb757bda962cd38e3d663db0073f1
```

## How to Run the Project.

### 1. Compile

```bash
make
```
Execute `make` in the project folder to compile all the programs and obtain the executable files serverM, serverS, serverD, serverU, and client.

---

**Optional**

This project utilizes three additional dynamic libraries(-lrt -lssl -lcrypto), which have been verified to already exist in the studentVM's Ubuntu environment.

In case the absence of dynamic libraries prevents compilation, you can try using the following command to install them.

```bash
# install librt 
sudo apt-get install libc6-dev
# install OpenSSL
sudo apt-get install libssl-dev
```

### 2. Run

**It is recommended to conduct tests using TA's own methods.**

---

**Optional**

If no personal method is available, consider using the provided script, `run_test.sh`. It terminates potential port occupations and sequentially starts and names the programs as required.

```bash
./run_test.sh
```

Once GNOME Terminal is not found, you can also install it.

```bash
sudo apt install gnome-terminal
```

## Format of Messages Exchanged

The format for information exchange in this project is not unified. There are different designs depending on the specific situation.

Generally speaking, the preset code is transmitted and then parsed on the receiving side.

### 1. For Authentication

1. Username and Password Sending

The initial encryption requirement was simply to shift positions. When potential delimiters are present in the password, it could lead to recognition errors. To avoid the issue of passwords being confused with delimiters, which leads to recognition errors, usernames and passwords are sent separately. To address the TCP TCP segment coalescing issue that this could cause, a scheme of 'sending the data length before sending the data content' was adopted.

After updating the encryption algorithm, the previous practices were continued without modification.

2. Response to Authentication

I preset the result codes: 100, 200, 300, 400, each representing different verification results. After verifying user login information, serverM returns these status codes to the client, which then interprets them.

`100`: Guest Login Success

`200`: Member Username doesnot exist

`300`: Member Password doesnot match

`400`: Member Login Success

### 2. For Room Request

1. Client Send Request to ServerM

The Client uses the format `requestType:roomCode` when sending a room request. The two strings are separated by colons.

For example,

Availability request for room S201:

`"Availability:S201"`

Reservation request for room U597:

`"Reservation:U597"`

2. ServerM forward Request to Backend Server S/D/U

ServerM forwards the request to the backend server using the format `requestType:roomCode:clientId`.

Since multiple clients make requests at the same time, serverM will fork() multiple child processes for processing. The UDP Socket cannot determine which client the data belongs to. Therefore, in order to solve the problem of inter-process communication, it is important to carry an **Identifier** that can identify the client in the data.

### 3. For Room Response

I preset different result codes, 500, 600, 700, 800 to represent different responses to the Client request.

`500`: Permission Denied

`600`: Room Available

`700`: Room Unavailable

`800`: Room Doesnot Exist

These result codes are combined into a standard format `resultCode:clientId` by the backend server and sent back to serverM.

At serverM, the sub-process that processes the corresponding client request is identified through the clientId, and the current username, requestType, and roomcode can be obtained.

serverM sends the corresponding result code back to the client, using `resultCode`.

Both client and serverM determine the processing result and print out the corresponding information by combining the result code and request type. 

## Project Design Overview

### 1. Multi-process Design

In this specific case, serverM has four processes working at the same time:

1: UDP Socket that continues to receive response data from the backend servers and forward the data to the corresponding child process;

2: Continue to listen to the TCP connection of the new client and establish a new child process;

3: Responsible for processing the child process of clientA;

4: Responsible for processing the child process of clientB.

I designed it this way: After establishing a UDP socket, I fork a process specifically for receiving response data from backend servers through the UDP socket. The main process is dedicated to continuously listening on the TCP port and creating child sockets. Whenever a child socket is established, I fork a subprocess to handle the specific transactions.  This approach is taken into consideration that receiving UDP data in multiple subprocesses through recvfrom() might lead to data confusion, where one process might receive UDP data that is not intended for it. 

The data received from the UDP socket by the process is precisely transmitted to the corresponding subprocess via a **Message Queue**.

I concatenate the queue name of the message queue with the pid() of the current child process, turn it into a client Identifier, send it to the back-end server and receive it back to serverM. In this way, the UDP Socket process can easily open the message queue leading to a specific sub-process and send information.

### 2. Encryption Algorithm

This project adopts SHA256 encryption for the username and password. It uses a simple shift-N encryption of the username as the 'salt'.

### 3. Robust

Efforts have been made to handle boundary conditions of user input, making the code more robust.

### 4. Some Constraints:

Because there are only 3 backend servers in this case, when initially sending roomstatus to serverM, I simply fixed the number of times serverM calls recvfrom() to 3. Rather than specifically fork() a child process to keep receiving new backend server connections dynamically.

## Code Reference

The code was not directly copied. I consulted various sources including C/C++ documentation and Beej's Guide to Network Programming. After fully understanding the material, I completed the project independently.