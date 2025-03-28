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

static auto info_response_hostname = ""s;
static auto info_response_hostxuid = ""s;

extern uintptr_t dwProcessBase;
extern uintptr_t cgArray;
extern uintptr_t cg_entitiesArray;

extern float boldness;
extern float flHue;
extern float rgb_speed;

const static auto Cbuf_AddText = reinterpret_cast<std::uintptr_t(__fastcall*)(int, const char*)>(qwProcessBase + 0x3CDE880);
const static auto Cmd_ExecuteSingleCommand = reinterpret_cast<std::uintptr_t(__fastcall*)(int, int, const char* text, bool)>(qwProcessBase + 0x3CF4B80);
