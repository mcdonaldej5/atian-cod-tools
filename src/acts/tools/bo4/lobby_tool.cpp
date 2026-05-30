#include <includes.hpp>
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>

namespace {

	// =========================================================================
	// CfgCbuf — installs a config rawfile into the game's RAWFILE pool and
	// queues an "exec" of it through the engine's command buffer.
	//
	// Mechanism:
	//   1. Allocate the cfg text as a string in the game process.
	//   2. Overwrite the default_systemlink.cfg RAWFILE asset's buffer/size
	//      so it points at our allocated text.
	//   3. Write "exec gamedata/configs/common/default_systemlink.cfg\n" into
	//      the CBuff (whose pointer lives at static offset 0xF99B168), so the
	//      engine runs our cfg next frame through the full DVAR system.
	//
	// Because the function reuses a single rawfile slot and frees the previous
	// allocation on each call, multiple DVAR commands must be batched into ONE
	// call separated by newlines (a single cfg can hold many commands).
	// =========================================================================
	void CfgCbuf(std::string&& cfg, std::string& logs) {
		Process bo4 = L"BlackOps4.exe";

		if (!bo4) {
			logs = "Can't find game";
			return;
		}

		if (!bo4.Open()) {
			logs = "Can't open game";
			return;
		}

		static DWORD oldPid{};
		static uintptr_t oldAlloc{};
		static size_t oldAllocSize;

		size_t sizeOut;
		uintptr_t alloc{ bo4.AllocateString(cfg.c_str(), &sizeOut)};

		if (!alloc) {
			logs = "Can't allocate string";
			return;
		}

		try {

			struct XAssetPoolEntry {
				uintptr_t pool;
				uint32_t itemSize;
				int32_t itemCount;
				byte isSingleton;
				int32_t itemAllocCount;
				uintptr_t freeHead;
			};
			struct RawFileEntry {
				uint64_t name; // 0x8
				uintptr_t pad0; // 0x10
				uintptr_t size; // 0x18
				uintptr_t buffer; // 0x20
			};


			auto pe{ bo4.ReadMemoryObjectEx<XAssetPoolEntry>(bo4[games::bo4::offset::assetPool] + games::bo4::pool::ASSET_TYPE_RAWFILE * sizeof(XAssetPoolEntry)) };

			RawFileEntry entry{};
			constexpr const char* hookCfgName = "gamedata/configs/common/default_systemlink.cfg";
			entry.name = hash::Hash64(hookCfgName);
			entry.buffer = alloc;
			entry.size = sizeOut;

			if (pe->itemSize != sizeof(entry)) {
				throw std::runtime_error("INVALID POOL SIZE");
			}

			auto entries{ bo4.ReadMemoryArrayEx<RawFileEntry>(pe->pool, pe->itemAllocCount) };

			bool patched{};
			for (size_t i = 0; i < pe->itemAllocCount; i++) {
				if (entries[i].name != entry.name) continue; // not ours
				uintptr_t entryLoc{ pe->pool + sizeof(entries[i]) * i };
				if (!bo4.WriteMemory(entryLoc, &entry, sizeof(entry))) {
					throw std::runtime_error("Can't write data");
				}
				patched = true;
				break;
			}

			if (!patched) {
				throw std::runtime_error("Can't find default_systemlink.cfg");
			}

			// remove previous data
			if (bo4.m_pid == oldPid && oldAlloc) {
				bo4.FreeMemory(oldAlloc, oldAllocSize);
				oldAlloc = 0;
			}

			oldPid = bo4.m_pid;
			oldAlloc = alloc;
			oldAllocSize = sizeOut;

			struct CBuff {
				uintptr_t buffer; // const char* 
				int32_t allocated;
				int32_t used;
			};

			auto buff{ bo4.ReadMemoryObjectEx<CBuff>(bo4[0xF99B168]) };

			const char* cmd{ utils::va("exec %s\n", hookCfgName) };
			size_t len{ std::strlen(cmd) };
			bo4.WriteMemory(buff->buffer + buff->used, cmd, len + 1);
			buff->used += (int32_t)len;
			if (!bo4.WriteMemory(bo4[0xF99B168], buff.get(), sizeof(*buff))) {
				throw std::runtime_error("Can't write cbuf");
			}
			logs = "Injected";
		}
		catch (std::runtime_error& re) {
			if (alloc) {
				bo4.FreeMemory(alloc, sizeOut);
			}
			logs = re.what();
		}
	}

	// =========================================================================
	// NUI tool
	// =========================================================================
	void bo4_lobby_tool() {
		tool::nui::NuiUseDefaultWindow dw{};
		ImGui::SeparatorText("BO4 Lobby tool");

		// --- XP & Stat Tracking ---------------------------------------------
		ImGui::SeparatorText("XP & Stat Tracking");

		ImGui::TextDisabled("Applies the settings needed for a custom game to earn");
		ImGui::TextDisabled("XP, weapon XP, merits, tiers, Black Market progress and");
		ImGui::TextDisabled("challenge tracking. Apply AFTER the map has loaded,");
		ImGui::TextDisabled("not from the lobby. Re-apply on each map load.");
		ImGui::Spacing();

		static std::string xpLogs{};

		if (ImGui::Button("Make Custom Game Earn XP & Track All Stats")) {
			// All four DVAR sets batched into ONE cfg so the single rawfile/exec
			// round-trip carries every command (CfgCbuf reuses one rawfile slot).
			CfgCbuf(
				"set SESSIONMODE_BASE_DVAR online\n"
				"set dohackedstats 0\n"
				"set unlockXp 1\n"
				"set scr_disablestattracking 0\n",
				xpLogs
			);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::TextUnformatted("Injects four DVAR commands via the game's Cbuf:");
			ImGui::TextUnformatted("  SESSIONMODE_BASE_DVAR online -> session counts as online");
			ImGui::TextUnformatted("  dohackedstats 0              -> real stat persistence");
			ImGui::TextUnformatted("  unlockXp 1                   -> level.rankedmatch = 1");
			ImGui::TextUnformatted("  scr_disablestattracking 0    -> increments write through");
			ImGui::EndTooltip();
		}

		ImGui::SameLine();

		if (ImGui::Button("Revert XP Patch")) {
			CfgCbuf(
				"set SESSIONMODE_BASE_DVAR private_online\n"
				"set dohackedstats 1\n"
				"set unlockXp 0\n",
				xpLogs
			);
		}

		if (!xpLogs.empty()) {
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::TextWrapped("%s", xpLogs.c_str());
		}
	}

	ADD_TOOL_NUI(bo4_lobby_tool, "BO4 Lobby tool", bo4_lobby_tool);

}
