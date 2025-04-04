#include <includes.hpp>
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>

namespace {
	// string,int,int,string
	constexpr uint64_t itemshop_slot_1 = hash::Hash64("gamedata/events/itemshop_slot_1_pc.csv");
	constexpr uint64_t start = 1545156000;
	constexpr uint64_t end = 2147367600;

	void bo4_store_tool() {
		tool::nui::NuiUseDefaultWindow dw{};
		ImGui::SeparatorText("Store tool");

		static const char* slot1s[]{
			"camo_active_dlc1_ww2_wrapper",
			"camo_active_dlc1_masks_wrapper",
			"camo_dlc2_benjamins",
			"camo_dlc2_smooch",
			"weaponcamo_rocketbox_bundle",
			"mastercraft_vampire_hunter",
			"bacon_weapon_camo_bundle",
		};
		static const char* slot2s[]{
			"camo_active_dlc1_ww2_wrapper",
			"camo_active_dlc1_masks_wrapper",
			"camo_dlc2_benjamins",
			"camo_dlc2_smooch",
			"weaponcamo_rocketbox_bundle",
			"mastercraft_vampire_hunter",
			"bacon_weapon_camo_bundle",
		};
		
		
		static const char* slot1{ slot1s[ARRAYSIZE(slot1s) - 1] };
		static const char* slot2{ slot2s[0] };
		static uintptr_t currentData{};
		static size_t currentDataSize{};
		static DWORD currentPID{};
		static std::string notif{};

		static std::once_flag of{};

		
		ImGui::SeparatorText("Store Slot 1");

		if (ImGui::BeginCombo("##slot1", slot1)) {
			for (int n = 0; n < ARRAYSIZE(slot1s); n++)
			{
				bool isSelected = (slot1 == slot1s[n]);
				if (ImGui::Selectable(slot1s[n], isSelected)) {
					slot1 = slot1s[n];
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::SeparatorText("Store Slot 2");

		if (ImGui::BeginCombo("##slot2", slot2)) {
			for (int n = 0; n < ARRAYSIZE(slot2s); n++)
			{
				bool isSelected = (slot2 == slot2s[n]);
				if (ImGui::Selectable(slot2s[n], isSelected)) {
					slot2 = slot2s[n];
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Inject")) {
			std::vector<const char*> store{};

			if (*slot1) {
				store.push_back(slot1);
			}
			if (*slot2) {
				store.push_back(slot2);
			}

			LOG_TRACE("Injecting({})", store.size());
			for (const char* ev : store) {
				LOG_TRACE("{}", ev);
			}


			Process bo4 = L"BlackOps4.exe";
			uintptr_t allocated{};
			size_t allocatedSize{};
			try {

				if (!bo4 || !bo4.Open()) {
					throw std::runtime_error("Can't open bo4");
				}

				std::vector<byte> rfile{};
				std::vector<size_t> storeStrings{};

				// index 0 is all for event type
				size_t allLoc{ utils::WriteString(rfile, "all") };

				enum StringTableCellType : int32_t {
					STC_TYPE_UNDEFINED = 0,
					STC_TYPE_STRING = 1,
					STC_TYPE_HASHED = 2,
					STC_TYPE_INT = 4,
					STC_TYPE_FLOAT = 5,
					STC_TYPE_BOOL = 6
				};

				struct StringTableCell {
					byte value[20] = {};
					StringTableCellType type{};
				};

				struct StringTableEntry {
					XHash name{};
					int32_t columnCount{};
					int32_t rowCount{};
					int32_t cellscount{};
					int32_t unk24{};
					uintptr_t cells{};
					uintptr_t values{}; // StringTableCell*
					uintptr_t unk48{};
					uintptr_t unk56{};
				}; static_assert(sizeof(StringTableEntry) == 0x40);

				size_t cells{ utils::Allocate(rfile, sizeof(StringTableCell) * 3 * store.size()) };

				for (size_t i = 0; i < store.size(); i++) {
					size_t evLoc{ utils::WriteString(rfile, store[i]) };
					StringTableCell* row{ &reinterpret_cast<StringTableCell*>(&rfile[cells])[i * 3] };

					row[0].type = STC_TYPE_STRING;
					*(size_t*)&(row[0].value) = evLoc;
					row[1].type = STC_TYPE_INT;
					*(int64_t*)&(row[1].value) = start;
					row[2].type = STC_TYPE_INT;
					*(int64_t*)&(row[2].value) = end;
				}

				StringTableEntry entry{};
				entry.name.name = itemshop_slot_1;
				entry.columnCount = 3;
				entry.rowCount = (int32_t)store.size();

				allocatedSize = rfile.size();
				allocated = bo4.AllocateMemory(allocatedSize);

				if (!allocated) {
					throw std::runtime_error(std::format("Can't allocate {} bytes", rfile.size()));
				}

				// link strings
				entry.values = allocated + cells;
				StringTableCell* row{ reinterpret_cast<StringTableCell*>(&rfile[cells]) };
				for (size_t i = 0; i < store.size(); i++) {
					*(uintptr_t*)row[i * 3].value += allocated;
					*(uintptr_t*)row[i * 3 + 2].value += allocated;
				}

				// write data
				if (!bo4.WriteMemory(allocated, rfile.data(), allocatedSize)) {
					throw std::runtime_error(std::format("Can't write {} bytes", rfile.size()));
				}

				struct XAssetPoolEntry {
					uintptr_t pool;
					uint32_t itemSize;
					int32_t itemCount;
					byte isSingleton;
					int32_t itemAllocCount;
					uintptr_t freeHead;
				};


				auto pe{ bo4.ReadMemoryObjectEx<XAssetPoolEntry>(bo4[games::bo4::offset::assetPool] + games::bo4::pool::ASSET_TYPE_STRINGTABLE * sizeof(XAssetPoolEntry))};

				if (pe->itemSize != sizeof(entry)) {
					throw std::runtime_error("INVALID POOL SIZE");
				}

				auto entries{ bo4.ReadMemoryArrayEx<StringTableEntry>(pe->pool, pe->itemAllocCount) };

				bool patched{};
				for (size_t i = 0; i < pe->itemAllocCount; i++) {
					if (entries[i].name.name != entry.name.name) continue; // not ours
					uintptr_t entryLoc{ pe->pool + sizeof(entries[i]) * i };
					if (!bo4.WriteMemory(entryLoc, &entry, sizeof(entry))) {
						throw std::runtime_error("Can't write data");
					}
					notif = "Injected";
					patched = true;
					break;
				}

				if (!patched) {
					throw std::runtime_error("Can't find itemshop_slot_1_pc.csv");
				}

				// remove previous data
				if (currentPID == bo4.m_pid) {
					bo4.FreeMemory(currentData, currentDataSize);
				}
				currentPID = bo4.m_pid;
				currentData = allocated;
				currentDataSize = rfile.size();
			}
			catch (std::runtime_error& re) {
				if (allocated) {
					bo4.FreeMemory(allocated, allocatedSize);
				}
				notif = re.what();
			}
		}


		if (!notif.empty()) {
			ImGui::Separator();

			ImGui::Text("%s", notif.data());
		}
		
	}

	ADD_TOOL_NUI(bo4_store_tool, "BO4 Store tool", bo4_store_tool);

}