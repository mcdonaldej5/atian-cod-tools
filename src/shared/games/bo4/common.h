#pragma once



using namespace std;
using namespace std::literals;
using namespace std::string_literals;
using namespace std::literals::string_literals;

extern uintptr_t dwProcessBase;

const static auto Cbuf_AddText = reinterpret_cast<std::uintptr_t(__fastcall*)(int, const char*)>(dwProcessBase + 0x3CDE880);
