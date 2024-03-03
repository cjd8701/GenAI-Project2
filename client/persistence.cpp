#include <iostream>
#include <string>
#include <windows.h>
#include "modules.h"

#include "modules.h"

bool AddToStartup() {
    // Get the full path of the executable
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        std::wcerr << L"Failed to get module file name." << std::endl;
        return false;
    }

    // Open the registry key
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

    // Set the value of the registry key
    // Replace "YourAppName" with a unique name for your application
    status = RegSetValueExW(hkey,
                            L"Client.exe",
                            0,
                            REG_SZ,
                            (const BYTE*)exePath,
                            (wcslen(exePath) + 1) * sizeof(wchar_t));

    RegCloseKey(hkey); // Always close the key when you're done

    if (status != ERROR_SUCCESS) {
        std::wcerr << L"Failed to set registry value." << std::endl;
        return false;
    }

    return true;
}