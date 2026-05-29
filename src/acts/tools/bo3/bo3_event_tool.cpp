#include <includes.hpp>
#ifdef _WIN32
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/bo3/pools.hpp>

namespace {

    // Path of the schedule StringTable asset as it exists in the BO3 asset pool.
    // BO3 stores the name as a plain const char* (no XHash), so we compare by string.
    constexpr const char* SCHEDULE_ASSET_NAME = "gamedata/events/schedule.csv";

    // Timestamps: start in the past, end far in the future so injected events
    // are always considered active by the game's scheduler.
    constexpr int64_t EVENT_START = 1000000000; // Sept 2001 - safely in the past
    constexpr int64_t EVENT_END   = 2147367600; // Jan 2038 - max practical unix time

    // BO3 schedule.csv has 6 columns:
    //   event_name | start_time | end_time | platform | extra1 | extra2
    constexpr int32_t COLUMN_COUNT = 6;

    // -----------------------------------------------------------------------
    // BO3 in-memory StringTable structs (from dumpt7.cpp analysis)
    // -----------------------------------------------------------------------

    // Each cell is a pointer to a string + a precomputed hash of that string.
    struct T7StringTableCell {
        uintptr_t string; // const char* in the target process
        int32_t   hash;
        int32_t   _pad;   // alignment padding to reach 0x10 bytes
    };
    static_assert(sizeof(T7StringTableCell) == 0x10, "T7StringTableCell size mismatch");

    // The StringTable asset pool entry itself.
    struct T7StringTable {
        uintptr_t name;        // const char*  - asset name/path
        int32_t   columnCount;
        int32_t   rowCount;
        uintptr_t values;      // T7StringTableCell*
        uintptr_t cellIndex;   // __int16* - lookup index, not needed for our patch
    };
    static_assert(sizeof(T7StringTable) == 0x20, "T7StringTable size mismatch");

    // -----------------------------------------------------------------------
    // Simple djb2 hash matching how BO3 hashes StringTable cell strings.
    // Used to fill the 'hash' field so the game's lookup code is happy.
    // -----------------------------------------------------------------------
    constexpr int32_t Djb2Hash(const char* str) {
        uint32_t hash = 0x1505;
        while (*str) {
            hash = ((hash << 5) + hash) + (uint8_t)*str++;
        }
        return (int32_t)hash;
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
    static DWORD     s_prevPID       = 0;
    static std::string s_notif{};

    // -----------------------------------------------------------------------
    // Tool UI function - called every frame by the NUI render loop
    // -----------------------------------------------------------------------
    void bo3_event_tool() {
        tool::nui::NuiUseDefaultWindow dw{};
        ImGui::SeparatorText("BO3 Event Tool");
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

            // Build the list of event name strings to inject as schedule rows.
            // Each string maps to column 0 of a schedule row.
            std::vector<std::string> events{};

            // Cherry Fizz camo - requires two separate schedule event names:
            //   "limited_time_summer_camo_ui_ON"        -> activates the promo banner in the Black Market UI
            //   "limited_time_summer_camo_inventory_version_ON" -> sets loot_limitedtimeitemversions to ,103
            if (s_cherryCamoUI) {
                events.push_back("limited_time_summer_camo_ui_ON");
            }
            if (s_cherryCamoInv) {
                events.push_back("limited_time_summer_camo_inventory_version_ON");
            }

            // M14 weapon - single event handles both the promo UI and item version 107
            if (s_m14WeaponUI) {
                events.push_back("limited_time_m14_weapon_ui_ON");
            }

            // XP / currency bonus events (mapped via eventmapping.csv)
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

                Process bo3 = L"BlackOps3.exe";
                uintptr_t allocated{};
                size_t    allocatedSize{};

                try {
                    if (!bo3 || !bo3.Open()) {
                        throw std::runtime_error("Can't open BlackOps3.exe - is the game running?");
                    }

                    // ------------------------------------------------------------------
                    // Build the raw memory block we will inject.
                    //
                    // Layout inside rfile[]:
                    //   [cells]    - COLUMN_COUNT * events.size() T7StringTableCell structs
                    //   [strings]  - null-terminated strings for every cell value
                    //
                    // After writing to the remote process we fix up the string pointers
                    // so they point to their actual remote addresses.
                    // ------------------------------------------------------------------
                    std::vector<byte> rfile{};

                    // String helpers: write a string into rfile and return its offset.
                    auto writeStr = [&](const std::string& s) -> size_t {
                        size_t off = rfile.size();
                        rfile.insert(rfile.end(), s.begin(), s.end());
                        rfile.push_back('\0');
                        return off;
                    };

                    // Align rfile to 8 bytes.
                    auto align8 = [&]() {
                        while (rfile.size() % 8) rfile.push_back(0);
                    };

                    // Reserve space for the cell array first.
                    size_t numRows  = events.size();
                    size_t numCells = numRows * COLUMN_COUNT;
                    size_t cellsOff = rfile.size();
                    rfile.resize(cellsOff + sizeof(T7StringTableCell) * numCells, 0);

                    // Pre-build the string values for the six columns of each row.
                    //   col 0: event name
                    //   col 1: start timestamp (as decimal string)
                    //   col 2: end timestamp (as decimal string)
                    //   col 3: platform tag ("all")
                    //   col 4: empty string
                    //   col 5: empty string
                    std::string startStr = std::to_string(EVENT_START);
                    std::string endStr   = std::to_string(EVENT_END);
                    std::string allStr   = "all";
                    std::string emptyStr = "";

                    // Write fixed strings once, reuse their offsets.
                    align8();
                    size_t startOff = writeStr(startStr);
                    size_t endOff   = writeStr(endStr);
                    size_t allOff   = writeStr(allStr);
                    size_t emptyOff = writeStr(emptyStr);

                    // Write per-event name strings and populate cell array.
                    for (size_t i = 0; i < numRows; i++) {
                        align8();
                        size_t nameOff = writeStr(events[i]);

                        T7StringTableCell* row = reinterpret_cast<T7StringTableCell*>(&rfile[cellsOff]) + i * COLUMN_COUNT;

                        // col 0: event name
                        row[0].string = nameOff;   // will be relocated below
                        row[0].hash   = Djb2Hash(events[i].c_str());

                        // col 1: start time
                        row[1].string = startOff;
                        row[1].hash   = Djb2Hash(startStr.c_str());

                        // col 2: end time
                        row[2].string = endOff;
                        row[2].hash   = Djb2Hash(endStr.c_str());

                        // col 3: platform
                        row[3].string = allOff;
                        row[3].hash   = Djb2Hash("all");

                        // col 4: empty
                        row[4].string = emptyOff;
                        row[4].hash   = 0;

                        // col 5: empty
                        row[5].string = emptyOff;
                        row[5].hash   = 0;
                    }

                    // Allocate memory in the remote process.
                    allocatedSize = rfile.size();
                    allocated = bo3.AllocateMemory(allocatedSize);
                    if (!allocated) {
                        throw std::runtime_error(std::format("Can't allocate {} bytes in remote process", allocatedSize));
                    }

                    // Relocate all string offsets to real remote addresses.
                    for (size_t i = 0; i < numRows; i++) {
                        T7StringTableCell* row = reinterpret_cast<T7StringTableCell*>(&rfile[cellsOff]) + i * COLUMN_COUNT;
                        for (int c = 0; c < COLUMN_COUNT; c++) {
                            row[c].string += allocated; // offset -> absolute remote address
                        }
                    }

                    // Write the whole block into the remote process.
                    if (!bo3.WriteMemory(allocated, rfile.data(), allocatedSize)) {
                        throw std::runtime_error(std::format("Can't write {} bytes to remote process", allocatedSize));
                    }

                    // ------------------------------------------------------------------
                    // Locate the StringTable asset pool and find the schedule entry.
                    // ------------------------------------------------------------------

                    // Read the pool descriptor for T7_ASSET_TYPE_STRINGTABLE (index 48).
                    constexpr size_t STRINGTABLE_INDEX = bo3::pool::T7XAssetType::T7_ASSET_TYPE_STRINGTABLE;
                    uintptr_t poolDescAddr = bo3::pool::xassetpools + STRINGTABLE_INDEX * sizeof(bo3::pool::T7XAssetPool);

                    auto poolDesc = bo3.ReadMemoryObjectEx<bo3::pool::T7XAssetPool>(poolDescAddr);
                    if (!poolDesc) {
                        throw std::runtime_error("Can't read StringTable pool descriptor");
                    }

                    if (poolDesc->itemSize != sizeof(T7StringTable)) {
                        throw std::runtime_error(std::format(
                            "StringTable pool itemSize mismatch: got {}, expected {}",
                            poolDesc->itemSize, sizeof(T7StringTable)));
                    }

                    // Read all pool entries.
                    auto entries = bo3.ReadMemoryArrayEx<T7StringTable>(poolDesc->pool, poolDesc->itemAllocCount);
                    if (!entries) {
                        throw std::runtime_error("Can't read StringTable pool entries");
                    }

                    // Search for the schedule.csv entry by name string comparison.
                    bool patched = false;
                    for (int32_t i = 0; i < poolDesc->itemAllocCount; i++) {
                        const T7StringTable& e = entries[i];
                        if (!e.name) continue;

                        auto entryName = bo3.ReadStringTmp(e.name, nullptr);
                        if (!entryName || std::string_view(entryName) != SCHEDULE_ASSET_NAME) continue;

                        // Build the replacement entry: keep name and cellIndex pointers
                        // as-is (they remain valid in the game's memory), only update
                        // the cell count and values pointer.
                        T7StringTable replacement{};
                        replacement.name        = e.name;                          // original name ptr
                        replacement.columnCount = COLUMN_COUNT;
                        replacement.rowCount    = (int32_t)numRows;
                        replacement.values      = allocated + cellsOff;            // our injected cells
                        replacement.cellIndex   = e.cellIndex;                     // leave original index

                        uintptr_t entryAddr = poolDesc->pool + sizeof(T7StringTable) * i;
                        if (!bo3.WriteMemory(entryAddr, &replacement, sizeof(replacement))) {
                            throw std::runtime_error("Can't write patched StringTable entry");
                        }

                        patched = true;
                        break;
                    }

                    if (!patched) {
                        throw std::runtime_error(
                            "Could not find 'gamedata/events/schedule.csv' in the StringTable pool.\n"
                            "Make sure you are in the main menu (not loading a map) before injecting.");
                    }

                    // Free the previous allocation for this process (avoids leaking remote memory).
                    if (s_prevPID == bo3.GetProcessId() && s_prevAllocAddr) {
                        bo3.FreeMemory(s_prevAllocAddr, s_prevAllocSize);
                    }
                    s_prevPID       = bo3.GetProcessId();
                    s_prevAllocAddr = allocated;
                    s_prevAllocSize = allocatedSize;

                    s_notif = std::format("Injected {} event(s) successfully.", numRows);
                }
                catch (const std::runtime_error& err) {
                    if (allocated) {
                        bo3.FreeMemory(allocated, allocatedSize);
                    }
                    s_notif = err.what();
                    LOG_ERROR("BO3 event tool error: {}", err.what());
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Restore Original")) {
            // Patch the schedule pointer back to null rowCount so the game re-reads
            // from its own loaded data on next reload. The simplest approach that
            // doesn't require us to store the original pointer is to write rowCount=0,
            // which causes the event system to treat the table as empty.
            Process bo3 = L"BlackOps3.exe";
            try {
                if (!bo3 || !bo3.Open()) {
                    throw std::runtime_error("Can't open BlackOps3.exe");
                }

                constexpr size_t STRINGTABLE_INDEX = bo3::pool::T7XAssetType::T7_ASSET_TYPE_STRINGTABLE;
                uintptr_t poolDescAddr = bo3::pool::xassetpools + STRINGTABLE_INDEX * sizeof(bo3::pool::T7XAssetPool);
                auto poolDesc = bo3.ReadMemoryObjectEx<bo3::pool::T7XAssetPool>(poolDescAddr);
                if (!poolDesc) throw std::runtime_error("Can't read pool descriptor");

                auto entries = bo3.ReadMemoryArrayEx<T7StringTable>(poolDesc->pool, poolDesc->itemAllocCount);
                if (!entries) throw std::runtime_error("Can't read pool entries");

                for (int32_t i = 0; i < poolDesc->itemAllocCount; i++) {
                    if (!entries[i].name) continue;
                    auto n = bo3.ReadStringTmp(entries[i].name, nullptr);
                    if (!n || std::string_view(n) != SCHEDULE_ASSET_NAME) continue;

                    // Zero the rowCount so the schedule appears empty to the game.
                    uintptr_t rowCountAddr = poolDesc->pool + sizeof(T7StringTable) * i
                        + offsetof(T7StringTable, rowCount);
                    int32_t zero = 0;
                    bo3.WriteMemory(rowCountAddr, &zero, sizeof(zero));
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
