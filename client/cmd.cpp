#include "modules.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

std::string execCommand(const char* cmd) {
    std::string result;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
    saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0)) {
        std::cerr << "Create pipe failed\n";
        return "";
    }

    STARTUPINFOA si = {sizeof(STARTUPINFOA)};
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput = hPipeWrite;
    si.hStdError = hPipeWrite;
    si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = {0};

    BOOL bSuccess = CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    if (!bSuccess) {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        std::cerr << "Create process failed\n";
        return "";
    }

    // Don't need the write end of the pipe in the parent process
    CloseHandle(hPipeWrite);

    // Read output from the child process
    for (;;) {
        char buf[1024];
        DWORD dwRead;
        BOOL bSuccess = ReadFile(hPipeRead, buf, sizeof(buf) - 1, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) break;
        buf[dwRead] = '\0';
        result += buf;
    }

    // Close handles
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return result;
}

// int main() {
//     // Replace "cmd /c dir" with your command
//     std::string output = execCommand("cmd /c ipconfig");
//     std::cout << "Command Output:\n" << output;

//     return 0;
// }