#pragma once
#include <cstdint>
#include includes.h

#ifdef max
#undef max
#endif



extern uintptr_t dwProcessBase;

const static auto Cbuf_AddText = reinterpret_cast<std::uintptr_t(__fastcall*)(int, const char*)>(dwProcessBase + 0x3CDE880);