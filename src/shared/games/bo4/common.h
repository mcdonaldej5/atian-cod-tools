#pragma once
#include <cstdint>
#include "includes.h"



extern uintptr_t dwProcessBase;

const static auto Cbuf_AddText = reinterpret_cast<std::uintptr_t(__fastcall*)(const char*)>(dwProcessBase + 0x3CDE880);