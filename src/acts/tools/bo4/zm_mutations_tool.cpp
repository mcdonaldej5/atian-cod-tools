#include <includes.hpp>
#ifdef _WIN32
#include "tools/tools_nui.hpp"
#include <core/config.hpp>
#include <tools/pool.hpp>
#include <games/bo4/pool.hpp>
#include <games/bo4/offsets.hpp>
#include <utils/memapi_calls.hpp>

// ============================================================================
//  BO4 Zombies Custom Mutations Tool
//
//  Adapted from the community BO4 lobby tool. All Multiplayer and Blackout
//  references removed. Populated with the full set of 221 zombie custom-game
//  ("custom mutations") settings, extracted from the verified defaults table in
//  zm_customgame.gsc. Each setting renders its name and a dropdown of its valid
//  values; defaults are pre-selected so injecting with nothing changed leaves
//  the game playing normally. One Inject button at the bottom writes all
//  settings via the same gts-config / cbuf-exec mechanism as the original tool.
// ============================================================================

namespace {

// AUTO-GENERATED: names + options sourced from the Edit Game Rules
// gamesetting bundles (scriptbundle/gamesettings/*.json). Defaults and
// categories from zm_customgame.gsc. Option labels resolved where the
// label hash was identifiable (Off/On/Only/Small/Medium/Large); numeric
// settings show their value. 221 settings total.
struct ZmOpt { int value; const char* label; };
struct ZmSetting { const char* category; const char* name; const char* label; int def; std::vector<ZmOpt> opts; };
static const std::vector<ZmSetting> zmSettings = {
    { "General", "startround", "Starting Round", 1, { { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 29, "29" }, { 31, "31" }, { 33, "33" }, { 35, "35" }, { 36, "36" }, { 40, "40" } } },
    { "General", "headshotsonly", "Headshots Only", 0, { { 0, "Off" }, { 1, "On" } } },
    { "General", "zmroundcap", "Round Cap", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "General", "zmtimecap", "Time Cap", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "General", "zmshowtimer", "Show Timer", 0, { { 0, "Off" }, { 1, "On" } } },
    { "General", "zmkillcap", "Kill Cap", 0, { { 0, "Off" }, { 50, "50" }, { 100, "100" }, { 150, "150" }, { 200, "200" }, { 250, "250" }, { 300, "300" }, { 350, "350" }, { 400, "400" }, { 450, "450" }, { 500, "500" }, { 550, "550" }, { 600, "600" }, { 650, "650" }, { 700, "700" }, { 750, "750" }, { 800, "800" }, { 850, "850" }, { 900, "900" }, { 950, "950" }, { 1000, "1000" }, { 1050, "1050" }, { 1100, "1100" }, { 1150, "1150" }, { 1200, "1200" }, { 1250, "1250" }, { 1300, "1300" }, { 1350, "1350" }, { 1400, "1400" }, { 1500, "1500" }, { 1600, "1600" }, { 1700, "1700" }, { 1750, "1750" }, { 1950, "1950" } } },
    { "General", "zmspecialroundsenabled", "Specialrounds", 1, { { 0, "Off" }, { 1, "On" } } },
    { "General", "zmendonquest", "End On Quest", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmrunnerstate", "Runnerstate", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmwalkerstate", "Walkerstate", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmcrawlerstate", "Crawlers", 1, { { 0, "Off" }, { 1, "On" }, { 2, "Only" } } },
    { "Enemies", "zmzombiespread", "Zombie Spread", 1, { { 0, "0" }, { 1, "Medium" }, { 2, "2" } } },
    { "Enemies", "zmzombieminspeed", "Zombiespeedmin", 0, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
    { "Enemies", "zmzombiemaxspeed", "Zombiespeedmax", 3, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
    { "Enemies", "zmzombiedamagemult", "Zombie Damage", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmzombiehealthmult", "Zombie Health", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmcrawlerdamagemult", "Crawlerdamagemult", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmcrawlerhealthmult", "Crawlerhealthmult", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmcrawleraggro", "Crawleraggro", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmheavystate", "Heavy", 1, { { 0, "Off" }, { 1, "On" }, { 2, "Only" } } },
    { "Enemies", "zmheavydamagemult", "Heavydamagemult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmheavyhealthmult", "Heavyhealthmult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmheavyaggro", "Heavyaggro", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmminibossstate", "Mini Boss", 1, { { 0, "Off" }, { 1, "On" }, { 2, "Only" } } },
    { "Enemies", "zmminibossdamagemult", "Minibossdamagemult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmminibosshealthmult", "Minibosshealthmult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmminibossaggro", "Minibossaggro", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Enemies", "zmheavyspawnfreq", "Heavyfrequency", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
    { "Enemies", "zmminibossspawnfreq", "Minibossfrequency", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
    { "Enemies", "zmpopcornstate", "Vermin", 1, { { 0, "Off" }, { 1, "On" }, { 2, "Only" } } },
    { "Enemies", "zmpopcorndamagemult", "Popcorndamagemult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmpopcornhealthmult", "Popcornhealthmult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmpopcornspawnfreq", "Popcornspawnfreq", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
    { "Enemies", "zmenhancedstate", "Enhanced", 1, { { 0, "Off" }, { 1, "On" }, { 2, "Only" } } },
    { "Enemies", "zmenhanceddamagemult", "Enhanceddamagemult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmenhancedhealthmult", "Enhancedhealthmult", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Enemies", "zmenhancedspawnfreq", "Enhancedfrequency", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
    { "Health", "zmhealthstartingbars", "Healthstarting", 3, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" } } },
    { "Health", "zmhealthregendelay", "Health Regen Delay", 1, { { 0, "0" }, { 1, "Medium" }, { 2, "2" } } },
    { "Health", "zmhealthregenrate", "Health Regen Rate", 2, { { 0, "0" }, { 1, "1" }, { 2, "Medium" }, { 3, "3" }, { 4, "Large" } } },
    { "Health", "zmhealthonkill", "Kills Give Health", 0, { { 0, "Off" }, { 1, "Small" }, { 2, "Medium" }, { 3, "3" } } },
    { "Health", "zmhealthdrain", "Healthdrain", 0, { { 0, "Off" }, { 1, "Small" }, { 2, "Medium" }, { 3, "3" } } },
    { "Health", "zmlaststandduration", "Laststandduration", 2, { { 0, "Off" }, { 1, "1" }, { 2, "Medium" }, { 3, "3" } } },
    { "Health", "zmlimiteddownsamount", "Laststandcount", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "Health", "zmbarricadestate", "Barricadestate", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Health", "zmpointlossondown", "Pointlossondown", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "Points", "zmpointsfixed", "Pointsfixed", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Points", "zmpointsstarting", "Pointsstarting", 5, { { 0, "0" }, { 5, "5" }, { 10, "10" }, { 15, "15" }, { 20, "20" }, { 25, "25" }, { 30, "30" }, { 35, "35" }, { 40, "40" }, { 45, "45" }, { 50, "50" }, { 55, "55" }, { 60, "60" }, { 65, "65" }, { 70, "70" }, { 75, "75" }, { 80, "80" }, { 85, "85" }, { 90, "90" }, { 95, "95" }, { 100, "100" }, { 110, "110" }, { 120, "120" }, { 130, "130" }, { 140, "140" }, { 150, "150" }, { 160, "160" }, { 170, "170" }, { 180, "180" }, { 200, "200" }, { 250, "250" }, { 300, "300" }, { 325, "325" }, { 425, "425" } } },
    { "Points", "zmpointslosstype", "Pointslosstype", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" } } },
    { "Points", "zmpointslosspercent", "Pointslosspercent", 1, { { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 29, "29" }, { 31, "31" }, { 33, "33" }, { 35, "35" }, { 36, "36" }, { 40, "40" } } },
    { "Points", "zmpointslossvalue", "Pointslossvalue", 100, { { 100, "100" }, { 200, "200" }, { 300, "300" }, { 400, "400" }, { 500, "500" }, { 600, "600" }, { 700, "700" }, { 800, "800" }, { 900, "900" }, { 1000, "1000" }, { 1100, "1100" }, { 1200, "1200" }, { 1300, "1300" }, { 1400, "1400" }, { 1500, "1500" }, { 1600, "1600" }, { 1700, "1700" }, { 1800, "1800" }, { 1900, "1900" }, { 2000, "2000" }, { 2100, "2100" }, { 2200, "2200" }, { 2300, "2300" }, { 2400, "2400" }, { 2500, "2500" }, { 2600, "2600" }, { 2700, "2700" }, { 2800, "2800" }, { 2900, "2900" }, { 3100, "3100" }, { 3300, "3300" }, { 3500, "3500" }, { 3600, "3600" }, { 4000, "4000" } } },
    { "Points", "zmpointlossondeath", "Pointlossondeath", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "Points", "zmpointlossonteammatedeath", "Pointlossonteammatedeath", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "Perks", "zmperksactive", "Perksactive", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksjuggernaut", "Perksjuggernaut", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksspeed", "Perksspeed", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksquickrevive", "Perksrevive", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperkswidowswail", "Perkswidow", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksstaminup", "Perksstaminup", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksmulekick", "Perksmulekick", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperkselectricburst", "Perkselectricburst", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksdeadshot", "Perksdeadshot", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperkscooldown", "Perkscooldown", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksdyingwish", "Perksdyingwish", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperkdecay", "Perkdecay", 1, { { 1, "1" }, { 2, "2" } } },
    { "Perks", "zmperksbandolier", "Perksbandolier", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksdeathperception", "Perksdeathperception", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksphdslider", "Perksphdslider", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperkssecretsauce", "Perkssecretsauce", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksstonecold", "Perksstonecold", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperkswolfprotector", "Perkswolfprotector", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksdeathdash", "Perksdeathdash", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Perks", "zmperksvictorious", "Perksvictorious", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupsactive", "Powerups", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupnuke", "Powerupsnuke", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupdouble", "Powerups2X", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupinstakill", "Powerupsinsta", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupchaospoints", "Powerupschaos", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupfiresale", "Powerupsfiresale", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupspecialweapon", "Powerupsspecweap", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupfreeperk", "Powerupfreeperk", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupmaxammo", "Powerupsmaxammo", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupcarpenter", "Powerupscarpenter", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupsislimitedround", "Powerupsislimited", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupslimitround", "Poweruplimit", 1, { { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" } } },
    { "Power-Ups", "zmpowerupsharing", "Powerupsharing", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Power-Ups", "zmpowerupfrequency", "Powerupfrequency", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Weapons", "zmpapenabled", "Pack", 1, { { 0, "Off" }, { 1, "1" }, { 2, "2" } } },
    { "Weapons", "zmsuperpapenabled", "Superpack", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponspistol", "Weaponspistol", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponsshotgun", "Weapshotgun", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponssmg", "Weapsmg", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponsar", "Weapar", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponstr", "Weaptr", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponslmg", "Weaplmg", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponssniper", "Weapsniper", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponsknife", "Weapknife", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmweaponsmelee", "Weapmelee", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmspecweaponisenabled", "Specweap", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmspecweaponchargerate", "Specweapcharge", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Weapons", "zmwonderweaponisenabled", "Wonderweap", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmequipmentisenabled", "Equipment", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmequipmentchargerate", "Equipcharge", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Weapons", "zmshieldisenabled", "Shield", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Weapons", "zmshielddurability", "Shielddurability", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Weapons", "zmstartingweaponenabled", "Allowstartingweapon", 1, { { 0, "0" }, { 1, "On" } } },
    { "Weapons", "zmretainweapons", "Retainweapons", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Mystery Box", "zmmysteryboxstate", "Mystery Box", 2, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
    { "Mystery Box", "zmmysteryboxlimitmove", "Mysteryboxlimitmove", 0, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" } } },
    { "Mystery Box", "zmmysteryboxlimit", "Mysteryboxlimit", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "Mystery Box", "zmmysteryboxlimitround", "Mysteryboxlimitrnd", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" } } },
    { "Elixirs", "zmelixirsenabled", "Elixirs", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirscooldown", "Elixirscooldown", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Elixirs", "zmelixirsindividual", "Elixirsindividual", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirsdurables", "Elixirsdurables", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirscommon", "Elixirscommon", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirsrare", "Elixirsrare", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirslegendary", "Elixirslegendary", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirsepic", "Elixirsepic", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixiralwaysdoneswiftly", "Elixiralwaysdoneswiftly", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixiranywherebuthere", "Elixiranywherebuthere", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirarsenalaccelerator", "Elixirarsenalaccelerator", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirdangerclosest", "Elixirdangerclosest", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirinplainsight", "Elixirinplainsight", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirnewtoniannegation", "Elixirnewtoniannegation", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirnowyouseeme", "Elixirnowyouseeme", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirstockoption", "Elixirstockoption", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirboardgames", "Elixirboardgames", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirburnedout", "Elixirburnedout", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixircrawlspace", "Elixircrawlspace", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirpopshocks", "Elixirpopshocks", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirtemporalgift", "Elixirtemporalgift", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirpointdrops", "Elixirpointdrops", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixiralchemicalantithesis", "Elixiralchemicalantithesis", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirswordflay", "Elixirswordflay", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirdeadofnuclearwinter", "Elixirdeadofnuclearwinter", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirlicensedcontractor", "Elixirlicensedcontractor", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirundeadmanwalking", "Elixirundeadmanwalking", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirwhoskeepingscore", "Elixirwhoskeepingscore", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixiraftertaste", "Elixiraftertaste", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirextracredit", "Elixirextracredit", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirkilljoy", "Elixirkilljoy", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirsodafountain", "Elixirsodafountain", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirctrlz", "Elixirctrlz", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirfreefire", "Elixirfreefire", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixircacheback", "Elixircacheback", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirimmolationliquidation", "Elixirimmolationliquidation", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirphoenixup", "Elixirphoenixup", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirpowerkeg", "Elixirpowerkeg", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirblooddebt", "Elixirblooddebt", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirneardeathexperience", "Elixirneardeathexperience", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirperkaholic", "Elixirperkaholic", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirwallpower", "Elixirwallpower", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirantientrapment", "Elixirantientrapment", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirequipmint", "Elixirequipmint", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirheadscan", "Elixirheadscan", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirjointheparty", "Elixirjointheparty", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirnowherebutthere", "Elixirnowherebutthere", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirphantomreload", "Elixirphantomreload", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirshieldsup", "Elixirshieldsup", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirwalltowall", "Elixirwalltowall", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirconflagrationliquidation", "Elixirconflagrationliquidation", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirperkup", "Elixirperkup", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Elixirs", "zmelixirrefreshmint", "Elixirrefreshmint", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismansenabled", "Talismans", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismansindividual", "Talismansindividual", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanscommon", "Talismanscommon", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismansrare", "Talismansrare", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanslegendary", "Talismanslegendary", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismansepic", "Talismansepic", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanboxguaranteeboxonly", "Talismanboxguaranteeboxonly", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanboxguaranteelmg", "Talismanboxguaranteelmg", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismancoagulant", "Talismancoagulant", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanextraclaymore", "Talismanextraclaymore", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanextrafrag", "Talismanextrafrag", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanextraminiturret", "Talismanextraminiturret", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanextramolotov", "Talismanextramolotov", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanextrasemtex", "Talismanextrasemtex", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanimpatient", "Talismanimpatient", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkmodsingle", "Talismanperkmodsingle", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkpermanent1", "Talismanperkperm1", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkpermanent2", "Talismanperkperm2", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkpermanent3", "Talismanperkperm3", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkpermanent4", "Talismanperkperm4", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkreducecost1", "Talismanperkreducecost1", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkreducecost2", "Talismanperkreducecost2", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkreducecost3", "Talismanperkreducecost3", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkreducecost4", "Talismanperkreducecost4", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkstart1", "Talismanperkstart1", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkstart2", "Talismanperkstart2", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkstart3", "Talismanperkstart3", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanperkstart4", "Talismanperkstart4", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanshielddurabilitylegendary", "Talismanshielddurabilitylegendary", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanshielddurabilityrare", "Talismanshielddurabilityrare", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanshieldprice", "Talismanshieldprice", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanspecialstartlvl2", "Talismanspecialstart2", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanspecialstartlvl3", "Talismanspecialstart3", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanspecialxprate", "Talismanspecialxprate", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanstartweaponar", "Talismanweaponstartar", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanstartweaponlmg", "Talismanweaponstartlmg", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanstartweaponsmg", "Talismanweaponstartsmg", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismanreducepapcost", "Talismanweaponpapcost", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Talismans", "zmtalismansultra", "Talismansultra", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Traps", "zmtrapsenabled", "Trapsenabled", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Map", "zmdoorstate", "Doors", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Map", "zmpowerstate", "Power", 1, { { 0, "Off" }, { 1, "1" }, { 2, "On" } } },
    { "Map", "zmpowerdoorstate", "Powerdoorstate", 1, { { 0, "0" }, { 1, "1" }, { 2, "2" } } },
    { "Other", "hash_5d65c0983698a539", "Hash 5D65C0983698A539", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "hash_23fe21eb92ffbc2c", "Hash 23Fe21Eb92Ffbc2C", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "hash_751384283abde22c", "Hash 751384283Abde22C", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "zmwallbuysenabled", "Wallbuys", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "zmrandomwallbuys", "Randomwallbuys", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "zmheadshotsonly", "Headshots", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "hash_5566698b97a6282e", "Hash 5566698B97A6282E", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "hash_12f776f6bc579bb4", "Hash 12F776F6Bc579Bb4", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "hash_3c5363541b97ca3e", "Hash 3C5363541B97Ca3E", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "hash_61695e52556ff2d1", "Hash 61695E52556Ff2D1", 1, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "hash_429b520a87274afb", "Hash 429B520A87274Afb", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "zmselfreviveamount", "Selfrevivecount", 0, { { 0, "Off" }, { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" }, { 5, "5" }, { 6, "6" }, { 7, "7" }, { 8, "8" }, { 9, "9" }, { 10, "10" }, { 11, "11" }, { 12, "12" }, { 13, "13" }, { 14, "14" }, { 15, "15" }, { 16, "16" }, { 17, "17" }, { 18, "18" }, { 19, "19" }, { 20, "20" }, { 21, "21" }, { 22, "22" }, { 23, "23" }, { 24, "24" }, { 25, "25" }, { 26, "26" }, { 27, "27" }, { 28, "28" }, { 30, "30" }, { 32, "32" }, { 34, "34" }, { 35, "35" }, { 39, "39" } } },
    { "Other", "zmcraftingkeyline", "Craftingkeyline", 0, { { 0, "Off" }, { 1, "On" } } },
    { "Other", "zmfriendlyfiretype", "Friendly Fire Type", 0, { { 0, "0" }, { 1, "1" }, { 2, "2" }, { 3, "3" } } },
};
	// -----------------------------------------------------------------------
	//  Core mechanism (preserved from the original lobby tool):
	//  Allocate the cfg text into the game, point the
	//  default_systemlink.cfg rawfile entry at it, then exec it via the cbuf.
	//  This is mode-agnostic — "gts <name> <value>" works for zombies the same
	//  as for the modes the original tool targeted.
	// -----------------------------------------------------------------------
	void CfgCbuf(std::string&& cfg, std::string& logs) {
		Process bo4 = L"BlackOps4.exe";

		if (!bo4) { logs = "Can't find game"; return; }
		if (!bo4.Open()) { logs = "Can't open game"; return; }

		size_t sizeOut;
		uintptr_t alloc{ bo4.AllocateString(cfg.c_str(), &sizeOut) };

		if (!alloc) { logs = "Can't allocate string"; return; }

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
				throw std::runtime_error("Bad rawfile entry size");
			}

			bool patched{};
			for (int32_t i = 0; i < pe->itemCount; i++) {
				RawFileEntry e{ bo4.ReadMemoryObjectEx<RawFileEntry>(pe->pool + (uintptr_t)i * sizeof(RawFileEntry)).get()[0] };
				if (e.name == entry.name) {
					if (!bo4.WriteMemory(pe->pool + (uintptr_t)i * sizeof(RawFileEntry), &entry, sizeof(entry))) {
						throw std::runtime_error("Can't patch rawfile");
					}
					patched = true;
					break;
				}
			}

			if (!patched) {
				throw std::runtime_error("Can't find default_systemlink.cfg");
			}

			// NOTE: the original tool cached the previous allocation and freed it
			// on the next injection, guarded by a process-id comparison. This
			// ACTS Process class does not expose a process-id accessor, so we
			// drop that guard. Each injection allocates a small cfg string
			// (a few KB) that is referenced by the rawfile entry until the next
			// injection replaces it; not freeing the prior one leaks only a few
			// KB per inject within a session, which is harmless. Avoiding the
			// free entirely is strictly safer than risking a free of a stale
			// handle, and removes the dependency on the unavailable API.

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
			logs = "Injected " + std::to_string(zmSettings.size()) + " settings";
		}
		catch (std::runtime_error& re) {
			if (alloc) { bo4.FreeMemory(alloc, sizeOut); }
			logs = re.what();
		}
	}

	// -----------------------------------------------------------------------
	//  Zombies custom mutations UI.
	// -----------------------------------------------------------------------
	void zm_mutations_tool() {
		tool::nui::NuiUseDefaultWindow dw{};
		ImGui::SeparatorText("Zombies Custom Mutations");

		// Per-setting current selection (index into that setting's opts list).
		// Initialised once to each setting's DEFAULT value, persisted in config.
		static std::vector<int> sel(zmSettings.size(), 0);
		static int init{ ([]() {
			for (size_t i = 0; i < zmSettings.size(); i++) {
				const auto& s = zmSettings[i];
				// find index of the default value within opts
				int defIdx = 0;
				for (size_t k = 0; k < s.opts.size(); k++) {
					if (s.opts[k].value == s.def) { defIdx = (int)k; break; }
				}
				sel[i] = core::config::GetInteger(std::format("ui.zmmut.{}", s.name), defIdx);
			}
			return 0;
		})() };

		static std::string log{};

		ImGui::TextWrapped("Defaults are pre-selected. Injecting without changes leaves the match normal. Change any dropdown then press Inject.");
		ImGui::Spacing();

		// Scrollable region containing all categories.
		ImGui::BeginChild("##zmmut_scroll", ImVec2(0, 520), true);

		const char* curCat = nullptr;
		bool catOpen = false;
		for (size_t i = 0; i < zmSettings.size(); i++) {
			const auto& s = zmSettings[i];

			// New category header (collapsing section).
			if (curCat == nullptr || std::strcmp(curCat, s.category) != 0) {
				if (curCat != nullptr && catOpen) {
					ImGui::Unindent();
					ImGui::Spacing();
				}
				curCat = s.category;
				catOpen = ImGui::CollapsingHeader(curCat, ImGuiTreeNodeFlags_DefaultOpen);
				if (catOpen) ImGui::Indent();
			}

			if (!catOpen) continue;

			// Setting name label.
			ImGui::Text("%s", s.label);

			// Dropdown of this setting's valid values.
			std::string preview = s.opts[sel[i]].label;
			std::string comboId = std::string("##") + s.name;
			if (ImGui::BeginCombo(comboId.c_str(), preview.c_str())) {
				for (size_t k = 0; k < s.opts.size(); k++) {
					bool isSel = (sel[i] == (int)k);
					std::string optLabel = s.opts[k].label;
					if (s.opts[k].value == s.def) optLabel += " (default)";
					if (ImGui::Selectable(optLabel.c_str(), isSel)) {
						sel[i] = (int)k;
						core::config::SetInteger(std::format("ui.zmmut.{}", s.name), sel[i]);
						tool::nui::SaveNextConfig();
					}
					if (isSel) ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		if (curCat != nullptr && catOpen) ImGui::Unindent();

		ImGui::EndChild();

		ImGui::Spacing();
		ImGui::Separator();

		// Reset-to-defaults convenience.
		if (ImGui::Button("Reset all to default")) {
			for (size_t i = 0; i < zmSettings.size(); i++) {
				const auto& s = zmSettings[i];
				int defIdx = 0;
				for (size_t k = 0; k < s.opts.size(); k++) {
					if (s.opts[k].value == s.def) { defIdx = (int)k; break; }
				}
				sel[i] = defIdx;
				core::config::SetInteger(std::format("ui.zmmut.{}", s.name), sel[i]);
			}
			tool::nui::SaveNextConfig();
			log = "Reset to defaults";
		}

		ImGui::SameLine();

		// The single Inject button: writes every setting's chosen value.
		if (ImGui::Button("Inject all settings")) {
			std::stringstream gtsCfg{};
			for (size_t i = 0; i < zmSettings.size(); i++) {
				const auto& s = zmSettings[i];
				gtsCfg << "gts " << s.name << " " << s.opts[sel[i]].value << "\n";
			}
			CfgCbuf(gtsCfg.str(), log);
		}

		if (!log.empty()) {
			ImGui::Separator();
			ImGui::Text("%s", log.data());
		}
	}
	ADD_TOOL_NUI(zm_mutations_tool, "BO4 Zombies Mutations", zm_mutations_tool);
}
#endif
