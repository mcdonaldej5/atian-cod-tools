#include <includes.hpp>
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>

namespace {

    // =========================================================================
    // My Deals are controlled by gamedata/loot/loot_myshop.csv, a StringTable
    // in pool ASSET_TYPE_STRINGTABLE. The table has 6 columns:
    //
    //   col 0: item name   (STC_TYPE_STRING - char* pointer to name string)
    //   col 1: category    (STC_TYPE_STRING - char* pointer to category string)
    //   col 2: myshop_id   (STC_TYPE_INT    - slot-encoded unique ID: 1XXXXXX/2XXXXXX)
    //   col 3: cod_points  (STC_TYPE_INT    - COD Points price)
    //   col 4: reserves    (STC_TYPE_INT    - Reserve Cases cost, 0 = CP only)
    //   col 5: slot        (STC_TYPE_INT    - 1 or 2)
    //
    // The game randomly picks one row per slot every 12 hours (client timer).
    // Injecting a table with exactly one row per slot forces that specific item.
    //
    // NOTE: The # prefix on strings in the dump is just the dumper marking
    // values it couldn't fully resolve - the engine still stores them as plain
    // char* string pointers in a StringTable, the same as all other StringTables.
    // =========================================================================

    constexpr uint64_t loot_myshop_csv = hash::Hash64("gamedata/loot/loot_myshop.csv");

    // =========================================================================
    // Item descriptor. All data sourced directly from loot_myshop.csv.
    // category: the raw string written to the category column.
    //   - Known resolvable values: "loot_stream", "special_order"
    //   - For items whose category is an unresolved hash in the dump, we use
    //     "loot_stream" as a safe placeholder (the engine uses category for
    //     UI tab grouping only, not for item identity or price lookup).
    // myshopId1/2: the slot-encoded item IDs from col 2 of loot_myshop.csv.
    //   Leading digit encodes slot (1 = slot1, 2 = slot2).
    // =========================================================================
    struct MyShopItem {
        const char* display;
        const char* itemName;   // written as STC_TYPE_STRING col 0
        const char* category;   // written as STC_TYPE_STRING col 1
        int32_t     myshopId1;  // col 2 when slot == 1
        int32_t     myshopId2;  // col 2 when slot == 2
        int32_t     codPts1;    // col 3 when slot == 1
        int32_t     codPts2;    // col 3 when slot == 2
        int32_t     reserves1;  // col 4 when slot == 1
        int32_t     reserves2;  // col 4 when slot == 2
    };

    // =========================================================================
    // All named (resolvable) items from loot_myshop.csv.
    // Items whose col 0 is an unresolved #hash_ are omitted - we cannot target
    // them by name since we don't know their string.
    // Categories using the unresolved #hash_731315ef3fb1ae07 are written as
    // "loot_stream" (a safe known-valid string) since category is UI-only.
    // =========================================================================
    static const MyShopItem g_items[] = {
        // -- Camos --
        { "DLC1: Masks Active Camo",         "camo_active_dlc1_masks_wrapper",              "loot_stream",   1373223, 2373223,  500,  320,  1300000, 0      },
        { "DLC1: WW2 Active Camo",           "camo_active_dlc1_ww2_wrapper",                "loot_stream",   1868993, 2868993,  360,  320,  1303,    1303   },
        { "DLC5: TR Midburst Bomber Camo",   "camo_active_sig_tr_midburst_bomber_base",     "loot_stream",   1640460, 0,        1000, 0,    0,       0      },
        // -- Characters --
        { "Takeo (ZM Character)",            "loot_character_takeo_ofc",                    "loot_stream",   1284655, 2284655,  720,  640,  1851,    1851   },
        // -- Emotes --
        { "Emote: Stabby Banana",            "loot_emote_exotic_shared_stabby_banana",      "loot_stream",   1212907, 2212907,  360,  320,  1116,    1116   },
        { "Emote: Shush",                    "loot_emote_shared_shush",                     "loot_stream",   1471455, 2471455,  180,  160,  0,       0      },
        // -- Mastercrafts --
        { "Mastercraft: Vampire Hunter",     "mastercraft_vampire_hunter",                  "loot_stream",   1285318, 2285318,  900,  800,  10000,   0      },
        // -- Outfits --
        { "Outfit: Flaming Skull Warpaint",  "outfit_global_warpaint_bundle_flaming_skull", "loot_stream",   1255038, 2255038,  225,  200,  0,       0      },
        { "Outfit: Marauder (Battery)",      "outfit_marauder_rare4_battery",               "loot_stream",   1960738, 2960738,  180,  160,  0,       0      },
        { "Outfit: N.Lights Lgd (Firebreak)","outfit_northern_lights_legendary3_firebreak", "loot_stream",   1240245, 2240245,  540,  480,  2679,    2679   },
        { "Outfit: Zero Legendary 1",        "outfit_snake_legendary1_zero",                "loot_stream",   1127737, 2127737,  360,  320,  1117,    1117   },
        { "Outfit: Seraph Legendary 2",      "outfit_snake_legendary2_seraph",              "loot_stream",   1846821, 2846821,  360,  320,  1006,    1006   },
        { "Outfit: Battery Base Palette",    "outfit_warrior_base_pal_exo1_battery",        "loot_stream",   1849642, 2849642,  720,  640,  0,       0      },
        { "Outfit: Crash Epic 1",            "outfit_warrior_epic1_crash",                  "loot_stream",   1205283, 2205283,  720,  640,  2494,    2494   },
        { "Outfit: Prophet Epic 1",          "outfit_warrior_epic1_prophet",                "loot_stream",   1534910, 2534910,  720,  640,  1344,    1344   },
        { "Outfit: Zero Epic 2",             "outfit_warrior_epic2_zero",                   "loot_stream",   1012900, 2012900,  720,  640,  1289,    1289   },
        // -- Reserve Crate Bundles --
        { "3x Reserve Cases",               "abr_3_crates",                                "loot_stream",   1123898, 0,        1600, 0,    0,       0      },
        { "5x Reserve Cases",               "abr_5_crates",                                "loot_stream",   1186962, 0,        2000, 0,    0,       0      },
        { "8x Reserve Cases",               "abr_8_crates",                                "loot_stream",   1163052, 2163052,  2000, 2400, 50000,   0      },
        // -- Special Order Contracts (Muertos - 807 reserves) --
        { "Contract: Ajax Muertos",         "loot_contract_ajax_muertos",                  "special_order", 1943942, 2943942,  480,  500,  807,     807    },
        { "Contract: Battery Muertos",      "loot_contract_battery_muertos",               "special_order", 1519612, 2519612,  400,  500,  807,     807    },
        { "Contract: Crash Muertos",        "loot_contract_crash_muertos",                 "special_order", 1406463, 2406463,  490,  500,  807,     807    },
        { "Contract: Firebreak Muertos",    "loot_contract_firebreak_muertos",             "special_order", 1573245, 2573245,  400,  500,  807,     807    },
        { "Contract: Nomad Muertos",        "loot_contract_nomad_muertos",                 "special_order", 1518581, 2518581,  480,  500,  807,     807    },
        { "Contract: Prophet Muertos",      "loot_contract_prophet_muertos",               "special_order", 1220241, 2220241,  490,  500,  807,     807    },
        { "Contract: Recon Muertos",        "loot_contract_recon_muertos",                 "special_order", 1255337, 2255337,  480,  500,  807,     807    },
        { "Contract: Ruin Muertos",         "loot_contract_ruin_muertos",                  "special_order", 1573672, 2573672,  400,  500,  807,     807    },
        { "Contract: Seraph Muertos",       "loot_contract_seraph_muertos",                "special_order", 1617121, 2617121,  480,  500,  807,     807    },
        { "Contract: Torque Muertos",       "loot_contract_torque_muertos",                "special_order", 1446502, 2446502,  490,  500,  807,     807    },
        // -- Special Order Contracts (Prestige/Legendary - 2202+ reserves) --
        { "Contract: Recon Samurai",        "loot_contract_recon_samurai",                 "special_order", 1088436, 2088436,  580,  600,  2202,    2202   },
        { "Contract: Ruin Modern Viking",   "loot_contract_ruin_modern_viking",            "special_order", 1424760, 2424760,  580,  600,  2202,    2202   },
        { "Contract: Seraph Crimson Lotus", "loot_contract_seraph_crimson_lotus",          "special_order", 1257893, 2257893,  500,  600,  2202,    2202   },
        { "Contract: Torque Highlander",    "loot_contract_torque_highlander",             "special_order", 1366927, 2366927,  600,  800,  2202,    2202   },
        { "Contract: Crash Cop",            "loot_contract_crash_cop",                     "special_order", 1444031, 2444031,  700,  800,  2203,    2203   },
        { "Contract: Nomad Heist",          "loot_contract_nomad_heist",                   "special_order", 1365499, 2365499,  700,  800,  2203,    2203   },
        { "Contract: Seraph Cop",           "loot_contract_seraph_cop",                    "special_order", 1913949, 2913949,  700,  800,  2203,    2203   },
        { "Contract: Zero Cyberpunk",       "loot_contract_zero_cyberpunk",                "special_order", 1404466, 2404466,  500,  600,  2458,    2458   },
        { "Contract: Richtofen (ZM)",       "loot_contract_richtofen",                     "special_order", 1781759, 2781759,  500,  580,  2694,    2694   },
    };
    static constexpr size_t g_itemCount = ARRAYSIZE(g_items);

    static const char* k_noChange = "(Keep Random)";
    static const MyShopItem* g_deal1 = nullptr; // nullptr = no override
    static const MyShopItem* g_deal2 = nullptr;
    static std::string g_notif{};

    struct InjectionState {
        uintptr_t data{}; size_t dataSize{}; DWORD pid{};
    };
    static InjectionState g_state{};

    // =========================================================================
    // Inject loot_myshop.csv
    //
    // Mirrors the exact pattern of the original working event/store tools:
    // - Write all strings into rfile first (via utils::WriteString)
    // - Allocate cell array via utils::Allocate
    // - Fill cell values as relative offsets for string columns, raw int64 for INT columns
    // - After AllocateMemory, add the remote base address to every string cell's value
    //   (the same pointer fixup pattern as the original working tools)
    // =========================================================================
    static bool InjectMyShop(Process& bo4, std::string& errOut) {
        struct Row { const MyShopItem* item; int slot; };
        std::vector<Row> rows;
        if (g_deal1) rows.push_back({ g_deal1, 1 });
        if (g_deal2) rows.push_back({ g_deal2, 2 });
        if (rows.empty()) { errOut = "No items selected"; return false; }

        enum StringTableCellType : int32_t {
            STC_TYPE_STRING = 1,
            STC_TYPE_INT    = 4,
        };
        struct StringTableCell {
            byte    value[20]{};
            int32_t type{};
        };
        struct StringTableEntry {
            XHash     name{};
            int32_t   columnCount{};
            int32_t   rowCount{};
            int32_t   cellscount{};
            int32_t   unk24{};
            uintptr_t cells{};
            uintptr_t values{};
            uintptr_t unk48{};
            uintptr_t unk56{};
        }; static_assert(sizeof(StringTableEntry) == 0x40);

        std::vector<byte> rfile{};

        // Write all strings up front, collect offsets
        // (same pattern as original working tools)
        struct RowStrings { size_t nameLoc; size_t catLoc; };
        std::vector<RowStrings> strOffsets;
        strOffsets.reserve(rows.size());
        for (const auto& r : rows) {
            size_t nameLoc = utils::WriteString(rfile, r.item->itemName);
            size_t catLoc  = utils::WriteString(rfile, r.item->category);
            strOffsets.push_back({ nameLoc, catLoc });
        }

        // Allocate cell block: 6 cells per row, each StringTableCell is 24 bytes
        size_t cells = utils::Allocate(rfile, sizeof(StringTableCell) * 6 * rows.size());

        for (size_t i = 0; i < rows.size(); i++) {
            const MyShopItem* item = rows[i].item;
            int slot = rows[i].slot;
            StringTableCell* row = &reinterpret_cast<StringTableCell*>(&rfile[cells])[i * 6];

            // Col 0: item name - STRING pointer (relative offset, fixed up after AllocateMemory)
            row[0].type = STC_TYPE_STRING;
            *(size_t*)&row[0].value = strOffsets[i].nameLoc;

            // Col 1: category - STRING pointer (relative offset, fixed up after AllocateMemory)
            row[1].type = STC_TYPE_STRING;
            *(size_t*)&row[1].value = strOffsets[i].catLoc;

            // Col 2: myshop item ID
            row[2].type = STC_TYPE_INT;
            *(int64_t*)&row[2].value = (slot == 1) ? item->myshopId1 : item->myshopId2;

            // Col 3: COD Points price
            row[3].type = STC_TYPE_INT;
            *(int64_t*)&row[3].value = (slot == 1) ? item->codPts1 : item->codPts2;

            // Col 4: Reserve Cases cost
            row[4].type = STC_TYPE_INT;
            *(int64_t*)&row[4].value = (slot == 1) ? item->reserves1 : item->reserves2;

            // Col 5: slot number
            row[5].type = STC_TYPE_INT;
            *(int64_t*)&row[5].value = slot;
        }

        StringTableEntry entry{};
        entry.name.name   = loot_myshop_csv;
        entry.columnCount = 6;
        entry.rowCount    = (int32_t)rows.size();

        size_t    allocatedSize = rfile.size();
        uintptr_t allocated     = bo4.AllocateMemory(allocatedSize);
        if (!allocated) { errOut = std::format("Can't allocate {} bytes", rfile.size()); return false; }

        // Fix up all STRING column pointers: add remote base address
        // (same pattern as original tools - relative offset -> absolute remote pointer)
        entry.values = allocated + cells;
        StringTableCell* row = reinterpret_cast<StringTableCell*>(&rfile[cells]);
        for (size_t i = 0; i < rows.size(); i++) {
            *(uintptr_t*)row[i * 6 + 0].value += allocated; // col 0: item name
            *(uintptr_t*)row[i * 6 + 1].value += allocated; // col 1: category
        }

        if (!bo4.WriteMemory(allocated, rfile.data(), allocatedSize)) {
            bo4.FreeMemory(allocated, allocatedSize);
            errOut = "Can't write data";
            return false;
        }

        struct XAssetPoolEntry {
            uintptr_t pool; uint32_t itemSize; int32_t itemCount;
            byte isSingleton; int32_t itemAllocCount; uintptr_t freeHead;
        };

        auto pe = bo4.ReadMemoryObjectEx<XAssetPoolEntry>(
            bo4[games::bo4::offset::assetPool] +
            games::bo4::pool::ASSET_TYPE_STRINGTABLE * sizeof(XAssetPoolEntry));

        if (pe->itemSize != sizeof(entry)) {
            bo4.FreeMemory(allocated, allocatedSize);
            errOut = "INVALID POOL SIZE";
            return false;
        }

        auto entries = bo4.ReadMemoryArrayEx<StringTableEntry>(pe->pool, pe->itemAllocCount);
        for (size_t i = 0; i < (size_t)pe->itemAllocCount; i++) {
            if (entries[i].name.name != entry.name.name) continue;

            uintptr_t entryLoc = pe->pool + sizeof(entries[i]) * i;
            if (!bo4.WriteMemory(entryLoc, &entry, sizeof(entry))) {
                bo4.FreeMemory(allocated, allocatedSize);
                errOut = "Can't write pool entry";
                return false;
            }

            if (g_state.pid == bo4.m_pid && g_state.data)
                bo4.FreeMemory(g_state.data, g_state.dataSize);
            g_state.pid = bo4.m_pid; g_state.data = allocated; g_state.dataSize = allocatedSize;
            return true;
        }

        bo4.FreeMemory(allocated, allocatedSize);
        errOut = "Can't find loot_myshop.csv in pool";
        return false;
    }

    // =========================================================================
    // UI helpers
    // =========================================================================
    static void DrawItemCombo(const char* id, const MyShopItem*& selected) {
        const char* preview = selected ? selected->display : k_noChange;
        if (ImGui::BeginCombo(id, preview)) {
            if (ImGui::Selectable(k_noChange, selected == nullptr)) selected = nullptr;
            if (!selected) ImGui::SetItemDefaultFocus();
            ImGui::Separator();
            for (size_t n = 0; n < g_itemCount; n++) {
                bool isSel = (selected == &g_items[n]);
                if (ImGui::Selectable(g_items[n].display, isSel)) selected = &g_items[n];
                if (isSel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    // =========================================================================
    // Main NUI tool
    // =========================================================================
    void bo4_store_tool() {
        tool::nui::NuiUseDefaultWindow dw{};
        ImGui::SeparatorText("Store tool - My Deals");
        ImGui::TextDisabled("Requires sunset_features active (Event tool). Re-inject after each lobby.");

        ImGui::SeparatorText("My Deals - Slot 1");
        DrawItemCombo("##deal1", g_deal1);
        if (g_deal1)
            ImGui::TextDisabled("  %d CP  /  %d Reserves", g_deal1->codPts1, g_deal1->reserves1);

        ImGui::SeparatorText("My Deals - Slot 2");
        DrawItemCombo("##deal2", g_deal2);
        if (g_deal2)
            ImGui::TextDisabled("  %d CP  /  %d Reserves", g_deal2->codPts2, g_deal2->reserves2);

        ImGui::Separator();

        bool disabled = (!g_deal1 && !g_deal2);
        if (disabled) ImGui::BeginDisabled();
        if (ImGui::Button("Inject")) {
            g_notif.clear();
            Process bo4 = L"BlackOps4.exe";
            if (!bo4 || !bo4.Open()) {
                g_notif = "Can't open BlackOps4.exe";
            } else {
                std::string err{};
                if (!InjectMyShop(bo4, err)) {
                    g_notif = err;
                } else {
                    g_notif = "Injected loot_myshop.csv";
                    if (g_deal1) g_notif += std::format(" | Slot1: {}", g_deal1->itemName);
                    if (g_deal2) g_notif += std::format(" | Slot2: {}", g_deal2->itemName);
                }
            }
        }
        if (disabled) ImGui::EndDisabled();

        if (!g_notif.empty()) {
            ImGui::Separator();
            ImGui::TextWrapped("%s", g_notif.data());
        }
    }

    ADD_TOOL_NUI(bo4_store_tool, "BO4 Store tool", bo4_store_tool);

} // namespace
