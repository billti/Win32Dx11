#pragma once

#include "common.h"
#include <string>

class DiagOutput
{
public:
    void CreateConsole() {
        // Further details: https://stackoverflow.com/questions/30422802/decide-an-application-to-be-of-console-windows-subsystem-at-run-time
        // FILE* stream;
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) AllocConsole();
        //freopen_s(&stream, "CON", "r", stdin);
        //freopen_s(&stream, "CON", "w", stdout);
        //freopen_s(&stream, "CON", "w", stderr);
        hasConsole = true;
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }

    void Log(const std::wstring& msg) {
        DWORD written = 0;
        if (hasConsole) {
            if (!WriteConsole(hStdOut, (void*)msg.c_str(), msg.length(), &written, nullptr)) {
                throw std::exception("Failed to write to console");
            }
        }
    }

private:
    bool hasConsole = false;
    HANDLE hStdOut = INVALID_HANDLE_VALUE;
};
