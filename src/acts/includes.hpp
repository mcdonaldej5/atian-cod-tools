#pragma once

#define _WINSOCKAPI_
#include <includes_shared.hpp>
#include <zlib.h>

#pragma warning(push)
#pragma warning(disable:4455)
#include <PS4DBG.hpp>
#pragma warning(pop)

#include "hashutils.hpp"
#include "mods/mods.hpp"
#include "tools/tools.hpp"


extern uintptr_t dwProcessBase;
extern uintptr_t cgArray;
extern uintptr_t cg_entitiesArray;

const static auto Cbuf_AddText = reinterpret_cast<std::uintptr_t(__fastcall*)(int, const char*)>(dwProcessBase + 0x3CDE880);
const static auto Cmd_ExecuteSingleCommand = reinterpret_cast<std::uintptr_t(__fastcall*)(int, int, const char* text, bool)>(dwProcessBase + 0x3CF4B80);
