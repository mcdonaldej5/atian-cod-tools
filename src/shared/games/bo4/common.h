#pragma once
#include <cstdint>
#include <stdint.h>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include "includes.h"

typedef INT8   int8_t;
typedef INT16  int16_t;
typedef INT32  int32_t;
typedef INT64  int64_t;
typedef UINT8  uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;

typedef unsigned __int64 __uint64_t;

typedef __uint64_t bdUInt64;

uintptr_t GetModuleBaseAddress(const wchar_t* moduleName) {
    DWORD processID = 0;

    // Get the process ID of BlackOps4.exe
    HWND hwnd = FindWindowW(NULL, L"Call of Duty®: Black Ops 4");  // Window title of BO4
    if (hwnd == NULL) {
        std::cerr << "Error: Could not find game window!" << std::endl;
        return 0;
    }
    GetWindowThreadProcessId(hwnd, &processID);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);
    if (hSnap == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32 modEntry;
    modEntry.dwSize = sizeof(modEntry);

    if (Module32First(hSnap, &modEntry)) {
        do {
            if (!_wcsicmp(modEntry.szModule, moduleName)) {
                CloseHandle(hSnap);
                return (uintptr_t)modEntry.modBaseAddr;
            }
        } while (Module32Next(hSnap, &modEntry));
    }

    CloseHandle(hSnap);
    return 0;
}

extern uintptr_t dwProcessBase;

const static auto Cbuf_AddText = reinterpret_cast<std::uintptr_t(__fastcall*)(int, const char*)>(dwProcessBase + 0x3CDE880);