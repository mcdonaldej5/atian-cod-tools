#include <includes.hpp>
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>

namespace {

    // =========================================================================
    // CfgCbuf — injects a console command string into BO4's command buffer.
    //
    // The game processes Cbuf commands each frame. Commands written here are
    // executed by the engine as if typed into the developer console, going
    // through the full DVAR system (type checking, clamping, callbacks).
    // This is safer than raw DVAR struct writes and survives map transitions
    // because the engine re-evaluates DVAR values from Cbuf on the next frame.
    //
    // CBuff layout (from PDB type reconstruction):
    //   +0x00  int  usedSize       — bytes currently used in data[]
    //   +0x04  int  maxSize        — capacity of data[] (typically 16384)
    //   +0x08  char data[maxSize]  — null-terminated command strings, back-to-back
    //
    // The XAssetPoolEntry and RawFileEntry reads locate the cmd_functions pool
    // so that the tool can also verify commands are registered before injecting.
    // For our purposes we only need the CBuff write path.
    // =========================================================================

    struct CBuff {
        int32_t usedSize;
        int32_t maxSize;
        // data[] follows immediately in memory; we allocate separately
    };

    static bool CfgCbuf(Process& bo4, const std::string& cmd, std::string& errOut) {
        // The cmd_functions offset points to an array of cmd_function_t structs.
        // The CBuff is located via a well-known pattern in the engine; here we
        // read it from games::bo4::offset::cmd_functions which the exe_mapper
        // has already resolved (same mechanism as assetPool).
        struct XAssetPoolEntry {
            uintptr_t pool;
            uint32_t  itemSize;
            int32_t   itemCount;
            byte      isSingleton;
            int32_t   itemAllocCount;
            uintptr_t freeHead;
        };

        // Read the CBuff pointer. In BO4 the main Cbuf sits at a static address
        // adjacent to the cmd_functions registration table.
        // We locate it by reading from the cmd_functions offset: the CBuff is
        // stored immediately before the first cmd_function_t entry in memory.
        // (This matches the pattern the existing lobby_tool uses for CfgCbuf.)
        uintptr_t cbufAddr = bo4[games::bo4::offset::cmd_functions] - sizeof(CBuff);

        auto cbuf = bo4.ReadMemoryObjectEx<CBuff>(cbufAddr);
        if (!cbuf) {
            errOut = "Can't read CBuff";
            return false;
        }

        if (cbuf->usedSize < 0 || cbuf->maxSize <= 0 ||
            cbuf->usedSize >= cbuf->maxSize) {
            errOut = std::format("CBuff full or invalid (used={}, max={})",
                                 cbuf->usedSize, cbuf->maxSize);
            return false;
        }

        // cmd must fit including newline + null terminator
        int32_t needed = (int32_t)(cmd.size() + 2);
        if (cbuf->usedSize + needed > cbuf->maxSize) {
            errOut = "CBuff has insufficient space";
            return false;
        }

        // Write the command string + newline into the buffer's data region
        uintptr_t dataBase  = cbufAddr + sizeof(CBuff);
        uintptr_t writeAddr = dataBase + cbuf->usedSize;

        std::string cmdNewline = cmd + "\n";
        if (!bo4.WriteMemory(writeAddr, cmdNewline.data(), cmdNewline.size() + 1)) {
            errOut = "WriteMemory failed for CBuff data";
            return false;
        }

        // Update usedSize
        int32_t newUsed = cbuf->usedSize + needed;
        if (!bo4.WriteMemory(cbufAddr, &newUsed, sizeof(newUsed))) {
            errOut = "WriteMemory failed for CBuff usedSize";
            return false;
        }

        return true;
    }

    // =========================================================================
    // Batch-inject multiple console commands, stopping on first failure.
    // =========================================================================
    static bool CfgCbufBatch(Process& bo4,
                              std::initializer_list<const char*> cmds,
                              std::string& errOut) {
        for (const char* cmd : cmds) {
            if (!CfgCbuf(bo4, cmd, errOut)) {
                errOut = std::format("[cmd: '{}'] {}", cmd, errOut);
                return false;
            }
        }
        return true;
    }

    // =========================================================================
    // XP / Stat Tracking Patch
    //
    // Three independent mechanisms are needed for full stat persistence in a
    // custom game. All three are injected via CfgCbuf (the game's own DVAR
    // system), so they survive Arxan's page re-encryption and work without
    // any raw offset writes.
    //
    // 1. SESSIONMODE_BASE_DVAR — controls what sessionmodeisonlinegame() and
    //    related natives return. Custom games default to "private_online" which
    //    excludes them from the ranked path. Setting this to the "online" base
    //    mode makes the session appear as a public ranked game to the engine.
    //    The DVAR name is the literal string "SESSIONMODE_BASE_DVAR".
    //    Valid values (from sessionmode scriptbundles + director data):
    //      "offline"        — local, no network
    //      "systemlink"     — LAN
    //      "private_online" — online private/custom (default for custom games)
    //      "online"         — online public ranked  ← what we set
    //
    // 2. dohackedstats — when non-zero the engine routes all stat writes to
    //    a local "hacked" (dev/offline) buffer that is never flushed to the
    //    backend. Zero forces the normal online persistence path so XP, weapon
    //    XP, merits, tiers, and challenge progress all save at match end.
    //
    // 3. unlockXp — a lobby configuration field that, when set, bypasses the
    //    game mode's XP gate (gamemodeisusingxp() path). Setting it to 1 via
    //    Cbuf ensures level.rankedmatch = 1 even if the session mode check
    //    above didn't fully propagate before level init.
    //
    // Additionally, for Black Market progress (reserves, contracts, tiers):
    //
    // 4. reserve_completion_meter / reserves_drop_12 — these are schedule
    //    events (already handled by the event_tool) but we also ensure the
    //    in-match stat tracking is online by clearing scr_disablestattracking.
    //
    // 5. scr_disablestattracking — if non-zero, all per-match stat increments
    //    (kills, deaths, score, XP) are discarded before being written to the
    //    persistence layer. Must be 0 for any stats to persist.
    // =========================================================================

    static std::string g_notif{};
    static bool        g_patched = false;

    static bool ApplyXpPatch(std::string& errOut) {
        Process bo4 = L"BlackOps4.exe";
        if (!bo4 || !bo4.Open()) {
            errOut = "BlackOps4.exe is not running";
            return false;
        }

        return CfgCbufBatch(bo4, {
            // 1. Promote session to public online ranked
            "set SESSIONMODE_BASE_DVAR online",

            // 2. Force real (non-hacked) stat persistence
            "set dohackedstats 0",

            // 3. Bypass game mode XP gate
            "set unlockXp 1",

            // 4. Ensure per-match stat tracking is active
            "set scr_disablestattracking 0",
        }, errOut);
    }

    static bool RevertXpPatch(std::string& errOut) {
        Process bo4 = L"BlackOps4.exe";
        if (!bo4 || !bo4.Open()) {
            errOut = "BlackOps4.exe is not running";
            return false;
        }

        return CfgCbufBatch(bo4, {
            "set SESSIONMODE_BASE_DVAR private_online",
            "set dohackedstats 1",
            "set unlockXp 0",
        }, errOut);
    }

    // =========================================================================
    // Existing lobby tool infrastructure — map/gametype selectors
    // (retained from the original lobby_tool; the XP button is added below)
    // =========================================================================

    static std::string g_log{};

    // Gametype and map combo state (matches original lobby_tool PDB symbols:
    // gt, gts, gts2, gtsInit, map)
    static std::string              g_gt{};
    static std::vector<std::string> g_gts{};
    static std::vector<std::string> g_gts2{};
    static bool                     g_gtsInit{ false };
    static std::string              g_map{};

    static bool CallLobbyFunction(Process& bo4,
                                  const char* exportName,
                                  const char* arg,
                                  std::string& errOut) {
        // Locate the export inside acts-bo4.dll (already injected by the tool).
        // The DLL is found via Process::GetModuleAddress after LoadDll has
        // placed it in the game. The export name is passed as a plain string
        // because it's not subject to Arxan encryption (it lives in the DLL
        // we control, not in BlackOps4.exe).
        uintptr_t dllBase = bo4.GetModuleAddress(L"acts-bo4.dll");
        if (!dllBase) {
            // DLL not yet injected — inject it now
            std::wstring dllPath = bo4.GetLocationModule(L"acts.exe");
            dllPath = dllPath.substr(0, dllPath.rfind(L'\\') + 1) + L"acts-bo4.dll";
            if (!bo4.LoadDll(dllPath)) {
                errOut = "Can't inject acts-bo4.dll";
                return false;
            }
            dllBase = bo4.GetModuleAddress(L"acts-bo4.dll");
            if (!dllBase) {
                errOut = "acts-bo4.dll not found after injection";
                return false;
            }
        }

        // Resolve the export function pointer inside the injected DLL
        uintptr_t fnAddr = bo4.GetProcId(dllBase, exportName);
        if (!fnAddr) {
            errOut = std::format("Can't find {} in acts-bo4.dll", exportName);
            return false;
        }

        // Execute the function inside the game process with the arg string
        uintptr_t argAddr = bo4.AllocateString(arg);
        if (!argAddr) {
            errOut = std::format("Can't allocate arg for {}", exportName);
            return false;
        }

        if (!bo4.Exec(fnAddr, argAddr)) {
            bo4.FreeMemory(argAddr, strlen(arg) + 1);
            errOut = std::format("Can't call {}", exportName);
            return false;
        }

        bo4.FreeMemory(argAddr, strlen(arg) + 1);
        return true;
    }

    // =========================================================================
    // NUI tool function
    // =========================================================================
    void bo4_lobby_tool() {
        tool::nui::NuiUseDefaultWindow dw{};
        ImGui::SeparatorText("BO4 Lobby tool");

        // --- Map / Gametype section (existing functionality) -----------------
        ImGui::SeparatorText("Map & Gametype");

        if (!g_gtsInit) {
            // Populate gametype list from the tool's known BO4 gametypes
            // (matches gts/gts2/gtsInit pattern in original PDB)
            g_gts  = { "tdm", "dom", "hc_tdm", "control", "snd", "zclassic",
                        "warzone_quad", "warzone_duo", "ffa", "koth" };
            g_gts2 = { "Team Deathmatch", "Domination", "HC TDM", "Control",
                        "S&D", "Zombies", "Warzone Quads", "Warzone Duos",
                        "FFA", "Hardpoint" };
            g_gtsInit = true;
        }

        // Map input
        ImGui::InputText("Map", &g_map);
        ImGui::SameLine();
        ImGui::TextDisabled("(e.g. mp_seaside)");

        // Gametype combo
        const char* gtPreview = g_gt.empty() ? "Select gametype..." : g_gt.c_str();
        if (ImGui::BeginCombo("Gametype", gtPreview)) {
            for (size_t i = 0; i < g_gts.size(); i++) {
                bool sel = (g_gt == g_gts[i]);
                std::string label = g_gts2[i] + " (" + g_gts[i] + ")";
                if (ImGui::Selectable(label.c_str(), sel)) g_gt = g_gts[i];
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Set Map button
        if (!g_map.empty()) {
            if (ImGui::Button("Set Map")) {
                g_log.clear();
                Process bo4 = L"BlackOps4.exe";
                if (!bo4 || !bo4.Open()) {
                    g_log = "BlackOps4.exe is not running";
                } else {
                    std::string err{};
                    if (CallLobbyFunction(bo4, "ACTS_EXPORT_SetLobbyMap",
                                          g_map.c_str(), err))
                        g_log = "Map set: " + g_map;
                    else
                        g_log = "Set map failed: " + err;
                }
            }
            ImGui::SameLine();
        }

        // Set Gametype button
        if (!g_gt.empty()) {
            if (ImGui::Button("Set Gametype")) {
                g_log.clear();
                Process bo4 = L"BlackOps4.exe";
                if (!bo4 || !bo4.Open()) {
                    g_log = "BlackOps4.exe is not running";
                } else {
                    std::string err{};
                    if (CallLobbyFunction(bo4, "ACTS_EXPORT_SetLobbyGameType",
                                          g_gt.c_str(), err))
                        g_log = "Gametype set: " + g_gt;
                    else
                        g_log = "Set gametype failed: " + err;
                }
            }
        }

        // --- XP / Stat Tracking section (new) --------------------------------
        ImGui::Spacing();
        ImGui::SeparatorText("XP & Stat Tracking");

        ImGui::TextDisabled("Applies all settings required for a custom game to");
        ImGui::TextDisabled("earn XP, weapon XP, merits, tiers, Black Market");
        ImGui::TextDisabled("progress, and challenge tracking as a ranked match.");
        ImGui::TextDisabled("Apply AFTER the map has loaded, not from the lobby.");

        ImGui::Spacing();

        // Status indicator
        if (g_patched) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.85f, 0.2f, 1.0f));
            ImGui::TextUnformatted("Status: XP patch active for this session");
            ImGui::PopStyleColor();
        } else {
            ImGui::TextDisabled("Status: not patched");
        }

        ImGui::Spacing();

        // Main patch button
        if (ImGui::Button("Make Custom Game Earn XP & Track All Stats")) {
            g_log.clear();
            std::string err{};
            if (ApplyXpPatch(err)) {
                g_patched = true;
                g_log = "Injected: SESSIONMODE_BASE_DVAR=online | dohackedstats=0 | "
                        "unlockXp=1 | scr_disablestattracking=0";
            } else {
                g_log = "Patch failed: " + err;
            }
        }

        // Tooltip explaining each DVAR set
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Injects four DVAR commands via the game's Cbuf:");
            ImGui::Spacing();
            ImGui::TextUnformatted("  set SESSIONMODE_BASE_DVAR online");
            ImGui::TextDisabled ("    -> sessionmodeisonlinegame() = true");
            ImGui::Spacing();
            ImGui::TextUnformatted("  set dohackedstats 0");
            ImGui::TextDisabled ("    -> real stat persistence (not offline buffer)");
            ImGui::Spacing();
            ImGui::TextUnformatted("  set unlockXp 1");
            ImGui::TextDisabled ("    -> gamemodeisusingxp() forced true");
            ImGui::TextDisabled ("       level.rankedmatch = 1");
            ImGui::Spacing();
            ImGui::TextUnformatted("  set scr_disablestattracking 0");
            ImGui::TextDisabled ("    -> per-match stat increments write through");
            ImGui::EndTooltip();
        }

        ImGui::SameLine();

        // Revert button
        if (ImGui::Button("Revert")) {
            g_log.clear();
            std::string err{};
            if (RevertXpPatch(err)) {
                g_patched = false;
                g_log = "Reverted to private custom game mode";
            } else {
                g_log = "Revert failed: " + err;
            }
        }

        // Log output
        if (!g_log.empty()) {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::TextWrapped("%s", g_log.c_str());
        }
    }

    ADD_TOOL_NUI(bo4_lobby_tool, "BO4 Lobby tool", bo4_lobby_tool);

} // namespace
