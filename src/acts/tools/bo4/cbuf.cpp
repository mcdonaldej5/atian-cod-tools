#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

#define GAME_EXE "BlackOps4.exe"
#define CBUF_ADDTEXT_OFFSET 0x3CDE880  // Offset for Cbuf_AddText

// Function to get the process ID of BlackOps4.exe
DWORD GetProcessID(const char* processName) {
    DWORD processID = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnap, &pe)) {
            do {
                if (_stricmp(pe.szExeFile, processName) == 0) {
                    processID = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return processID;
}

// Function to get the base address of BlackOps4.exe
uintptr_t GetModuleBaseAddress(DWORD processID, const char* moduleName) {
    uintptr_t baseAddress = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hSnap != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 me;
        me.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnap, &me)) {
            do {
                if (_stricmp(me.szModule, moduleName) == 0) {
                    baseAddress = (uintptr_t)me.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &me));
        }
        CloseHandle(hSnap);
    }
    return baseAddress;
}

// Function to execute a command in Black Ops 4 (For use in ImGui Button)
void ExecuteGameCommand(const char* command) {
    DWORD processID = GetProcessID(GAME_EXE);
    if (!processID) {
        std::cerr << "[ERROR] BlackOps4.exe not found!\n";
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        std::cerr << "[ERROR] Failed to open process!\n";
        return;
    }

    uintptr_t baseAddress = GetModuleBaseAddress(processID, GAME_EXE);
    uintptr_t cbufAddTextAddr = baseAddress + CBUF_ADDTEXT_OFFSET;

    // Allocate memory in the game process
    void* remoteMemory = VirtualAllocEx(hProcess, NULL, strlen(command) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteMemory) {
        std::cerr << "[ERROR] Memory allocation failed!\n";
        CloseHandle(hProcess);
        return;
    }

    // Write the command string into the allocated memory
    if (!WriteProcessMemory(hProcess, remoteMemory, command, strlen(command) + 1, NULL)) {
        std::cerr << "[ERROR] Failed to write memory!\n";
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Create a remote thread to call Cbuf_AddText with our command
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)cbufAddTextAddr, remoteMemory, 0, NULL);
    if (!hThread) {
        std::cerr << "[ERROR] Failed to create remote thread!\n";
        VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return;
    }

    // Wait for the thread to complete and clean up
    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    std::cout << "[SUCCESS] Command executed: " << command << "\n";
}