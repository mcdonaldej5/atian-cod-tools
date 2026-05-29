#include <includes.hpp>
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>

namespace {
    // ==========================================================================
    // Constants
    // ==========================================================================

    // schedule_pc.csv controls all live events (seasons, XP, seasonal streams)
    constexpr uint64_t schedule_pc_csv = hash::Hash64("gamedata/events/schedule_pc.csv");

    // itemshop_slot_3_pc.csv controls the "Special Orders" Black Market slot
    // (sets the active loot_contract_* bundle offered in the Reserve tab)
    constexpr uint64_t itemshop_slot_3_csv = hash::Hash64("gamedata/events/itemshop_slot_3_pc.csv");

    constexpr uint64_t schedule_start = 1540770058;
    constexpr uint64_t schedule_end   = 2147367600;

    // ==========================================================================
    // Season Stream options
    // Seasons 1 through 9. Season 1 has no _ON event in events.json
    // (it was the base launch state); injecting it is a no-op for season logic
    // but is kept here for completeness. Seasons 2-9 have proper _ON entries.
    // ==========================================================================
    struct NamedEntry { const char* display; const char* eventName; };

    static const NamedEntry seasonOps[] = {
        { "",                 ""                  }, // blank / no-op
        { "Season 2",         "season_2_stream"   },
        { "Season 3",         "season_3_stream"   },
        { "Season 4",         "season_4_stream"   },
        { "Season 5",         "season_5_stream"   },
        { "Season 6",         "season_6_stream"   },
        { "Season 7",         "season_7_stream"   },
        { "Season 8",         "season_8_stream"   },
        { "Season 9",         "season_9"          },
    };

    // ==========================================================================
    // Operation / Seasonal event options (op2 dropdown)
    // All confirmed _ON entries in events.json.
    // NOTE: "Operation First Strike" does not exist in BO4's event system.
    // ==========================================================================
    static const NamedEntry seasonalOps[] = {
        { "",                              ""                            },
        // Season bonuses / bonus streams
        { "Season 2 Bonus Stream",         "season_2_bonus_stream"       },
        { "Season 3 Starter Pack",         "season_3_starter_pack"       },
        { "Season 4 Starter Pack",         "season_4_starter_pack"       },
        { "Season 6 Starter Pack",         "season_6_starter_pack"       },
        { "Early Season 4 Loot Drop",      "early_season_4_loot_drop"    },
        { "Mid Season 3 Loot Drop",        "mid_season_3_loot_drop"      },
        // Holiday / seasonal streams (MP event contracts)
        { "Halloween Event Stream",        "halloween_event_stream"      },
        { "Winter Event Stream",           "winter_event_stream"         },
        { "4th of July Event Stream",      "fourth_of_july_event_stream" },
        { "Labor Day Event Stream",        "labor_day_event_stream"      },
        // Shamrock & Awe 2019
        { "Shamrock Shock & Awe 2019",     "shamrock_shock_and_awe_2019" },
        { "Shamrock Free Bundle 2019",     "shamrock_free_bundle_2019"   },
        // Community events
        { "Community Event June 2019",     "community_event_june_2019"   },
        // Summer events
        { "Summer Break Bundle",           "summer_break_bundle"         },
        // Reserve / crate promos (shown in Black Market promo banner)
        { "Bribe Offer - Launch",          "bribe_offer_launch"          },
        { "Bribe Offer - Holidays 2019",   "bribe_offer_holidays_2019"   },
        { "Bribe Offer 2020",              "bribe_offer_2020"            },
        { "Free Weapon Bribe May 2020",    "free_pick_weapon_bribe_may_2020" },
        { "Mar 2019 Free Weapon Bribe",    "mar_2019_free_weapon_bribe"  },
        { "Half Off Weapon Bribes",        "half_off_pick_weapon_bribes" },
        { "Half Off No-Dupe Crate",        "half_off_nd_crate"           },
        { "Half Off Item Store",           "half_off_item_store"         },
        { "Half Off Tiers",                "half_off_tiers"              },
        // Tier discount promos
        { "Tier Discount x20 (Slot 1)",    "tier_discount_20_for_1000_slot_1" },
        { "Tier Discount x20 (Slot 2)",    "tier_discount_20_for_1000_slot_2" },
        { "Single Tier Purchase",          "single_tier_purchase"        },
        // Captain Price MW Pre-Order unlock
        { "Captain Price (MW Pre-Order)",  "cpt_price_mw_pre_order"      },
        // CPT / wlh
        { "WLH Event Results Processing",  "wlh_event_results_processing"},
        // Sunset / end-of-life features (enables Blackjack shop and no-dupe purchases)
        { "Sunset Features (Blackjack Shop + No-Dupe)", "sunset_features" },
    };

    // ==========================================================================
    // Zombies events (zm checkbox section)
    // zm_halloween_event_2018_ON -> sets zm_active_event_calling 1 (Halloween callings)
    // zm_winter_event_2018_ON    -> sets zm_active_event_calling 2 (Winter callings)
    // DO NOT combine with zm_daily_calling_* — the daily calling system resets
    // zm_active_event_calling to 0, which immediately cancels the event.
    // ==========================================================================

    // ==========================================================================
    // Special Orders (itemshop_slot_3 - Black Market Reserve tab contract slot)
    // These are the loot_contract_* bundles that appear as purchasable Special
    // Orders in the Reserve tab. Injected into a SEPARATE StringTable from the
    // main schedule. The event string must match a loot_contract_* item ID that
    // exists in the loot tables.
    // ==========================================================================
    static const NamedEntry specialOrders[] = {
        { "",                                     ""                               },
        // Season 2 (early)
        { "Valkyrie Contract",                    "loot_contract_valkyrie"         },
        { "Richtofen Contract",                   "loot_contract_richtofen"        },
        // Season 3
        { "Party Rock Contract",                  "loot_contract_party_rock"       },
        { "Nomad Heist Contract",                 "loot_contract_nomad_heist"      },
        { "Space Shuttle Contract",               "loot_contract_space_shuttle"    },
        { "Seraph Cop Contract",                  "loot_contract_seraph_cop"       },
        { "Crash Cop Contract",                   "loot_contract_crash_cop"        },
        { "Zero Cyberpunk Contract",              "loot_contract_zero_cyberpunk"   },
        // Season 4
        { "Misty Contract",                       "loot_contract_misty"            },
        { "Patriot Contract",                     "loot_contract_patriot"          },
        { "Carbon Cobra Contract",                "loot_contract_carbon_cobra"     },
        { "Ajax Roman Warrior Contract",          "loot_contract_ajax_roman_warrior"},
        { "Ahab's Revenge Contract",              "loot_contract_ahabs_revenge"    },
        { "Deep Voyage Contract",                 "loot_contract_deep_voyage"      },
        { "Sergei Contract",                      "loot_contract_sergei"           },
        // Season 5
        { "ICR-7 Mastercraft Contract",           "loot_contract_icr7_mastercraft" },
        { "Russman Contract",                     "loot_contract_russman"          },
        { "Replacer Vacation Contract",           "loot_contract_replacer_vacation"},
        { "Rampart Mastercraft Contract",         "loot_contract_rampart_mastercraft" },
        { "Tedd Contract",                        "loot_contract_tedd"             },
        { "Afterglow Contract",                   "loot_contract_afterglow"        },
        // Season 6
        { "GKS Mastercraft Contract",             "loot_contract_gks_mastercraft"  },
        { "Stuhlinger Contract",                  "loot_contract_stuhlinger"       },
        { "KN-57 Mastercraft Contract",           "loot_contract_kn57_mastercraft" },
        { "Grigori Weaver Contract",              "loot_contract_grigori_weaver"   },
        { "GKS Mastercraft Contract (S6 alt)",    "loot_contract_gks_mastercraft"  },
    };

    // ==========================================================================
    // Per-session state
    // ==========================================================================
    static const NamedEntry* g_season   = &seasonOps[ARRAYSIZE(seasonOps) - 1]; // default S9
    static const NamedEntry* g_seasonal = &seasonalOps[0];
    static const NamedEntry* g_order    = &specialOrders[0];

    static bool g_set2xp     = false;
    static bool g_set2wxp    = false;
    static bool g_set2m      = false;
    static bool g_set2nb     = false;
    static bool g_set2xptier = false;

    // Zombies events
    static bool g_zmHalloween2018 = false;
    static bool g_zmWinter2018    = false;

    // Previous allocations per StringTable, so old memory is freed on re-inject
    struct InjectionState {
        uintptr_t data{};
        size_t    dataSize{};
        DWORD     pid{};
    };
    static InjectionState g_scheduleState{};
    static InjectionState g_orderState{};

    static std::string g_notif{};

    // ==========================================================================
    // StringTable injection helper
    // Builds a minimal 4-column StringTable (event, start, end, platform) and
    // patches it into the asset pool, freeing the previous allocation.
    // Returns true on success; fills errOut on failure.
    // ==========================================================================
    static bool InjectScheduleTable(
        Process&            bo4,
        uint64_t            tableHash,
        const std::vector<const char*>& events,
        InjectionState&     state,
        std::string&        errOut)
    {
        enum StringTableCellType : int32_t {
            STC_TYPE_UNDEFINED = 0,
            STC_TYPE_STRING    = 1,
            STC_TYPE_HASHED    = 2,
            STC_TYPE_INT       = 4,
            STC_TYPE_FLOAT     = 5,
            STC_TYPE_BOOL      = 6
        };

        struct StringTableCell {
            byte value[20] = {};
            StringTableCellType type{};
        };

        struct StringTableEntry {
            XHash    name{};
            int32_t  columnCount{};
            int32_t  rowCount{};
            int32_t  cellscount{};
            int32_t  unk24{};
            uintptr_t cells{};
            uintptr_t values{};    // StringTableCell*
            uintptr_t unk48{};
            uintptr_t unk56{};
        }; static_assert(sizeof(StringTableEntry) == 0x40);

        std::vector<byte> rfile{};
        uintptr_t allocated{};
        size_t    allocatedSize{};

        // "all" platform string used for column 4 of each row
        size_t allLoc{ utils::WriteString(rfile, "all") };

        // Allocate cell block: 4 cells per row
        size_t cells{ utils::Allocate(rfile, sizeof(StringTableCell) * 4 * events.size()) };

        for (size_t i = 0; i < events.size(); i++) {
            size_t evLoc{ utils::WriteString(rfile, events[i]) };
            StringTableCell* row = &reinterpret_cast<StringTableCell*>(&rfile[cells])[i * 4];

            row[0].type = STC_TYPE_STRING;
            *(size_t*)&row[0].value = evLoc;

            row[1].type = STC_TYPE_INT;
            *(int64_t*)&row[1].value = schedule_start;

            row[2].type = STC_TYPE_INT;
            *(int64_t*)&row[2].value = schedule_end;

            row[3].type = STC_TYPE_STRING;
            *(size_t*)&row[3].value = allLoc;
        }

        StringTableEntry entry{};
        entry.name.name   = tableHash;
        entry.columnCount = 4;
        entry.rowCount    = (int32_t)events.size();

        allocatedSize = rfile.size();
        allocated     = bo4.AllocateMemory(allocatedSize);
        if (!allocated) {
            errOut = std::format("Can't allocate {} bytes", rfile.size());
            return false;
        }

        // Fix up string pointers to the absolute remote address
        entry.values = allocated + cells;
        StringTableCell* row = reinterpret_cast<StringTableCell*>(&rfile[cells]);
        for (size_t i = 0; i < events.size(); i++) {
            *(uintptr_t*)row[i * 4 + 0].value += allocated;
            *(uintptr_t*)row[i * 4 + 3].value += allocated;
        }

        if (!bo4.WriteMemory(allocated, rfile.data(), allocatedSize)) {
            bo4.FreeMemory(allocated, allocatedSize);
            errOut = std::format("Can't write {} bytes", rfile.size());
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
                errOut = "Can't write entry to pool";
                return false;
            }

            // Free previous allocation if from same process
            if (state.pid == bo4.m_pid && state.data) {
                bo4.FreeMemory(state.data, state.dataSize);
            }
            state.pid      = bo4.m_pid;
            state.data     = allocated;
            state.dataSize = allocatedSize;
            return true;
        }

        bo4.FreeMemory(allocated, allocatedSize);
        errOut = std::format("Can't find table 0x{:016X} in pool", tableHash);
        return false;
    }

    // ==========================================================================
    // Special Orders table injection
    // itemshop_slot_3_pc.csv uses a 3-column layout: event, start, end
    // (no platform column, unlike schedule_pc.csv)
    // ==========================================================================
    static bool InjectSpecialOrder(
        Process&        bo4,
        const char*     orderEvent,
        InjectionState& state,
        std::string&    errOut)
    {
        enum StringTableCellType : int32_t {
            STC_TYPE_STRING = 1,
            STC_TYPE_INT    = 4,
        };
        struct StringTableCell {
            byte value[20] = {};
            int32_t type{};
        };
        struct StringTableEntry {
            XHash    name{};
            int32_t  columnCount{};
            int32_t  rowCount{};
            int32_t  cellscount{};
            int32_t  unk24{};
            uintptr_t cells{};
            uintptr_t values{};
            uintptr_t unk48{};
            uintptr_t unk56{};
        }; static_assert(sizeof(StringTableEntry) == 0x40);

        constexpr uint64_t SO_START = 1545156000;
        constexpr uint64_t SO_END   = 2147367600;

        std::vector<byte> rfile{};
        size_t evLoc   = utils::WriteString(rfile, orderEvent);
        size_t cells   = utils::Allocate(rfile, sizeof(StringTableCell) * 3);

        StringTableCell* row = reinterpret_cast<StringTableCell*>(&rfile[cells]);
        row[0].type = STC_TYPE_STRING;
        *(size_t*)&row[0].value = evLoc;
        row[1].type = STC_TYPE_INT;
        *(int64_t*)&row[1].value = SO_START;
        row[2].type = STC_TYPE_INT;
        *(int64_t*)&row[2].value = SO_END;

        StringTableEntry entry{};
        entry.name.name   = itemshop_slot_3_csv;
        entry.columnCount = 3;
        entry.rowCount    = 1;

        size_t allocatedSize = rfile.size();
        uintptr_t allocated  = bo4.AllocateMemory(allocatedSize);
        if (!allocated) {
            errOut = std::format("Can't allocate {} bytes", rfile.size());
            return false;
        }

        entry.values = allocated + cells;
        *(uintptr_t*)row[0].value += allocated;

        if (!bo4.WriteMemory(allocated, rfile.data(), allocatedSize)) {
            bo4.FreeMemory(allocated, allocatedSize);
            errOut = "Can't write Special Order data";
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
                errOut = "Can't write Special Order entry";
                return false;
            }
            if (state.pid == bo4.m_pid && state.data)
                bo4.FreeMemory(state.data, state.dataSize);
            state.pid = bo4.m_pid; state.data = allocated; state.dataSize = allocatedSize;
            return true;
        }

        bo4.FreeMemory(allocated, allocatedSize);
        errOut = "Can't find itemshop_slot_3_pc.csv in pool";
        return false;
    }

    // ==========================================================================
    // Combo helper
    // ==========================================================================
    static void DrawNamedCombo(const char* id, const NamedEntry* items, size_t count,
                               const NamedEntry*& selected)
    {
        if (ImGui::BeginCombo(id, selected->display)) {
            for (size_t n = 0; n < count; n++) {
                bool isSelected = (selected == &items[n]);
                if (ImGui::Selectable(items[n].display, isSelected))
                    selected = &items[n];
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }

    // ==========================================================================
    // Main NUI tool
    // ==========================================================================
    void bo4_event_tool() {
        tool::nui::NuiUseDefaultWindow dw{};
        ImGui::SeparatorText("Event tool");

        static std::once_flag of{};
        std::call_once(of, [] {
            g_set2xp          = core::config::GetBool("ui.bo4event.s2xp",          false);
            g_set2wxp         = core::config::GetBool("ui.bo4event.s2wxp",         false);
            g_set2m           = core::config::GetBool("ui.bo4event.s2m",           false);
            g_set2nb          = core::config::GetBool("ui.bo4event.s2nb",          false);
            g_set2xptier      = core::config::GetBool("ui.bo4event.s2xptier",      false);
            g_zmHalloween2018 = core::config::GetBool("ui.bo4event.zmhw18",        false);
            g_zmWinter2018    = core::config::GetBool("ui.bo4event.zmwinter18",    false);
            tool::nui::SaveNextConfig();
        });

        // -- XP / bonus checkboxes ------------------------------------------
        bool c{};
        ImGui::SeparatorText("Multipliers");
        if (ImGui::Checkbox("Double XP (MP + ZM)",         &g_set2xp))     { core::config::SetBool("ui.bo4event.s2xp",      g_set2xp);     c = true; }
        if (ImGui::Checkbox("Double Weapon XP (MP + ZM)",  &g_set2wxp))    { core::config::SetBool("ui.bo4event.s2wxp",     g_set2wxp);    c = true; }
        if (ImGui::Checkbox("Double Merits (Blackout)",     &g_set2m))      { core::config::SetBool("ui.bo4event.s2m",       g_set2m);      c = true; }
        if (ImGui::Checkbox("Double Nebula (ZM)",           &g_set2nb))     { core::config::SetBool("ui.bo4event.s2nb",      g_set2nb);     c = true; }
        if (ImGui::Checkbox("Double Tiers (MP + Blackout)", &g_set2xptier)) { core::config::SetBool("ui.bo4event.s2xptier", g_set2xptier); c = true; }

        // -- Zombies event checkboxes ----------------------------------------
        ImGui::SeparatorText("Zombies Events");
        ImGui::TextDisabled("Activates event callings. Do not combine with zm_daily_calling.");
        if (ImGui::Checkbox("ZM Halloween 2018 (zm_active_event_calling = 1)", &g_zmHalloween2018)) {
            if (g_zmHalloween2018) g_zmWinter2018 = false; // mutually exclusive
            core::config::SetBool("ui.bo4event.zmhw18",     g_zmHalloween2018); c = true;
        }
        if (ImGui::Checkbox("ZM Winter 2018 (zm_active_event_calling = 2)",    &g_zmWinter2018)) {
            if (g_zmWinter2018) g_zmHalloween2018 = false; // mutually exclusive
            core::config::SetBool("ui.bo4event.zmwinter18", g_zmWinter2018);    c = true;
        }

        // -- Season stream dropdown -----------------------------------------
        ImGui::SeparatorText("Season Stream (sets loot_season_stream + season-specific unlocks)");
        DrawNamedCombo("##season", seasonOps, ARRAYSIZE(seasonOps), g_season);

        // -- Seasonal / operation dropdown ----------------------------------
        ImGui::SeparatorText("Seasonal Operation / Event");
        DrawNamedCombo("##seasonal", seasonalOps, ARRAYSIZE(seasonalOps), g_seasonal);

        // -- Special Orders dropdown ----------------------------------------
        ImGui::SeparatorText("Special Order (Black Market Reserve tab contract)");
        ImGui::TextDisabled("Injects itemshop_slot_3_pc.csv. Restart lobby to see updated contract.");
        DrawNamedCombo("##order", specialOrders, ARRAYSIZE(specialOrders), g_order);

        if (c) tool::nui::SaveNextConfig();
        ImGui::Separator();

        // -- Inject button --------------------------------------------------
        if (ImGui::Button("Inject")) {
            g_notif.clear();

            // Build mandatory base events (always present)
            std::vector<const char*> events{};
            events.push_back("zm_lab_titanium_treble_slot_1");
            events.push_back("zm_lab_tungsten_tripler_slot_2");
            events.push_back("digital_refresh_v3");
            events.push_back("reserves_drop_12");
            events.push_back("bribe_offer_holidays_2019");
            events.push_back("reserve_completion_meter");
            events.push_back("sunset_features");
            events.push_back("season_9");

            // XP / bonus multipliers
            if (g_set2xp) {
                events.push_back("global_2xp_mp_server");
                events.push_back("global_2xp_mp_client");
                events.push_back("global_2xp_zm_server");
                events.push_back("global_2xp_zm_client");
            }
            if (g_set2wxp) {
                events.push_back("global_2wxp_mp_server");
                events.push_back("global_2wxp_mp_client");
                events.push_back("global_2wxp_zm_server");
                events.push_back("global_2wxp_zm_client");
            }
            if (g_set2m) {
                events.push_back("global_2x_merits_wz_server");
                events.push_back("global_2x_merits_wz_client");
            }
            if (g_set2nb) {
                events.push_back("global_2xnp_zm_server");
                events.push_back("global_2xnp_zm_client");
            }
            if (g_set2xptier) {
                events.push_back("global_2xtier_mp_server");
                events.push_back("global_2xtier_mp_client");
                events.push_back("global_2xtier_wz_server");
                events.push_back("global_2xtier_wz_client");
            }

            // Zombies events — mutually exclusive (both set zm_active_event_calling)
            // zm_halloween_event_2018_ON -> calling = 1
            // zm_winter_event_2018_ON    -> calling = 2
            if (g_zmHalloween2018) events.push_back("zm_halloween_event_2018_ON");
            else if (g_zmWinter2018) events.push_back("zm_winter_event_2018_ON");

            // Season stream
            if (*g_season->eventName) events.push_back(g_season->eventName);

            // Seasonal operation
            if (*g_seasonal->eventName) events.push_back(g_seasonal->eventName);

            LOG_TRACE("Injecting schedule ({} events):", events.size());
            for (const char* ev : events) LOG_TRACE("  {}", ev);

            Process bo4 = L"BlackOps4.exe";
            if (!bo4 || !bo4.Open()) {
                g_notif = "Can't open BlackOps4.exe";
            } else {
                std::string err{};
                bool ok = InjectScheduleTable(bo4, schedule_pc_csv, events, g_scheduleState, err);
                if (!ok) {
                    g_notif = "[Schedule] " + err;
                } else {
                    // Inject Special Order if selected
                    if (*g_order->eventName) {
                        LOG_TRACE("Injecting Special Order: {}", g_order->eventName);
                        if (!InjectSpecialOrder(bo4, g_order->eventName, g_orderState, err))
                            g_notif = "[Special Order] " + err;
                        else
                            g_notif = "Injected schedule + Special Order";
                    } else {
                        g_notif = "Injected schedule";
                    }
                }
            }
        }

        // -- Status ---------------------------------------------------------
        if (!g_notif.empty()) {
            ImGui::Separator();
            ImGui::Text("%s", g_notif.data());
        }
    }

    ADD_TOOL_NUI(bo4_event_tool, "BO4 Events tool", bo4_event_tool);

} // namespace
