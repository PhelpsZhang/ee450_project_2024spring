# ee450_project_2024spring

## Student Information
Author: Penghao Zhang

USC Student ID: #1119537089

Email: penghaoz@usc.edu

## Submission

What I've done.

I fully met the project requirements, including an upgrade to the encryption mechanism as an additional bonus feature.

## Code File Description

## Format of Messages Exchanged

## Idiosyncrasy

I designed it this way: After establishing a UDP socket, I fork a process specifically for receiving response data from backend servers through the UDP socket. The main process is dedicated to continuously listening on the TCP port and creating child sockets. Whenever a child socket is established, I fork a subprocess to handle the specific transactions.  This approach is taken into consideration that receiving UDP data in multiple subprocesses through recvfrom() might lead to data confusion, where one process might receive UDP data that is not intended for it. 

The data received from the UDP socket by the process is precisely transmitted to the corresponding subprocess via a **Message Queue**.

## Code Reference

The code was not directly copied. I consulted various sources including C/C++ documentation and Beej's Guide to Network Programming. After fully understanding the material, I completed the project independently.

## Warning:

