#include <includes.hpp>
#ifdef _WIN32
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/bo3/pools.hpp>

namespace {

    // Path of the schedule StringTable asset as it lives in the BO3 asset pool.
    constexpr const char* SCHEDULE_ASSET_NAME = "gamedata/events/schedule.csv";

    // Timestamps: start safely in the past, end far in the future so injected
    // events are always considered active by the game's scheduler.
    constexpr int64_t EVENT_START = 1000000000; // Sept 2001
    constexpr int64_t EVENT_END   = 2147367600; // Jan 2038

    // BO3 schedule.csv has 6 columns:
    //   event_name | start_time | end_time | platform | extra1 | extra2
    constexpr int32_t COLUMN_COUNT = 6;

    // -----------------------------------------------------------------------
    // BO3 in-memory structs (from dumpt7.cpp analysis)
    // -----------------------------------------------------------------------

    struct T7StringTableCell {
        uintptr_t string; // const char* in the target process
        int32_t   hash;
        int32_t   _pad;   // natural alignment padding
    };
    static_assert(sizeof(T7StringTableCell) == 0x10, "T7StringTableCell size mismatch");

    struct T7StringTable {
        uintptr_t name;        // const char*
        int32_t   columnCount;
        int32_t   rowCount;
        uintptr_t values;      // T7StringTableCell*
        uintptr_t cellIndex;   // __int16*
    };
    static_assert(sizeof(T7StringTable) == 0x20, "T7StringTable size mismatch");

    // -----------------------------------------------------------------------
    // Locate g_XAssetPools at runtime via a pattern scan.
    //
    // The pattern targets a call site of DB_LinkXAssetEntry that is followed
    // immediately by a LEA loading the pool array base address:
    //
    //   E8 ?? ?? ?? ?? 48 8D 2D ?? ?? ?? ?? 48 89 06
    //   CALL DB_LinkXAssetEntry
    //   LEA  rbp, [rip + g_XAssetPools]   <- 7-byte instruction starting at +5
    //   MOV  [rsi], rbp
    //
    // The 4-byte RIP-relative displacement sits at match+8.
    // The instruction after the LEA (i.e., the RIP reference point) is at match+12.
    // Absolute pool base = (match + 12) + *(int32_t*)(match + 8)
    // -----------------------------------------------------------------------
    uintptr_t FindXAssetPoolsBase(Process& proc) {
        // Pattern: CALL ?? | LEA rbp,[rip+??] | MOV [rsi],rbp
        constexpr const char* PATTERN =
            "E8 ?? ?? ?? ?? 48 8D 2D ?? ?? ?? ?? 48 89 06";

        uintptr_t match = proc.Scan(PATTERN);
        if (!match) {
            return 0;
        }

        // Read the signed 32-bit RIP-relative displacement embedded in the LEA.
        int32_t disp{};
        if (!proc.ReadMemory(&disp, match + 8, sizeof(disp))) {
            return 0;
        }

        // RIP at the end of the LEA instruction = match + 5 (CALL) + 7 (LEA) = match + 12.
        return static_cast<uintptr_t>(static_cast<int64_t>(match + 12) + disp);
    }

    // -----------------------------------------------------------------------
    // djb2 hash - fills the hash field in each StringTableCell.
    // The game uses this for lookup; we populate it to stay consistent.
    // -----------------------------------------------------------------------
    constexpr int32_t Djb2Hash(const char* str) {
        uint32_t hash = 0x1505;
        while (*str) {
            hash = ((hash << 5) + hash) + static_cast<uint8_t>(*str++);
        }
        return static_cast<int32_t>(hash);
    }

    // -----------------------------------------------------------------------
    // Tool state
    // -----------------------------------------------------------------------
    static bool s_cherryCamoUI      = false;
    static bool s_cherryCamoInv     = false;
    static bool s_m14WeaponUI       = false;
    static bool s_double2XP         = false;
    static bool s_doubleWeaponXP    = false;
    static bool s_doubleLiquidDiv   = false;

    static uintptr_t s_prevAllocAddr = 0;
    static size_t    s_prevAllocSize = 0;
    static std::string s_notif{};

    // -----------------------------------------------------------------------
    // Tool UI - called every frame by the NUI render loop
    // -----------------------------------------------------------------------
    void bo3_event_tool() {
        tool::nui::NuiUseDefaultWindow dw{};
        ImGui::SeparatorText("BO3 Events Tool");
        ImGui::TextDisabled("Injects events into the running BlackOps3.exe process.");
        ImGui::Spacing();

        static std::once_flag s_init{};
        std::call_once(s_init, [] {
            s_cherryCamoUI   = core::config::GetBool("ui.bo3event.cherryCamoUI",   false);
            s_cherryCamoInv  = core::config::GetBool("ui.bo3event.cherryCamoInv",  false);
            s_m14WeaponUI    = core::config::GetBool("ui.bo3event.m14WeaponUI",     false);
            s_double2XP      = core::config::GetBool("ui.bo3event.double2XP",       false);
            s_doubleWeaponXP = core::config::GetBool("ui.bo3event.doubleWeaponXP",  false);
            s_doubleLiquidDiv= core::config::GetBool("ui.bo3event.doubleLiqDiv",    false);
            tool::nui::SaveNextConfig();
        });

        bool changed = false;

        ImGui::SeparatorText("Limited Time Items");
        ImGui::TextDisabled("Cherry Fizz summer camo (item version 103)");
        if (ImGui::Checkbox("Cherry Fizz - Promo UI", &s_cherryCamoUI)) {
            core::config::SetBool("ui.bo3event.cherryCamoUI", s_cherryCamoUI);
            changed = true;
        }
        if (ImGui::Checkbox("Cherry Fizz - Inventory Version", &s_cherryCamoInv)) {
            core::config::SetBool("ui.bo3event.cherryCamoInv", s_cherryCamoInv);
            changed = true;
        }
        ImGui::Spacing();
        ImGui::TextDisabled("M14 weapon (item version 107)");
        if (ImGui::Checkbox("M14 Weapon - Promo UI + Inventory", &s_m14WeaponUI)) {
            core::config::SetBool("ui.bo3event.m14WeaponUI", s_m14WeaponUI);
            changed = true;
        }

        ImGui::SeparatorText("XP / Currency Events");
        if (ImGui::Checkbox("Double XP (MP + ZM)", &s_double2XP)) {
            core::config::SetBool("ui.bo3event.double2XP", s_double2XP);
            changed = true;
        }
        if (ImGui::Checkbox("Double Weapon XP (MP + ZM)", &s_doubleWeaponXP)) {
            core::config::SetBool("ui.bo3event.doubleWeaponXP", s_doubleWeaponXP);
            changed = true;
        }
        if (ImGui::Checkbox("Double Liquid Divinium", &s_doubleLiquidDiv)) {
            core::config::SetBool("ui.bo3event.doubleLiqDiv", s_doubleLiquidDiv);
            changed = true;
        }

        if (changed) {
            tool::nui::SaveNextConfig();
        }

        ImGui::Separator();

        if (ImGui::Button("Inject into BlackOps3.exe")) {

            std::vector<std::string> events{};

            // Cherry Fizz: two separate schedule event names required.
            // "limited_time_summer_camo_ui_ON" activates the Black Market promo banner.
            // "limited_time_summer_camo_inventory_version_ON" sets item version 103.
            if (s_cherryCamoUI) {
                events.push_back("limited_time_summer_camo_ui_ON");
            }
            if (s_cherryCamoInv) {
                events.push_back("limited_time_summer_camo_inventory_version_ON");
            }

            // M14: single event handles both promo UI and item version 107.
            if (s_m14WeaponUI) {
                events.push_back("limited_time_m14_weapon_ui_ON");
            }

            // XP / currency bonus events (mapped via eventmapping.csv).
            if (s_double2XP) {
                events.push_back("double_xp_mp_and_zm");
            }
            if (s_doubleWeaponXP) {
                events.push_back("double_weapon_xp");
            }
            if (s_doubleLiquidDiv) {
                events.push_back("double_liquid_divinium");
            }

            if (events.empty()) {
                s_notif = "No events selected.";
            }
            else {
                LOG_TRACE("BO3 event inject: {} events", events.size());
                for (const auto& ev : events) {
                    LOG_TRACE("  -> {}", ev);
                }

                Process proc = L"BlackOps3.exe";
                uintptr_t allocated{};
                size_t    allocatedSize{};

                try {
                    if (!proc || !proc.Open()) {
                        throw std::runtime_error("Can't open BlackOps3.exe - is the game running?");
                    }

                    // ------------------------------------------------------------------
                    // Build the raw memory block we will inject.
                    //
                    // Layout inside rfile[]:
                    //   [offset 0]         - COLUMN_COUNT * events.size() T7StringTableCell structs
                    //   [after cells]      - null-terminated strings for every cell value
                    // ------------------------------------------------------------------
                    std::vector<byte> rfile{};

                    // Append a string and return its offset from rfile[0].
                    auto writeStr = [&](const std::string& s) -> size_t {
                        size_t off = rfile.size();
                        rfile.insert(rfile.end(), s.begin(), s.end());
                        rfile.push_back('\0');
                        return off;
                    };

                    // Pad rfile to 8-byte alignment.
                    auto align8 = [&]() {
                        while (rfile.size() % 8) rfile.push_back(0);
                    };

                    // Reserve space for the cell array at offset 0.
                    size_t numRows  = events.size();
                    size_t numCells = numRows * COLUMN_COUNT;
                    size_t cellsOff = rfile.size(); // == 0
                    rfile.resize(cellsOff + sizeof(T7StringTableCell) * numCells, 0);

                    // Write the shared column strings once, reuse their offsets.
                    // Columns 1-5 are the same for every row.
                    std::string startStr = std::to_string(EVENT_START);
                    std::string endStr   = std::to_string(EVENT_END);

                    align8();
                    size_t startOff = writeStr(startStr);
                    size_t endOff   = writeStr(endStr);
                    size_t allOff   = writeStr("all");
                    size_t emptyOff = writeStr("");

                    // Write per-event name strings and populate each row of cells.
                    for (size_t i = 0; i < numRows; i++) {
                        align8();
                        size_t nameOff = writeStr(events[i]);

                        T7StringTableCell* row =
                            reinterpret_cast<T7StringTableCell*>(&rfile[cellsOff]) + i * COLUMN_COUNT;

                        row[0].string = nameOff;
                        row[0].hash   = Djb2Hash(events[i].c_str());

                        row[1].string = startOff;
                        row[1].hash   = Djb2Hash(startStr.c_str());

                        row[2].string = endOff;
                        row[2].hash   = Djb2Hash(endStr.c_str());

                        row[3].string = allOff;
                        row[3].hash   = Djb2Hash("all");

                        row[4].string = emptyOff;
                        row[4].hash   = 0;

                        row[5].string = emptyOff;
                        row[5].hash   = 0;
                    }

                    // Allocate memory in the remote process.
                    allocatedSize = rfile.size();
                    allocated     = proc.AllocateMemory(allocatedSize);
                    if (!allocated) {
                        throw std::runtime_error(
                            std::format("Can't allocate {} bytes in remote process", allocatedSize));
                    }

                    // Relocate all string offsets: offset-from-rfile -> absolute remote address.
                    for (size_t i = 0; i < numRows; i++) {
                        T7StringTableCell* row =
                            reinterpret_cast<T7StringTableCell*>(&rfile[cellsOff]) + i * COLUMN_COUNT;
                        for (int c = 0; c < COLUMN_COUNT; c++) {
                            row[c].string += allocated;
                        }
                    }

                    // Write the entire block into the remote process.
                    if (!proc.WriteMemory(allocated, rfile.data(), allocatedSize)) {
                        throw std::runtime_error(
                            std::format("Can't write {} bytes to remote process", allocatedSize));
                    }

                    // ------------------------------------------------------------------
                    // Locate g_XAssetPools via pattern scan, then find the
                    // StringTable pool descriptor (index 48).
                    // ------------------------------------------------------------------
                    uintptr_t poolsBase = FindXAssetPoolsBase(proc);
                    if (!poolsBase) {
                        throw std::runtime_error(
                            "Could not locate g_XAssetPools via pattern scan.\n"
                            "Ensure the game is fully loaded at the main menu.");
                    }

                    constexpr size_t STRINGTABLE_INDEX = bo3::pool::T7XAssetType::T7_ASSET_TYPE_STRINGTABLE;
                    uintptr_t poolDescAddr = poolsBase + STRINGTABLE_INDEX * sizeof(bo3::pool::T7XAssetPool);

                    auto poolDesc = proc.ReadMemoryObjectEx<bo3::pool::T7XAssetPool>(poolDescAddr);

                    if (!poolDesc->pool || poolDesc->itemCount <= 0) {
                        throw std::runtime_error(
                            "StringTable pool is empty or not yet loaded.\n"
                            "Make sure you are at the main menu before injecting.");
                    }

                    // Read all pool entries. Use itemCount to match the dumper behaviour.
                    auto entries = proc.ReadMemoryArrayEx<T7StringTable>(
                        poolDesc->pool, poolDesc->itemCount);

                    // Search for the schedule.csv entry by name string comparison.
                    bool patched = false;
                    for (int32_t i = 0; i < poolDesc->itemCount; i++) {
                        const T7StringTable& e = entries[i];
                        if (!e.name) continue;

                        auto entryName = proc.ReadStringTmp(e.name, nullptr);
                        if (!entryName || std::string_view(entryName) != SCHEDULE_ASSET_NAME) continue;

                        // Replace the entry: keep name and cellIndex from the original
                        // (they remain valid in game memory), update the cell data only.
                        T7StringTable replacement{};
                        replacement.name        = e.name;
                        replacement.columnCount = COLUMN_COUNT;
                        replacement.rowCount    = static_cast<int32_t>(numRows);
                        replacement.values      = allocated + cellsOff;
                        replacement.cellIndex   = e.cellIndex;

                        uintptr_t entryAddr = poolDesc->pool + sizeof(T7StringTable) * i;
                        if (!proc.WriteMemory(entryAddr, &replacement, sizeof(replacement))) {
                            throw std::runtime_error("Can't write patched StringTable entry");
                        }

                        patched = true;
                        break;
                    }

                    if (!patched) {
                        throw std::runtime_error(
                            "Could not find 'gamedata/events/schedule.csv' in the StringTable pool.\n"
                            "Make sure you are at the main menu before injecting.");
                    }

                    // Free the previous remote allocation if one exists.
                    // If the game was restarted, FreeMemory fails silently - that is fine.
                    if (s_prevAllocAddr) {
                        proc.FreeMemory(s_prevAllocAddr, s_prevAllocSize);
                    }
                    s_prevAllocAddr = allocated;
                    s_prevAllocSize = allocatedSize;

                    s_notif = std::format("Injected {} event(s) successfully.", numRows);
                }
                catch (const std::runtime_error& err) {
                    if (allocated) {
                        proc.FreeMemory(allocated, allocatedSize);
                    }
                    s_notif = err.what();
                    LOG_ERROR("BO3 event tool error: {}", err.what());
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Restore Original")) {
            Process proc = L"BlackOps3.exe";
            try {
                if (!proc || !proc.Open()) {
                    throw std::runtime_error("Can't open BlackOps3.exe");
                }

                uintptr_t poolsBase = FindXAssetPoolsBase(proc);
                if (!poolsBase) {
                    throw std::runtime_error("Could not locate g_XAssetPools via pattern scan.");
                }

                constexpr size_t STRINGTABLE_INDEX = bo3::pool::T7XAssetType::T7_ASSET_TYPE_STRINGTABLE;
                uintptr_t poolDescAddr = poolsBase + STRINGTABLE_INDEX * sizeof(bo3::pool::T7XAssetPool);

                auto poolDesc = proc.ReadMemoryObjectEx<bo3::pool::T7XAssetPool>(poolDescAddr);

                if (!poolDesc->pool || poolDesc->itemCount <= 0) {
                    throw std::runtime_error("StringTable pool not loaded; nothing to restore.");
                }

                auto entries = proc.ReadMemoryArrayEx<T7StringTable>(
                    poolDesc->pool, poolDesc->itemCount);

                for (int32_t i = 0; i < poolDesc->itemCount; i++) {
                    if (!entries[i].name) continue;
                    auto n = proc.ReadStringTmp(entries[i].name, nullptr);
                    if (!n || std::string_view(n) != SCHEDULE_ASSET_NAME) continue;

                    // Zero rowCount so the game treats the schedule as empty.
                    uintptr_t rowCountAddr = poolDesc->pool + sizeof(T7StringTable) * i
                        + offsetof(T7StringTable, rowCount);
                    int32_t zero = 0;
                    proc.WriteMemory(rowCountAddr, &zero, sizeof(zero));
                    s_notif = "Schedule cleared (restart map to fully restore).";
                    break;
                }
            }
            catch (const std::runtime_error& err) {
                s_notif = err.what();
            }
        }

        if (!s_notif.empty()) {
            ImGui::Separator();
            ImGui::TextWrapped("%s", s_notif.c_str());
        }
    }

    ADD_TOOL_NUI(bo3_event_tool, "BO3 Events Tool", bo3_event_tool);

} // namespace

#endif // _WIN32
