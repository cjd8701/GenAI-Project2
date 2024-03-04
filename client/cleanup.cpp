#include <windows.h>
#include <iostream>
#include <strsafe.h>

#include "modules.h"

// Function prototypes
bool DeleteRegistryKey();
void SelfDelete();

int cleanup() {
    if (!DeleteRegistryKey()) {
        std::cerr << "Failed to delete registry key." << std::endl;
    } else {
        std::cout << "Registry key deleted successfully." << std::endl;
    }

    SelfDelete();
    return 0;
}

bool DeleteRegistryKey() {
    // Open the registry key
    HKEY hKey;
    LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
                                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                                0, KEY_WRITE, &hKey);
    if (lResult != ERROR_SUCCESS) {
        std::cerr << "Error opening registry key." << std::endl;
        return false;
    }

    // Delete the value
    lResult = RegDeleteValue(hKey, TEXT("Client.exe"));
    RegCloseKey(hKey);

    if (lResult != ERROR_SUCCESS) {
        std::cerr << "Error deleting registry value." << std::endl;
        return false;
    }

    return true;
}

void SelfDelete() {
    TCHAR szModuleName[MAX_PATH];
    TCHAR szCmd[2 * MAX_PATH];
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};

    // Get the current executable's path
    GetModuleFileName(NULL, szModuleName, MAX_PATH);

    // Create command line for cmd.exe to delete the executable
    StringCchPrintf(szCmd, 2 * MAX_PATH, TEXT("cmd.exe /C timeout /t 3 & del /f /q \"%s\""), szModuleName);

    // Fill the STARTUPINFO structure
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Create the process
    if (!CreateProcess(NULL, szCmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "Self-deletion failed." << std::endl;
        return;
    }

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Exit the application
    ExitProcess(0);
}
