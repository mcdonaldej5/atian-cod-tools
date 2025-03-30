#pragma once

using namespace std;
using namespace std::literals;
using namespace std::string_literals;
using namespace std::literals::string_literals;


typedef void* (__fastcall* Cbuf_AddText_)(int, const char*);
Cbuf_AddText_ Cbuf_AddText = (game_base + 0x3CDE880);