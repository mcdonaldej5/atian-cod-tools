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

// AUTO-GENERATED from zm_customgame.gsc verified defaults table (221 settings).
struct ZmSetting { const char* category; const char* name; const char* label; int def; std::vector<int> opts; };
static const std::vector<ZmSetting> zmSettings = {
    { "General", "startround", "startround", 1, { 0, 1 } },
    { "General", "headshotsonly", "headshotsonly", 0, { 0, 1 } },
    { "General", "zmroundcap", "roundcap", 0, { 0, 1 } },
    { "General", "zmtimecap", "timecap", 0, { 0, 1 } },
    { "General", "zmshowtimer", "showtimer", 0, { 0, 1 } },
    { "General", "zmkillcap", "killcap", 0, { 0, 1 } },
    { "General", "zmspecialroundsenabled", "specialroundsenabled", 1, { 0, 1 } },
    { "General", "zmendonquest", "endonquest", 0, { 0, 1 } },
    { "Enemies", "zmrunnerstate", "runnerstate", 0, { 0, 1 } },
    { "Enemies", "zmwalkerstate", "walkerstate", 0, { 0, 1 } },
    { "Enemies", "zmcrawlerstate", "crawlerstate", 1, { 0, 1, 2 } },
    { "Enemies", "zmzombiespread", "zombiespread", 1, { 0, 1, 2 } },
    { "Enemies", "zmzombieminspeed", "zombieminspeed", 0, { 0, 1, 2, 3 } },
    { "Enemies", "zmzombiemaxspeed", "zombiemaxspeed", 3, { 0, 1, 2, 3 } },
    { "Enemies", "zmzombiedamagemult", "zombiedamagemult", 1, { 0, 1, 2 } },
    { "Enemies", "zmzombiehealthmult", "zombiehealthmult", 1, { 0, 1, 2 } },
    { "Enemies", "zmcrawlerdamagemult", "crawlerdamagemult", 1, { 0, 1 } },
    { "Enemies", "zmcrawlerhealthmult", "crawlerhealthmult", 1, { 0, 1 } },
    { "Enemies", "zmcrawleraggro", "crawleraggro", 1, { 0, 1 } },
    { "Enemies", "zmheavystate", "heavystate", 1, { 0, 1, 2 } },
    { "Enemies", "zmheavydamagemult", "heavydamagemult", 1, { 0, 1, 2 } },
    { "Enemies", "zmheavyhealthmult", "heavyhealthmult", 1, { 0, 1, 2 } },
    { "Enemies", "zmheavyaggro", "heavyaggro", 1, { 0, 1 } },
    { "Enemies", "zmminibossstate", "minibossstate", 1, { 0, 1, 2 } },
    { "Enemies", "zmminibossdamagemult", "minibossdamagemult", 1, { 0, 1, 2 } },
    { "Enemies", "zmminibosshealthmult", "minibosshealthmult", 1, { 0, 1, 2 } },
    { "Enemies", "zmminibossaggro", "minibossaggro", 1, { 0, 1 } },
    { "Enemies", "zmheavyspawnfreq", "heavyspawnfreq", 1, { 0, 1, 2, 3 } },
    { "Enemies", "zmminibossspawnfreq", "minibossspawnfreq", 1, { 0, 1, 2, 3 } },
    { "Enemies", "zmpopcornstate", "popcornstate", 1, { 0, 1, 2 } },
    { "Enemies", "zmpopcorndamagemult", "popcorndamagemult", 1, { 0, 1, 2 } },
    { "Enemies", "zmpopcornhealthmult", "popcornhealthmult", 1, { 0, 1, 2 } },
    { "Enemies", "zmpopcornspawnfreq", "popcornspawnfreq", 1, { 0, 1, 2, 3 } },
    { "Enemies", "zmenhancedstate", "enhancedstate", 1, { 0, 1, 2 } },
    { "Enemies", "zmenhanceddamagemult", "enhanceddamagemult", 1, { 0, 1, 2 } },
    { "Enemies", "zmenhancedhealthmult", "enhancedhealthmult", 1, { 0, 1, 2 } },
    { "Enemies", "zmenhancedspawnfreq", "enhancedspawnfreq", 1, { 0, 1, 2, 3 } },
    { "Health", "zmhealthstartingbars", "healthstartingbars", 3, { 0, 1, 2, 3 } },
    { "Health", "zmhealthregendelay", "healthregendelay", 1, { 0, 1, 2 } },
    { "Health", "zmhealthregenrate", "healthregenrate", 2, { 0, 1, 2, 3, 4 } },
    { "Health", "zmhealthonkill", "healthonkill", 0, { 0, 1, 2, 3 } },
    { "Health", "zmhealthdrain", "healthdrain", 0, { 0, 1, 2, 3 } },
    { "Health", "zmlaststandduration", "laststandduration", 2, { 0, 1, 2, 3 } },
    { "Health", "zmlimiteddownsamount", "limiteddownsamount", 0, { 0, 1 } },
    { "Health", "zmbarricadestate", "barricadestate", 1, { 0, 1 } },
    { "Health", "zmpointlossondown", "pointlossondown", 0, { 0, 1 } },
    { "Points", "zmpointsfixed", "pointsfixed", 0, { 0, 1 } },
    { "Points", "zmpointsstarting", "pointsstarting", 5, { 0, 1, 2, 3, 5 } },
    { "Points", "zmpointslosstype", "pointslosstype", 0, { 0, 1 } },
    { "Points", "zmpointslosspercent", "pointslosspercent", 1, { 0, 1 } },
    { "Points", "zmpointslossvalue", "pointslossvalue", 100, { 0, 1, 2, 3, 100 } },
    { "Points", "zmpointlossondeath", "pointlossondeath", 0, { 0, 1 } },
    { "Points", "zmpointlossonteammatedeath", "pointlossonteammatedeath", 0, { 0, 1 } },
    { "Perks", "zmperksactive", "perksactive", 1, { 0, 1 } },
    { "Perks", "zmperksjuggernaut", "perksjuggernaut", 1, { 0, 1 } },
    { "Perks", "zmperksspeed", "perksspeed", 1, { 0, 1 } },
    { "Perks", "zmperksquickrevive", "perksquickrevive", 1, { 0, 1 } },
    { "Perks", "zmperkswidowswail", "perkswidowswail", 1, { 0, 1 } },
    { "Perks", "zmperksstaminup", "perksstaminup", 1, { 0, 1 } },
    { "Perks", "zmperksmulekick", "perksmulekick", 1, { 0, 1 } },
    { "Perks", "zmperkselectricburst", "perkselectricburst", 1, { 0, 1 } },
    { "Perks", "zmperksdeadshot", "perksdeadshot", 1, { 0, 1 } },
    { "Perks", "zmperkscooldown", "perkscooldown", 1, { 0, 1 } },
    { "Perks", "zmperksdyingwish", "perksdyingwish", 1, { 0, 1 } },
    { "Perks", "zmperkdecay", "perkdecay", 1, { 0, 1 } },
    { "Perks", "zmperksbandolier", "perksbandolier", 1, { 0, 1 } },
    { "Perks", "zmperksdeathperception", "perksdeathperception", 1, { 0, 1 } },
    { "Perks", "zmperksphdslider", "perksphdslider", 1, { 0, 1 } },
    { "Perks", "zmperkssecretsauce", "perkssecretsauce", 1, { 0, 1 } },
    { "Perks", "zmperksstonecold", "perksstonecold", 1, { 0, 1 } },
    { "Perks", "zmperkswolfprotector", "perkswolfprotector", 1, { 0, 1 } },
    { "Perks", "zmperksdeathdash", "perksdeathdash", 1, { 0, 1 } },
    { "Perks", "zmperksvictorious", "perksvictorious", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupsactive", "powerupsactive", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupnuke", "powerupnuke", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupdouble", "powerupdouble", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupinstakill", "powerupinstakill", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupchaospoints", "powerupchaospoints", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupfiresale", "powerupfiresale", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupspecialweapon", "powerupspecialweapon", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupfreeperk", "powerupfreeperk", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupmaxammo", "powerupmaxammo", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupcarpenter", "powerupcarpenter", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupsislimitedround", "powerupsislimitedround", 0, { 0, 1 } },
    { "Power-Ups", "zmpowerupslimitround", "powerupslimitround", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupsharing", "powerupsharing", 1, { 0, 1 } },
    { "Power-Ups", "zmpowerupfrequency", "powerupfrequency", 1, { 0, 1 } },
    { "Weapons", "zmpapenabled", "papenabled", 1, { 0, 1 } },
    { "Weapons", "zmsuperpapenabled", "superpapenabled", 1, { 0, 1 } },
    { "Weapons", "zmweaponspistol", "weaponspistol", 1, { 0, 1 } },
    { "Weapons", "zmweaponsshotgun", "weaponsshotgun", 1, { 0, 1 } },
    { "Weapons", "zmweaponssmg", "weaponssmg", 1, { 0, 1 } },
    { "Weapons", "zmweaponsar", "weaponsar", 1, { 0, 1 } },
    { "Weapons", "zmweaponstr", "weaponstr", 1, { 0, 1 } },
    { "Weapons", "zmweaponslmg", "weaponslmg", 1, { 0, 1 } },
    { "Weapons", "zmweaponssniper", "weaponssniper", 1, { 0, 1 } },
    { "Weapons", "zmweaponsknife", "weaponsknife", 1, { 0, 1 } },
    { "Weapons", "zmweaponsmelee", "weaponsmelee", 1, { 0, 1 } },
    { "Weapons", "zmspecweaponisenabled", "specweaponisenabled", 1, { 0, 1 } },
    { "Weapons", "zmspecweaponchargerate", "specweaponchargerate", 1, { 0, 1 } },
    { "Weapons", "zmwonderweaponisenabled", "wonderweaponisenabled", 1, { 0, 1 } },
    { "Weapons", "zmequipmentisenabled", "equipmentisenabled", 1, { 0, 1 } },
    { "Weapons", "zmequipmentchargerate", "equipmentchargerate", 1, { 0, 1 } },
    { "Weapons", "zmshieldisenabled", "shieldisenabled", 1, { 0, 1 } },
    { "Weapons", "zmshielddurability", "shielddurability", 1, { 0, 1 } },
    { "Weapons", "zmstartingweaponenabled", "startingweaponenabled", 1, { 0, 1 } },
    { "Weapons", "zmretainweapons", "retainweapons", 1, { 0, 1 } },
    { "Mystery Box", "zmmysteryboxstate", "mysteryboxstate", 2, { 0, 1, 2, 3 } },
    { "Mystery Box", "zmmysteryboxlimitmove", "mysteryboxlimitmove", 0, { 0, 1 } },
    { "Mystery Box", "zmmysteryboxlimit", "mysteryboxlimit", 0, { 0, 1 } },
    { "Mystery Box", "zmmysteryboxlimitround", "mysteryboxlimitround", 0, { 0, 1 } },
    { "Elixirs", "zmelixirsenabled", "elixirsenabled", 1, { 0, 1 } },
    { "Elixirs", "zmelixirscooldown", "elixirscooldown", 1, { 0, 1 } },
    { "Elixirs", "zmelixirsindividual", "elixirsindividual", 1, { 0, 1 } },
    { "Elixirs", "zmelixirsdurables", "elixirsdurables", 1, { 0, 1 } },
    { "Elixirs", "zmelixirscommon", "elixirscommon", 1, { 0, 1 } },
    { "Elixirs", "zmelixirsrare", "elixirsrare", 1, { 0, 1 } },
    { "Elixirs", "zmelixirslegendary", "elixirslegendary", 1, { 0, 1 } },
    { "Elixirs", "zmelixirsepic", "elixirsepic", 1, { 0, 1 } },
    { "Elixirs", "zmelixiralwaysdoneswiftly", "elixiralwaysdoneswiftly", 1, { 0, 1 } },
    { "Elixirs", "zmelixiranywherebuthere", "elixiranywherebuthere", 1, { 0, 1 } },
    { "Elixirs", "zmelixirarsenalaccelerator", "elixirarsenalaccelerator", 1, { 0, 1 } },
    { "Elixirs", "zmelixirdangerclosest", "elixirdangerclosest", 1, { 0, 1 } },
    { "Elixirs", "zmelixirinplainsight", "elixirinplainsight", 1, { 0, 1 } },
    { "Elixirs", "zmelixirnewtoniannegation", "elixirnewtoniannegation", 1, { 0, 1 } },
    { "Elixirs", "zmelixirnowyouseeme", "elixirnowyouseeme", 1, { 0, 1 } },
    { "Elixirs", "zmelixirstockoption", "elixirstockoption", 1, { 0, 1 } },
    { "Elixirs", "zmelixirboardgames", "elixirboardgames", 1, { 0, 1 } },
    { "Elixirs", "zmelixirburnedout", "elixirburnedout", 1, { 0, 1 } },
    { "Elixirs", "zmelixircrawlspace", "elixircrawlspace", 1, { 0, 1 } },
    { "Elixirs", "zmelixirpopshocks", "elixirpopshocks", 1, { 0, 1 } },
    { "Elixirs", "zmelixirtemporalgift", "elixirtemporalgift", 1, { 0, 1 } },
    { "Elixirs", "zmelixirpointdrops", "elixirpointdrops", 1, { 0, 1 } },
    { "Elixirs", "zmelixiralchemicalantithesis", "elixiralchemicalantithesis", 1, { 0, 1 } },
    { "Elixirs", "zmelixirswordflay", "elixirswordflay", 1, { 0, 1 } },
    { "Elixirs", "zmelixirdeadofnuclearwinter", "elixirdeadofnuclearwinter", 1, { 0, 1 } },
    { "Elixirs", "zmelixirlicensedcontractor", "elixirlicensedcontractor", 1, { 0, 1 } },
    { "Elixirs", "zmelixirundeadmanwalking", "elixirundeadmanwalking", 1, { 0, 1 } },
    { "Elixirs", "zmelixirwhoskeepingscore", "elixirwhoskeepingscore", 1, { 0, 1 } },
    { "Elixirs", "zmelixiraftertaste", "elixiraftertaste", 1, { 0, 1 } },
    { "Elixirs", "zmelixirextracredit", "elixirextracredit", 1, { 0, 1 } },
    { "Elixirs", "zmelixirkilljoy", "elixirkilljoy", 1, { 0, 1 } },
    { "Elixirs", "zmelixirsodafountain", "elixirsodafountain", 1, { 0, 1 } },
    { "Elixirs", "zmelixirctrlz", "elixirctrlz", 1, { 0, 1 } },
    { "Elixirs", "zmelixirfreefire", "elixirfreefire", 1, { 0, 1 } },
    { "Elixirs", "zmelixircacheback", "elixircacheback", 1, { 0, 1 } },
    { "Elixirs", "zmelixirimmolationliquidation", "elixirimmolationliquidation", 1, { 0, 1 } },
    { "Elixirs", "zmelixirphoenixup", "elixirphoenixup", 1, { 0, 1 } },
    { "Elixirs", "zmelixirpowerkeg", "elixirpowerkeg", 1, { 0, 1 } },
    { "Elixirs", "zmelixirblooddebt", "elixirblooddebt", 1, { 0, 1 } },
    { "Elixirs", "zmelixirneardeathexperience", "elixirneardeathexperience", 1, { 0, 1 } },
    { "Elixirs", "zmelixirperkaholic", "elixirperkaholic", 1, { 0, 1 } },
    { "Elixirs", "zmelixirwallpower", "elixirwallpower", 1, { 0, 1 } },
    { "Elixirs", "zmelixirantientrapment", "elixirantientrapment", 1, { 0, 1 } },
    { "Elixirs", "zmelixirequipmint", "elixirequipmint", 1, { 0, 1 } },
    { "Elixirs", "zmelixirheadscan", "elixirheadscan", 1, { 0, 1 } },
    { "Elixirs", "zmelixirjointheparty", "elixirjointheparty", 1, { 0, 1 } },
    { "Elixirs", "zmelixirnowherebutthere", "elixirnowherebutthere", 1, { 0, 1 } },
    { "Elixirs", "zmelixirphantomreload", "elixirphantomreload", 1, { 0, 1 } },
    { "Elixirs", "zmelixirshieldsup", "elixirshieldsup", 1, { 0, 1 } },
    { "Elixirs", "zmelixirwalltowall", "elixirwalltowall", 1, { 0, 1 } },
    { "Elixirs", "zmelixirconflagrationliquidation", "elixirconflagrationliquidation", 1, { 0, 1 } },
    { "Elixirs", "zmelixirperkup", "elixirperkup", 1, { 0, 1 } },
    { "Elixirs", "zmelixirrefreshmint", "elixirrefreshmint", 1, { 0, 1 } },
    { "Talismans", "zmtalismansenabled", "talismansenabled", 1, { 0, 1 } },
    { "Talismans", "zmtalismansindividual", "talismansindividual", 1, { 0, 1 } },
    { "Talismans", "zmtalismanscommon", "talismanscommon", 1, { 0, 1 } },
    { "Talismans", "zmtalismansrare", "talismansrare", 1, { 0, 1 } },
    { "Talismans", "zmtalismanslegendary", "talismanslegendary", 1, { 0, 1 } },
    { "Talismans", "zmtalismansepic", "talismansepic", 1, { 0, 1 } },
    { "Talismans", "zmtalismanboxguaranteeboxonly", "talismanboxguaranteeboxonly", 1, { 0, 1 } },
    { "Talismans", "zmtalismanboxguaranteelmg", "talismanboxguaranteelmg", 1, { 0, 1 } },
    { "Talismans", "zmtalismancoagulant", "talismancoagulant", 1, { 0, 1 } },
    { "Talismans", "zmtalismanextraclaymore", "talismanextraclaymore", 1, { 0, 1 } },
    { "Talismans", "zmtalismanextrafrag", "talismanextrafrag", 1, { 0, 1 } },
    { "Talismans", "zmtalismanextraminiturret", "talismanextraminiturret", 1, { 0, 1 } },
    { "Talismans", "zmtalismanextramolotov", "talismanextramolotov", 1, { 0, 1 } },
    { "Talismans", "zmtalismanextrasemtex", "talismanextrasemtex", 1, { 0, 1 } },
    { "Talismans", "zmtalismanimpatient", "talismanimpatient", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkmodsingle", "talismanperkmodsingle", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkpermanent1", "talismanperkpermanent1", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkpermanent2", "talismanperkpermanent2", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkpermanent3", "talismanperkpermanent3", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkpermanent4", "talismanperkpermanent4", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkreducecost1", "talismanperkreducecost1", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkreducecost2", "talismanperkreducecost2", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkreducecost3", "talismanperkreducecost3", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkreducecost4", "talismanperkreducecost4", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkstart1", "talismanperkstart1", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkstart2", "talismanperkstart2", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkstart3", "talismanperkstart3", 1, { 0, 1 } },
    { "Talismans", "zmtalismanperkstart4", "talismanperkstart4", 1, { 0, 1 } },
    { "Talismans", "zmtalismanshielddurabilitylegendary", "talismanshielddurabilitylegendary", 1, { 0, 1 } },
    { "Talismans", "zmtalismanshielddurabilityrare", "talismanshielddurabilityrare", 1, { 0, 1 } },
    { "Talismans", "zmtalismanshieldprice", "talismanshieldprice", 1, { 0, 1 } },
    { "Talismans", "zmtalismanspecialstartlvl2", "talismanspecialstartlvl2", 1, { 0, 1 } },
    { "Talismans", "zmtalismanspecialstartlvl3", "talismanspecialstartlvl3", 1, { 0, 1 } },
    { "Talismans", "zmtalismanspecialxprate", "talismanspecialxprate", 1, { 0, 1 } },
    { "Talismans", "zmtalismanstartweaponar", "talismanstartweaponar", 1, { 0, 1 } },
    { "Talismans", "zmtalismanstartweaponlmg", "talismanstartweaponlmg", 1, { 0, 1 } },
    { "Talismans", "zmtalismanstartweaponsmg", "talismanstartweaponsmg", 1, { 0, 1 } },
    { "Talismans", "zmtalismanreducepapcost", "talismanreducepapcost", 1, { 0, 1 } },
    { "Talismans", "zmtalismansultra", "talismansultra", 1, { 0, 1 } },
    { "Traps", "zmtrapsenabled", "trapsenabled", 1, { 0, 1 } },
    { "Map", "zmdoorstate", "doorstate", 1, { 0, 1 } },
    { "Map", "zmpowerstate", "powerstate", 1, { 0, 1 } },
    { "Map", "zmpowerdoorstate", "powerdoorstate", 1, { 0, 1 } },
    { "Other", "hash_5d65c0983698a539", "hash_5d65c0983698a539", 0, { 0, 1 } },
    { "Other", "hash_23fe21eb92ffbc2c", "hash_23fe21eb92ffbc2c", 0, { 0, 1 } },
    { "Other", "hash_751384283abde22c", "hash_751384283abde22c", 0, { 0, 1 } },
    { "Other", "zmwallbuysenabled", "wallbuysenabled", 1, { 0, 1 } },
    { "Other", "zmrandomwallbuys", "randomwallbuys", 0, { 0, 1 } },
    { "Other", "zmheadshotsonly", "headshotsonly", 0, { 0, 1 } },
    { "Other", "hash_5566698b97a6282e", "hash_5566698b97a6282e", 0, { 0, 1 } },
    { "Other", "hash_12f776f6bc579bb4", "hash_12f776f6bc579bb4", 0, { 0, 1 } },
    { "Other", "hash_3c5363541b97ca3e", "hash_3c5363541b97ca3e", 1, { 0, 1 } },
    { "Other", "hash_61695e52556ff2d1", "hash_61695e52556ff2d1", 1, { 0, 1 } },
    { "Other", "hash_429b520a87274afb", "hash_429b520a87274afb", 0, { 0, 1 } },
    { "Other", "zmselfreviveamount", "selfreviveamount", 0, { 0, 1 } },
    { "Other", "zmcraftingkeyline", "craftingkeyline", 0, { 0, 1 } },
    { "Other", "zmfriendlyfiretype", "friendlyfiretype", 0, { 0, 1, 2, 3 } },
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

		static DWORD oldPid{};
		static uintptr_t oldAlloc{};
		static size_t oldAllocSize;

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

			if (bo4.GetProcessId() == oldPid && oldAlloc) {
				bo4.FreeMemory(oldAlloc, oldAllocSize);
				oldAlloc = 0;
			}

			oldPid = bo4.GetProcessId();
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
					if (s.opts[k] == s.def) { defIdx = (int)k; break; }
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
			std::string preview = std::to_string(s.opts[sel[i]]);
			std::string comboId = std::string("##") + s.name;
			if (ImGui::BeginCombo(comboId.c_str(), preview.c_str())) {
				for (size_t k = 0; k < s.opts.size(); k++) {
					bool isSel = (sel[i] == (int)k);
					std::string optLabel = std::to_string(s.opts[k]);
					if (s.opts[k] == s.def) optLabel += " (default)";
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
					if (s.opts[k] == s.def) { defIdx = (int)k; break; }
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
				gtsCfg << "gts " << s.name << " " << s.opts[sel[i]] << "\n";
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
