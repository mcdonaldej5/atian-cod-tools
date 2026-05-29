#include <includes.hpp>
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>

namespace {
	// string,int,int,string
	// Slot 1 = "My Deals" cosmetic slot 1 (weapon camos, mastercrafts, melee)
	constexpr uint64_t itemshop_slot_1 = hash::Hash64("gamedata/events/itemshop_slot_1_pc.csv");
	// Slot 2 = "My Deals" cosmetic slot 2 (outfits, gestures, jumppacks)
	constexpr uint64_t itemshop_slot_2 = hash::Hash64("gamedata/events/itemshop_slot_2_pc.csv");
	// Reserve = "Reserve Stack" deals
	constexpr uint64_t reserve_deals   = hash::Hash64("gamedata/events/reserve_deals_schedule_pc.csv");

	constexpr uint64_t start = 1545156000;
	constexpr uint64_t end   = 2147367600;

	// -------------------------------------------------------------------------
	// My Deals Slot 1 — Weapon camos, mastercrafts, reactive camos, melee
	// Full rotation pulled from itemshop_slot_1_pc.csv (all seasons)
	// -------------------------------------------------------------------------
	static const char* slot1_items[]{
		// Season 1
		"outfit_warrior_base_pal_exo1_battery",
		"camo_active_dlc1_ww2_wrapper",
		"camo_dlc2_benjamins",
		"camo_dlc2_smooch",
		"camo_dlc2_donuts",
		// Season 2
		"mastercraft_enforcer_bundle",
		"mastercraft_vampire_hunter",
		"mastercraft_safari_bundle",
		"mastercraft_hades_snake",
		"mastercraft_sdm_wetworks",
		// Reactive Camos
		"weaponcamo_cherryblossom_bundle",
		"weaponcamo_rocketbox_bundle",
		"camo_active_dlc1_masks_wrapper",
		"camo_active_dlc2_zombie_flick_wrapper",
		"camo_active_dlc3_celestial_wrapper",
		"camo_dlc3_armor",
		"camo_active_dlc4_115_wrapper",
		"camo_active_dlc4_brains_wrapper_bundle",
		"camo_dlc4_zombie_maitai_bundle",
		"camo_dlc4_picnic_royale_bundle",
		// Season 5 reactive
		"plasmadrive_reactive_camo_bundle",
		"dogfight_reactive_camo_bundle",
		"treasure_vault_reactive_camo_bundle",
		"party_rock_reactive_camo_bundle",
		"bacon_weapon_camo_bundle",
		// Season 6
		"camo_active_dlc5_phased_wrapper_bundle",
		"camo_active_dlc5_shattered_wrapper",
		"sniper_powerbolt_mk2_bundle",
		"ar_accurate_featured_item_bundle",
		"ar_modular_mk2_bundle",
		"tr_midburst_mk2_bundle",
		// Melee
		"rising_tide_melee_bundle",
		"zombie_arm_melee_bundle",
		"melee_stopsign_bundle",
		"melee_action_figure_bundle",
		// Crates / misc
		"abr_8_crates",
	};

	// -------------------------------------------------------------------------
	// My Deals Slot 2 — Outfits, gestures, jumppacks, emotes
	// Full rotation pulled from itemshop_slot_2_pc.csv (all seasons)
	// -------------------------------------------------------------------------
	static const char* slot2_items[]{
		// Season 1 outfits / gestures
		"outfit_snake_legendary2_seraph",
		"outfit_warrior_epic1_prophet",
		"outfit_snake_legendary1_zero",
		"jumppack_heist",
		"jumppack_chinese_new_year",
		"jumppack_american_flag",
		"outfit_warrior_epic1robber_firebreak",
		"outfit_northern_lights_legendary2_crash",
		"outfit_warrior_epic1robber_ruin",
		"outfit_global_warpaint_bundle_jester",
		"outfit_global_warpaint_bundle_spades",
		"loot_emote_shared_shuffle_dance",
		"gesture_fishmictoast_bundle",
		"gesture_dance_bundle",
		// Season 2 outfits
		"outfit_warrior_epic1_wetworks_spectre",
		"outfit_warrior_epic1_wetworks_torque",
		"outfit_warrior_epic1_wetworks_battery",
		"outfit_assassin_legendary1_ruin",
		"loot_character_takeo_ofc",
		"prophet_skipper_outfit_bundle",
		"outrider_buccaneer_outfit_bundle",
		"torque_smokedup_outfit_bundle",
		"outfit_warrior_epic1_crash",
		// Season 5 Apocalypse / Zombie
		"spectre_apocalypse_outfit_bundle",
		"outrider_apocalypse_outfit_bundle",
		"ruin_apocalypse_outfit_bundle",
		"funky_dance_bundle",
		"outfit_zombie_epic1_reaper_bundle",
		"outfit_zombie_epic1_firebreak_bundle",
		"outfit_zombie_epic1_nomad_bundle",
		"summer_gesture_bundle",
		// Season 6 Super Hero
		"outfit_super_hero_epic1_seraph_bundle",
		"outfit_super_hero_epic1_spectre_bundle",
		"outfit_super_hero_epic1_prophet_bundle",
		"outfit_super_hero_epic1_firebreak_bundle",
		"outfit_super_hero_epic1_ruin_bundle",
		"outfit_northern_lights_legendary3_firebreak",
	};

	// -------------------------------------------------------------------------
	// Reserve Stack — Bundles, triple plays, starter packs, Blackout characters
	// Full rotation pulled from reserve_deals_schedule_pc.csv (all seasons)
	// -------------------------------------------------------------------------
	static const char* reserve_items[]{
		// Weapon bundles
		"ar_accurate_bundle",
		"lmg_standard_bundle",
		"smg_accurate_bundle",
		"tr_powersemi_bundle",
		"smg_handling_greatlion_mastercraft_bundle",
		// Outfit bundles
		"battery_queensguard_outfit_bundle",
		"firebreak_dragonslayer_outfit_bundle",
		"torque_assassin_outfit_bundle",
		"battery_assassin_outfit_bundle",
		"ajax_assassin_outfit_bundle",
		"recon_wetworks_outfit_bundle",
		"zero_persian_outfit_bundle",
		"recon_samurai_outfit_bundle",
		"seraph_heist_outfit_bundle",
		// Blackout characters
		"hudson_blackout_character_bundle",
		"dempsey_blackout_character_bundle",
		"warden_character_bundle",
		"nikolai_bundle",
		// Triple plays / seasonal
		"triple_play_mar2019",
		"daysofsummer_triple_play_01",
		"daysofsummer_triple_play_02",
		"daysofsummer_vacation_bundle",
		"season5_triple_play_01",
		"season5_triple_play_02",
		"season5_triple_play_03",
		"season6_triple_play",
		// Melee
		"melee_club_bundle",
		"melee_coinbag_bundle",
		"rising_tide_melee_bundle_offer",
		// Reactive camo
		"retro_vegas_reactive_camo_bundle",
		// Starter packs
		"starter_pack_season5",
		"starter_pack_season_6",
	};

	// Per-slot state -------------------------------------------------------
	struct SlotState {
		const char* selected{};
		uintptr_t   currentData{};
		size_t      currentDataSize{};
		DWORD       currentPID{};
	};

	static SlotState g_slot1{ slot1_items[ARRAYSIZE(slot1_items) - 1] };
	static SlotState g_slot2{ slot2_items[0] };
	static SlotState g_reserve{ reserve_items[0] };
	static std::string g_notif{};

	// Helper: build and inject a StringTable for a given asset hash/item -----
	static bool InjectStringTable(
		Process& bo4,
		uint64_t tableHash,
		const char* item,
		SlotState& state,
		std::string& err)
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
			uintptr_t values{};   // StringTableCell*
			uintptr_t unk48{};
			uintptr_t unk56{};
		}; static_assert(sizeof(StringTableEntry) == 0x40);

		std::vector<byte>   rfile{};
		uintptr_t           allocated{};
		size_t              allocatedSize{};

		// Write event string and build one row: [event_string, start_int, end_int]
		size_t evLoc  = utils::WriteString(rfile, item);
		size_t cells  = utils::Allocate(rfile, sizeof(StringTableCell) * 3);

		StringTableCell* row = reinterpret_cast<StringTableCell*>(&rfile[cells]);

		row[0].type = STC_TYPE_STRING;
		*(size_t*)&(row[0].value) = evLoc;

		row[1].type = STC_TYPE_INT;
		*(int64_t*)&(row[1].value) = start;

		row[2].type = STC_TYPE_INT;
		*(int64_t*)&(row[2].value) = end;

		StringTableEntry entry{};
		entry.name.name    = tableHash;
		entry.columnCount  = 3;
		entry.rowCount     = 1;

		allocatedSize = rfile.size();
		allocated     = bo4.AllocateMemory(allocatedSize);
		if (!allocated) {
			err = std::format("Can't allocate {} bytes", rfile.size());
			return false;
		}

		// Link string pointer to allocated base
		entry.values = allocated + cells;
		*(uintptr_t*)row[0].value += allocated;

		if (!bo4.WriteMemory(allocated, rfile.data(), allocatedSize)) {
			bo4.FreeMemory(allocated, allocatedSize);
			err = std::format("Can't write {} bytes", rfile.size());
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
			err = "INVALID POOL SIZE";
			return false;
		}

		auto entries = bo4.ReadMemoryArrayEx<StringTableEntry>(pe->pool, pe->itemAllocCount);

		for (size_t i = 0; i < (size_t)pe->itemAllocCount; i++) {
			if (entries[i].name.name != entry.name.name) continue;

			uintptr_t entryLoc = pe->pool + sizeof(entries[i]) * i;
			if (!bo4.WriteMemory(entryLoc, &entry, sizeof(entry))) {
				bo4.FreeMemory(allocated, allocatedSize);
				err = "Can't write data";
				return false;
			}

			// Free previous allocation for this slot
			if (state.currentPID == bo4.m_pid && state.currentData) {
				bo4.FreeMemory(state.currentData, state.currentDataSize);
			}
			state.currentPID      = bo4.m_pid;
			state.currentData     = allocated;
			state.currentDataSize = allocatedSize;
			return true;
		}

		bo4.FreeMemory(allocated, allocatedSize);
		err = "Can't find table in pool";
		return false;
	}

	// Combo helper -----------------------------------------------------------
	static void DrawSlotCombo(
		const char* label,
		const char** items,
		size_t       count,
		const char*& selected)
	{
		if (ImGui::BeginCombo(label, selected)) {
			for (size_t n = 0; n < count; n++) {
				bool isSelected = (selected == items[n]);
				if (ImGui::Selectable(items[n], isSelected))
					selected = items[n];
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}

	// Main NUI tool ----------------------------------------------------------
	void bo4_store_tool() {
		tool::nui::NuiUseDefaultWindow dw{};
		ImGui::SeparatorText("Store tool (Season 6 - Current Black Market)");

		// -- My Deals Slot 1 -------------------------------------------------
		ImGui::SeparatorText("My Deals - Slot 1 (Camos / Mastercrafts / Melee)");
		DrawSlotCombo("##slot1", slot1_items, ARRAYSIZE(slot1_items), g_slot1.selected);

		// -- My Deals Slot 2 -------------------------------------------------
		ImGui::SeparatorText("My Deals - Slot 2 (Outfits / Gestures / Jumppacks)");
		DrawSlotCombo("##slot2", slot2_items, ARRAYSIZE(slot2_items), g_slot2.selected);

		// -- Reserve Stack ---------------------------------------------------
		ImGui::SeparatorText("Reserve Stack (Bundles / Triple Plays / Starter Packs)");
		DrawSlotCombo("##reserve", reserve_items, ARRAYSIZE(reserve_items), g_reserve.selected);

		// -- Inject button ---------------------------------------------------
		if (ImGui::Button("Inject All")) {
			g_notif.clear();

			Process bo4 = L"BlackOps4.exe";
			if (!bo4 || !bo4.Open()) {
				g_notif = "Can't open BlackOps4.exe";
			}
			else {
				std::string err{};
				bool ok = true;

				struct {
					uint64_t    hash;
					const char* item;
					SlotState&  state;
					const char* label;
				} slots[] = {
					{ itemshop_slot_1, g_slot1.selected,   g_slot1,   "My Deals Slot 1" },
					{ itemshop_slot_2, g_slot2.selected,   g_slot2,   "My Deals Slot 2" },
					{ reserve_deals,  g_reserve.selected,  g_reserve, "Reserve Stack"   },
				};

				for (auto& s : slots) {
					if (!s.item || !*s.item) continue;
					LOG_TRACE("Injecting {} -> {}", s.label, s.item);
					if (!InjectStringTable(bo4, s.hash, s.item, s.state, err)) {
						g_notif = std::format("[{}] {}", s.label, err);
						ok = false;
						break;
					}
				}

				if (ok) g_notif = "Injected successfully!";
			}
		}

		// "Inject" individual slots side-by-side
		ImGui::SameLine();
		if (ImGui::Button("Inject Slot 1 Only")) {
			g_notif.clear();
			Process bo4 = L"BlackOps4.exe";
			if (!bo4 || !bo4.Open()) { g_notif = "Can't open BlackOps4.exe"; }
			else {
				std::string err{};
				if (!InjectStringTable(bo4, itemshop_slot_1, g_slot1.selected, g_slot1, err))
					g_notif = "[Slot 1] " + err;
				else
					g_notif = "Slot 1 injected!";
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Inject Slot 2 Only")) {
			g_notif.clear();
			Process bo4 = L"BlackOps4.exe";
			if (!bo4 || !bo4.Open()) { g_notif = "Can't open BlackOps4.exe"; }
			else {
				std::string err{};
				if (!InjectStringTable(bo4, itemshop_slot_2, g_slot2.selected, g_slot2, err))
					g_notif = "[Slot 2] " + err;
				else
					g_notif = "Slot 2 injected!";
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Inject Reserve Only")) {
			g_notif.clear();
			Process bo4 = L"BlackOps4.exe";
			if (!bo4 || !bo4.Open()) { g_notif = "Can't open BlackOps4.exe"; }
			else {
				std::string err{};
				if (!InjectStringTable(bo4, reserve_deals, g_reserve.selected, g_reserve, err))
					g_notif = "[Reserve] " + err;
				else
					g_notif = "Reserve injected!";
			}
		}

		// -- Status ----------------------------------------------------------
		if (!g_notif.empty()) {
			ImGui::Separator();
			ImGui::Text("%s", g_notif.data());
		}
	}

	ADD_TOOL_NUI(bo4_store_tool, "BO4 Store tool", bo4_store_tool);

} // namespace
