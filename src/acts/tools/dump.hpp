#pragma once
#include <includes.hpp>
#include <games/bo4/scriptinstance.hpp>

namespace tool::dump {
    using namespace games::bo4;
    struct T8ScriptParseTreeEntry {
        uint64_t name;
        uint64_t pad0;
        uintptr_t buffer;
        uint32_t size;
        uint32_t pad02;
    };

    struct T8GSCSimpleHeader {
        byte magic[8];
        int32_t crc;
        int32_t pad;
        uint64_t name;
    };

    struct T8ObjFileInfo {
        uintptr_t activeVersion;
        int32_t slot;
        int32_t refCount;
        uint32_t groupId;
        int32_t Pad0;
    };

    struct T8EventMapObj {
        uintptr_t next;
        uintptr_t handler;
        uint32_t name;
        int8_t gameSideRegistered;
        int8_t Pad01;
        int16_t Pad02;
    };
    struct T8BuiltInFunc
    {
        uint32_t name;
        uint32_t minArgs;
        uint32_t maxArgs;
        uint32_t pad01;
        uintptr_t function;
        uint32_t type;
        uint32_t pad02;
    };

    struct FunctionPoolDef {
        scriptinstance::ScriptInstance instance;
        bool methodPool;
        uint64_t offset;
        uint32_t size;
    };

    /*
    Can be extract from these functions, you can also search for the hash32("spawn") in the executable,
    it'll find the location of a pool search the x ref to it. (spawn is available in both CSC/GSC)
    Scr_GetFunction sub_33AF840
    Scr_GetMethod sub_33AFC20
    CScr_GetFunction sub_1F13140
    CScr_GetMethod sub_1F13650
    */
    static FunctionPoolDef functionPool[] {
        { scriptinstance::SI_SERVER, false, 0x49b60c0, 8 },
        { scriptinstance::SI_SERVER, false, 0x4f437c0, 370 },
        { scriptinstance::SI_SERVER, false, 0x49b9ae0, 372 },
        { scriptinstance::SI_SERVER, false, 0x49600d0, 60 },
        { scriptinstance::SI_SERVER, false, 0x495f250, 114 },
        { scriptinstance::SI_SERVER, false, 0x49608a0, 75 },
        { scriptinstance::SI_SERVER, false, 0x49612d0, 14 },
        { scriptinstance::SI_SERVER, false, 0x49b5590, 9 },
        { scriptinstance::SI_SERVER, false, 0x49b56c0, 15 },
        { scriptinstance::SI_SERVER, false, 0x495cfb0, 16 },
        { scriptinstance::SI_SERVER, false, 0x495ceb0, 6 },
        { scriptinstance::SI_SERVER, false, 0x495cf78, 1 },
        { scriptinstance::SI_SERVER, false, 0x495d1c8, 1 },
        { scriptinstance::SI_SERVER, true, 0x498dcf0, 491 },
        { scriptinstance::SI_SERVER, true, 0x49b2980, 40 },
        { scriptinstance::SI_SERVER, true, 0x49b5b20, 45 },
        { scriptinstance::SI_SERVER, true, 0x49bcff0, 19 },
        { scriptinstance::SI_SERVER, true, 0x49bd540, 134 },
        { scriptinstance::SI_SERVER, true, 0x49b3180, 5 },
        { scriptinstance::SI_SERVER, true, 0x49f1ff0, 123 },
        { scriptinstance::SI_SERVER, true, 0x495cd40, 10 },
        { scriptinstance::SI_SERVER, true, 0x49b3490, 260 },
        { scriptinstance::SI_SERVER, true, 0x49b58a0, 1 },
        { scriptinstance::SI_SERVER, true, 0x4f466b0, 372 },

        { scriptinstance::SI_CLIENT, false, 0x4ed3470, 351 },
        { scriptinstance::SI_CLIENT, false, 0x49600d0, 60 },
        { scriptinstance::SI_CLIENT, false, 0x495f250, 114 },
        { scriptinstance::SI_CLIENT, false, 0x49608a0, 75 },
        { scriptinstance::SI_CLIENT, false, 0x49612d0, 14 },
        { scriptinstance::SI_CLIENT, false, 0x4ed01a0, 135 },
        { scriptinstance::SI_CLIENT, false, 0x4969d70, 34 },
        { scriptinstance::SI_CLIENT, false, 0x4969bf0, 12 },
        { scriptinstance::SI_CLIENT, false, 0x496a370, 25 },
        { scriptinstance::SI_CLIENT, false, 0x496a720, 26 },
        { scriptinstance::SI_CLIENT, false, 0x4967da0, 31 },
        { scriptinstance::SI_CLIENT, true, 0x496e7d0, 325 },
        { scriptinstance::SI_CLIENT, true, 0x4969370, 56 },
        { scriptinstance::SI_CLIENT, true, 0x496a1b0, 9 },
        { scriptinstance::SI_CLIENT, true, 0x496aa60, 3 },
        { scriptinstance::SI_CLIENT, true, 0x4968180, 13 },
    };
}