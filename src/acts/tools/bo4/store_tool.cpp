#include <includes.hpp>
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>

namespace {

    // =========================================================================
    // My Deals are driven by gamedata/loot/loot_myshop.csv, NOT by
    // itemshop_slot_1/2_pc.csv. The slot_1/2 CSVs are the pre-sunset rotation
    // schedule and are ignored once loot_sunsetblackjackshopactive = 1 (which
    // sunset_features sets). loot_myshop.csv has 6 columns:
    //   item_name (hashed), category (hashed), myshop_item_id (int),
    //   cod_points_price (int), reserve_cases_cost (int), slot_number (int)
    // The game randomly selects one item per slot from this table every 12 hours
    // (client-side timer). To force a specific item, we inject a table with
    // exactly one entry per slot, leaving the picker no other choice.
    // =========================================================================
    constexpr uint64_t loot_myshop_csv = hash::Hash64("gamedata/loot/loot_myshop.csv");

    // Known category hashes from loot_myshop.csv
    // #hash_731315ef3fb1ae07 = always-available featured items
    // #loot_stream           = current season stream items
    // #special_order         = special order contracts
    constexpr uint64_t CAT_FEATURED      = hash::Hash64("hash_731315ef3fb1ae07");  // will be resolved differently - see note below
    constexpr uint64_t CAT_LOOT_STREAM   = hash::Hash64("loot_stream");
    constexpr uint64_t CAT_SPECIAL_ORDER = hash::Hash64("special_order");

    // =========================================================================
    // Item descriptor: all fields needed to write one loot_myshop row
    // myshop_id: the unique ID from loot_myshop col3 (leading digit = slot)
    // cod_pts:   COD Points price
    // reserves:  Reserve Cases cost (0 = CP only)
    // cat:       category hash (determines which tab the item appears under)
    // =========================================================================
    struct MyShopItem {
        const char* display;     // UI label
        const char* itemName;    // raw name (without # prefix) for hash::Hash64
        uint64_t    cat;         // category hash
        int32_t     myshopId1;   // loot_myshop id when in slot 1 (1XXXXXX)
        int32_t     myshopId2;   // loot_myshop id when in slot 2 (2XXXXXX)
        int32_t     codPts1;     // COD Points price in slot 1
        int32_t     codPts2;     // COD Points price in slot 2
        int32_t     reserves1;   // Reserve Cases cost in slot 1
        int32_t     reserves2;   // Reserve Cases cost in slot 2
    };

    // Category hash for the "always available" pool.
    // In the CSV this appears as #hash_731315ef3fb1ae07, meaning the string
    // "hash_731315ef3fb1ae07" was hashed. We pass it pre-computed.
    static constexpr uint64_t CAT_HASH_FEATURED = 0x731315ef3fb1ae07ULL;

    // =========================================================================
    // Full item pool sourced from loot_myshop.csv - all named (non-unknown) items
    // Items with only #hash_ identifiers are excluded as they cannot be targeted
    // =========================================================================
    static const MyShopItem g_shopItems[] = {
        // display name                             itemName                                cat                  id1      id2      cp1   cp2   rsv1      rsv2
        // -- Camos --
        { "DLC1: Masks Active Camo (S2)",           "camo_active_dlc1_masks_wrapper",       CAT_HASH_FEATURED,   1373223, 2373223, 500,  320,  1300000,  0      },
        { "DLC1: WW2 Active Camo (S2)",             "camo_active_dlc1_ww2_wrapper",         CAT_HASH_FEATURED,   1868993, 2868993, 360,  320,  1303,     1303   },
        { "DLC5: TR Midburst Bomber Camo (S8)",     "camo_active_sig_tr_midburst_bomber_base", CAT_LOOT_STREAM,  1640460, 0,       1000, 0,    0,        0      },
        // -- Characters --
        { "Takeo (ZM Character)",                   "loot_character_takeo_ofc",             CAT_HASH_FEATURED,   1284655, 2284655, 720,  640,  1851,     1851   },
        // -- Emotes --
        { "Stabby Banana Emote",                    "loot_emote_exotic_shared_stabby_banana", CAT_HASH_FEATURED, 1212907, 2212907, 360,  320,  1116,     1116   },
        { "Shush Emote",                            "loot_emote_shared_shush",              CAT_HASH_FEATURED,   1471455, 2471455, 180,  160,  0,        0      },
        // -- Mastercrafts --
        { "Mastercraft: Vampire Hunter",            "mastercraft_vampire_hunter",           CAT_HASH_FEATURED,   1285318, 2285318, 900,  800,  10000,    0      },
        // -- Outfits --
        { "Outfit: Flaming Skull Warpaint",         "outfit_global_warpaint_bundle_flaming_skull", CAT_HASH_FEATURED, 1255038, 2255038, 225, 200, 0,   0      },
        { "Outfit: Marauder (Battery)",             "outfit_marauder_rare4_battery",        CAT_HASH_FEATURED,   1960738, 2960738, 180,  160,  0,        0      },
        { "Outfit: Northern Lights Legendary (Firebreak)", "outfit_northern_lights_legendary3_firebreak", CAT_HASH_FEATURED, 1240245, 2240245, 540, 480, 2679, 2679 },
        { "Outfit: Zero Legendary 1",               "outfit_snake_legendary1_zero",         CAT_HASH_FEATURED,   1127737, 2127737, 360,  320,  1117,     1117   },
        { "Outfit: Seraph Legendary 2",             "outfit_snake_legendary2_seraph",       CAT_HASH_FEATURED,   1846821, 2846821, 360,  320,  1006,     1006   },
        { "Outfit: Battery Base Palette",           "outfit_warrior_base_pal_exo1_battery", CAT_HASH_FEATURED,   1849642, 2849642, 720,  640,  0,        0      },
        { "Outfit: Crash Epic 1",                   "outfit_warrior_epic1_crash",           CAT_HASH_FEATURED,   1205283, 2205283, 720,  640,  2494,     2494   },
        { "Outfit: Prophet Epic 1",                 "outfit_warrior_epic1_prophet",         CAT_HASH_FEATURED,   1534910, 2534910, 720,  640,  1344,     1344   },
        { "Outfit: Zero Epic 2",                    "outfit_warrior_epic2_zero",            CAT_HASH_FEATURED,   1012900, 2012900, 720,  640,  1289,     1289   },
        // -- Reserve Crate Bundles --
        { "3x Reserve Cases",                       "abr_3_crates",                         CAT_HASH_FEATURED,   1123898, 0,       1600, 0,    0,        0      },
        { "5x Reserve Cases",                       "abr_5_crates",                         CAT_HASH_FEATURED,   1186962, 0,       2000, 0,    0,        0      },
        { "8x Reserve Cases",                       "abr_8_crates",                         CAT_HASH_FEATURED,   1163052, 2163052, 2000, 2400, 50000,    0      },
        // -- Special Order Contracts (Muertos) --
        { "Contract: Ajax Muertos",                 "loot_contract_ajax_muertos",           CAT_SPECIAL_ORDER,   1943942, 2943942, 480,  500,  807,      807    },
        { "Contract: Battery Muertos",              "loot_contract_battery_muertos",        CAT_SPECIAL_ORDER,   1519612, 2519612, 400,  500,  807,      807    },
        { "Contract: Crash Muertos",                "loot_contract_crash_muertos",          CAT_SPECIAL_ORDER,   1406463, 2406463, 490,  500,  807,      807    },
        { "Contract: Firebreak Muertos",            "loot_contract_firebreak_muertos",      CAT_SPECIAL_ORDER,   1573245, 2573245, 400,  500,  807,      807    },
        { "Contract: Nomad Muertos",                "loot_contract_nomad_muertos",          CAT_SPECIAL_ORDER,   1518581, 2518581, 480,  500,  807,      807    },
        { "Contract: Prophet Muertos",              "loot_contract_prophet_muertos",        CAT_SPECIAL_ORDER,   1220241, 2220241, 490,  500,  807,      807    },
        { "Contract: Recon Muertos",                "loot_contract_recon_muertos",          CAT_SPECIAL_ORDER,   1255337, 2255337, 480,  500,  807,      807    },
        { "Contract: Ruin Muertos",                 "loot_contract_ruin_muertos",           CAT_SPECIAL_ORDER,   1573672, 2573672, 400,  500,  807,      807    },
        { "Contract: Seraph Muertos",               "loot_contract_seraph_muertos",         CAT_SPECIAL_ORDER,   1617121, 2617121, 480,  500,  807,      807    },
        { "Contract: Torque Muertos",               "loot_contract_torque_muertos",         CAT_SPECIAL_ORDER,   1446502, 2446502, 490,  500,  807,      807    },
        // -- Special Order Contracts (Prestige/Legendary) --
        { "Contract: Recon Samurai",                "loot_contract_recon_samurai",          CAT_SPECIAL_ORDER,   1088436, 2088436, 580,  600,  2202,     2202   },
        { "Contract: Ruin Modern Viking",           "loot_contract_ruin_modern_viking",     CAT_SPECIAL_ORDER,   1424760, 2424760, 580,  600,  2202,     2202   },
        { "Contract: Seraph Crimson Lotus",         "loot_contract_seraph_crimson_lotus",   CAT_SPECIAL_ORDER,   1257893, 2257893, 500,  600,  2202,     2202   },
        { "Contract: Torque Highlander",            "loot_contract_torque_highlander",      CAT_SPECIAL_ORDER,   1366927, 2366927, 600,  800,  2202,     2202   },
        { "Contract: Crash Cop",                    "loot_contract_crash_cop",              CAT_SPECIAL_ORDER,   1444031, 2444031, 700,  800,  2203,     2203   },
        { "Contract: Nomad Heist",                  "loot_contract_nomad_heist",            CAT_SPECIAL_ORDER,   1365499, 2365499, 700,  800,  2203,     2203   },
        { "Contract: Seraph Cop",                   "loot_contract_seraph_cop",             CAT_SPECIAL_ORDER,   1913949, 2913949, 700,  800,  2203,     2203   },
        { "Contract: Zero Cyberpunk",               "loot_contract_zero_cyberpunk",         CAT_SPECIAL_ORDER,   1404466, 2404466, 500,  600,  2458,     2458   },
        { "Contract: Richtofen (ZM)",               "loot_contract_richtofen",              CAT_SPECIAL_ORDER,   1781759, 2781759, 500,  580,  2694,     2694   },
    };
    static constexpr size_t g_shopItemCount = ARRAYSIZE(g_shopItems);

    // Sentinel "no-op" entries (keep random pool)
    static const MyShopItem g_noChange = { "(Keep Random / No Override)", "", 0, 0, 0, 0, 0, 0, 0 };

    // =========================================================================
    // Per-slot selection state
    // =========================================================================
    static const MyShopItem* g_deal1   = &g_noChange;
    static const MyShopItem* g_deal2   = &g_noChange;
    static std::string        g_notif{};

    struct InjectionState {
        uintptr_t data{};
        size_t    dataSize{};
        DWORD     pid{};
    };
    static InjectionState g_myshopState{};

    // =========================================================================
    // loot_myshop.csv injection
    //
    // Column layout (6 columns, all cells are 24 bytes each):
    //   [0] item_name  -> STC_TYPE_HASHED  (uint64 hash of the item name)
    //   [1] category   -> STC_TYPE_HASHED  (uint64 hash of category string)
    //   [2] myshop_id  -> STC_TYPE_INT     (unique per-slot item identifier)
    //   [3] cod_points -> STC_TYPE_INT     (COD Points price)
    //   [4] reserves   -> STC_TYPE_INT     (Reserve Cases cost)
    //   [5] slot       -> STC_TYPE_INT     (1, 2, or 3)
    //
    // We write one row per selected slot. If a slot is set to "No Override" it
    // is excluded from the injected table entirely (leaving the random pool for
    // that slot intact is NOT possible once we replace the table — so in practice
    // if either slot has an item selected we inject both; if neither is set we
    // skip injection).
    // =========================================================================
    static bool InjectMyShop(
        Process&        bo4,
        InjectionState& state,
        std::string&    errOut)
    {
        // Build the list of rows to write
        struct Row { const MyShopItem* item; int slot; };
        std::vector<Row> rows{};

        if (*g_deal1->itemName) rows.push_back({ g_deal1, 1 });
        if (*g_deal2->itemName) rows.push_back({ g_deal2, 2 });

        if (rows.empty()) {
            errOut = "No deals selected — nothing to inject";
            return false;
        }

        enum StringTableCellType : int32_t {
            STC_TYPE_STRING = 1,
            STC_TYPE_HASHED = 2,
            STC_TYPE_INT    = 4,
        };

        struct StringTableCell {
            byte    value[20] = {};
            int32_t type{};
        }; // 24 bytes total

        struct StringTableEntry {
            XHash     name{};
            int32_t   columnCount{};
            int32_t   rowCount{};
            int32_t   cellscount{};
            int32_t   unk24{};
            uintptr_t cells{};
            uintptr_t values{};   // StringTableCell*
            uintptr_t unk48{};
            uintptr_t unk56{};
        }; static_assert(sizeof(StringTableEntry) == 0x40);

        std::vector<byte> rfile{};

        // Allocate cell block: 6 cells per row
        size_t cells = utils::Allocate(rfile, sizeof(StringTableCell) * 6 * rows.size());

        for (size_t i = 0; i < rows.size(); i++) {
            const MyShopItem* item = rows[i].item;
            int slot = rows[i].slot;

            StringTableCell* row = &reinterpret_cast<StringTableCell*>(&rfile[cells])[i * 6];

            // Col 0: item name hash
            row[0].type = STC_TYPE_HASHED;
            *(uint64_t*)&row[0].value = hash::Hash64(item->itemName);

            // Col 1: category hash
            row[1].type = STC_TYPE_HASHED;
            *(uint64_t*)&row[1].value = item->cat;

            // Col 2: myshop item ID (slot-specific)
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
        if (!allocated) {
            errOut = std::format("Can't allocate {} bytes", rfile.size());
            return false;
        }

        // No string pointers to fix up — all string cells use HASHED (uint64 value),
        // not STRING (pointer). No pointer arithmetic needed.
        entry.values = allocated + cells;

        if (!bo4.WriteMemory(allocated, rfile.data(), allocatedSize)) {
            bo4.FreeMemory(allocated, allocatedSize);
            errOut = "Can't write loot_myshop data";
            return false;
        }

        struct XAssetPoolEntry {
            uintptr_t pool;
            uint32_t  itemSize;
            int32_t   itemCount;
            byte      isSingleton;
            int32_t   itemAllocCount;
            uintptr_t freeHead;
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
                errOut = "Can't write loot_myshop pool entry";
                return false;
            }

            if (state.pid == bo4.m_pid && state.data)
                bo4.FreeMemory(state.data, state.dataSize);
            state.pid      = bo4.m_pid;
            state.data     = allocated;
            state.dataSize = allocatedSize;
            return true;
        }

        bo4.FreeMemory(allocated, allocatedSize);
        errOut = "Can't find loot_myshop.csv in pool";
        return false;
    }

    // =========================================================================
    // Combo helper
    // =========================================================================
    static void DrawItemCombo(const char* id, const MyShopItem*& selected)
    {
        if (ImGui::BeginCombo(id, selected->display)) {
            // First entry: no-override sentinel
            if (ImGui::Selectable(g_noChange.display, selected == &g_noChange))
                selected = &g_noChange;
            if (selected == &g_noChange) ImGui::SetItemDefaultFocus();

            ImGui::Separator();

            for (size_t n = 0; n < g_shopItemCount; n++) {
                bool isSelected = (selected == &g_shopItems[n]);
                if (ImGui::Selectable(g_shopItems[n].display, isSelected))
                    selected = &g_shopItems[n];
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    // =========================================================================
    // Main NUI tool
    // =========================================================================
    void bo4_store_tool() {
        tool::nui::NuiUseDefaultWindow dw{};
        ImGui::SeparatorText("Store tool - My Deals (Blackjack Shop)");

        ImGui::TextDisabled("Injects loot_myshop.csv. Requires sunset_features active (Event tool).");
        ImGui::TextDisabled("The game refreshes deals every 12h client-side. Re-inject after lobby.");

        ImGui::SeparatorText("My Deals - Slot 1");
        DrawItemCombo("##deal1", g_deal1);
        if (*g_deal1->itemName) {
            ImGui::SameLine();
            ImGui::TextDisabled("%d CP / %d Rsv", g_deal1->codPts1, g_deal1->reserves1);
        }

        ImGui::SeparatorText("My Deals - Slot 2");
        DrawItemCombo("##deal2", g_deal2);
        if (*g_deal2->itemName) {
            ImGui::SameLine();
            ImGui::TextDisabled("%d CP / %d Rsv", g_deal2->codPts2, g_deal2->reserves2);
        }

        ImGui::Separator();

        bool nothingSelected = !*g_deal1->itemName && !*g_deal2->itemName;

        if (nothingSelected) ImGui::BeginDisabled();
        if (ImGui::Button("Inject")) {
            g_notif.clear();
            Process bo4 = L"BlackOps4.exe";
            if (!bo4 || !bo4.Open()) {
                g_notif = "Can't open BlackOps4.exe";
            } else {
                std::string err{};
                if (!InjectMyShop(bo4, g_myshopState, err))
                    g_notif = err;
                else {
                    g_notif = "Injected loot_myshop.csv";
                    if (*g_deal1->itemName) g_notif += std::format(" | Slot1: {}", g_deal1->itemName);
                    if (*g_deal2->itemName) g_notif += std::format(" | Slot2: {}", g_deal2->itemName);
                }
            }
        }
        if (nothingSelected) ImGui::EndDisabled();

        if (!g_notif.empty()) {
            ImGui::Separator();
            ImGui::TextWrapped("%s", g_notif.data());
        }
    }

    ADD_TOOL_NUI(bo4_store_tool, "BO4 Store tool", bo4_store_tool);

} // namespace
