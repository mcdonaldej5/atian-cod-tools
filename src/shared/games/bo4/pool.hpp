#pragma once
#include <includes_shared.hpp>
#include <utils/hash.hpp>

struct XHash {
    uint64_t name;
    uint64_t str;

    constexpr bool operator==(const char* other) const {
        return name == hash::Hash64(other);
    }
    constexpr operator bool() const {
        return name;
    }
    constexpr operator uint64_t() const {
        return name;
    }
}; static_assert(sizeof(XHash) == 0x10);
namespace games::bo4::pool {
    typedef float vec2_t[2];
    typedef float vec3_t[3];
    typedef float vec4_t[4];

    enum XAssetType : byte{
        ASSET_TYPE_PHYSPRESET = 0,
        ASSET_TYPE_PHYSCONSTRAINTS = 1,
        ASSET_TYPE_DESTRUCTIBLEDEF = 2,
        ASSET_TYPE_XANIM = 3,
        ASSET_TYPE_XMODEL = 4,
        ASSET_TYPE_XMODELMESH = 5,
        ASSET_TYPE_MATERIAL = 6,
        ASSET_TYPE_COMPUTE_SHADER_SET = 7,
        ASSET_TYPE_TECHNIQUE_SET = 8,
        ASSET_TYPE_IMAGE = 9,
        ASSET_TYPE_SOUND = 10,
        ASSET_TYPE_CLIPMAP = 11,
        ASSET_TYPE_COMWORLD = 12,
        ASSET_TYPE_GAMEWORLD = 13,
        ASSET_TYPE_GFXWORLD = 14,
        ASSET_TYPE_FONTICON = 15,
        ASSET_TYPE_LOCALIZE_ENTRY = 16,
        ASSET_TYPE_LOCALIZE_LIST = 17,
        ASSET_TYPE_GESTURE = 18,
        ASSET_TYPE_GESTURE_TABLE = 19,
        ASSET_TYPE_WEAPON = 20,
        ASSET_TYPE_WEAPON_FULL = 21,
        ASSET_TYPE_WEAPON_TUNABLES = 22,
        ASSET_TYPE_CGMEDIA = 23,
        ASSET_TYPE_PLAYERSOUNDS = 24,
        ASSET_TYPE_PLAYERFX = 25,
        ASSET_TYPE_SHAREDWEAPONSOUNDS = 26,
        ASSET_TYPE_ATTACHMENT = 27,
        ASSET_TYPE_ATTACHMENT_UNIQUE = 28,
        ASSET_TYPE_WEAPON_CAMO = 29,
        ASSET_TYPE_CUSTOMIZATION_TABLE = 30,
        ASSET_TYPE_CUSTOMIZATION_TABLE_FE_IMAGES = 31,
        ASSET_TYPE_SNDDRIVER_GLOBALS = 32,
        ASSET_TYPE_FX = 33,
        ASSET_TYPE_TAGFX = 34,
        ASSET_TYPE_KLF = 35,
        ASSET_TYPE_IMPACT_FX = 36,
        ASSET_TYPE_IMPACT_SOUND = 37,
        ASSET_TYPE_AITYPE = 38,
        ASSET_TYPE_charACTER = 39,
        ASSET_TYPE_XMODELALIAS = 40,
        ASSET_TYPE_RAWFILE = 41,
        ASSET_TYPE_XANIM_TREE = 42,
        ASSET_TYPE_STRINGTABLE = 43,
        ASSET_TYPE_STRUCTURED_TABLE = 44,
        ASSET_TYPE_LEADERBOARD = 45,
        ASSET_TYPE_DDL = 46,
        ASSET_TYPE_GLASSES = 47,
        ASSET_TYPE_SCRIPTPARSETREE = 48,
        ASSET_TYPE_SCRIPTPARSETREEDBG = 49,
        ASSET_TYPE_SCRIPTPARSETREEFORCED = 50,
        ASSET_TYPE_KEYVALUEPAIRS = 51,
        ASSET_TYPE_VEHICLEDEF = 52,
        ASSET_TYPE_TRACER = 53,
        ASSET_TYPE_SURFACEFX_TABLE = 54,
        ASSET_TYPE_SURFACESOUNDDEF = 55,
        ASSET_TYPE_FOOTSTEP_TABLE = 56,
        ASSET_TYPE_ENTITYFXIMPACTS = 57,
        ASSET_TYPE_ENTITYSOUNDIMPACTS = 58,
        ASSET_TYPE_ZBARRIER = 59,
        ASSET_TYPE_VEHICLEFXDEF = 60,
        ASSET_TYPE_VEHICLESOUNDDEF = 61,
        ASSET_TYPE_TYPEINFO = 62,
        ASSET_TYPE_SCRIPTBUNDLE = 63,
        ASSET_TYPE_SCRIPTBUNDLELIST = 64,
        ASSET_TYPE_RUMBLE = 65,
        ASSET_TYPE_BULLETPENETRATION = 66,
        ASSET_TYPE_LOCDMGTABLE = 67,
        ASSET_TYPE_AIMTABLE = 68,
        ASSET_TYPE_SHOOTTABLE = 69,
        ASSET_TYPE_PLAYERGLOBALTUNABLES = 70,
        ASSET_TYPE_ANIMSELECTORTABLESET = 71,
        ASSET_TYPE_ANIMMAPPINGTABLE = 72,
        ASSET_TYPE_ANIMSTATEMACHINE = 73,
        ASSET_TYPE_BEHAVIORTREE = 74,
        ASSET_TYPE_BEHAVIORSTATEMACHINE = 75,
        ASSET_TYPE_TTF = 76,
        ASSET_TYPE_SANIM = 77,
        ASSET_TYPE_LIGHT_DESCRIPTION = 78,
        ASSET_TYPE_SHELLSHOCK = 79,
        ASSET_TYPE_STATUS_EFFECT = 80,
        ASSET_TYPE_CINEMATIC_CAMERA = 81,
        ASSET_TYPE_CINEMATIC_SEQUENCE = 82,
        ASSET_TYPE_SPECTATE_CAMERA = 83,
        ASSET_TYPE_XCAM = 84,
        ASSET_TYPE_BG_CACHE = 85,
        ASSET_TYPE_TEXTURE_COMBO = 86,
        ASSET_TYPE_FLAMETABLE = 87,
        ASSET_TYPE_BITFIELD = 88,
        ASSET_TYPE_MAPTABLE = 89,
        ASSET_TYPE_MAPTABLE_LIST = 90,
        ASSET_TYPE_MAPTABLE_LOADING_IMAGES = 91,
        ASSET_TYPE_MAPTABLE_PREVIEW_IMAGES = 92,
        ASSET_TYPE_MAPTABLEENTRY_LEVEL_ASSETS = 93,
        ASSET_TYPE_OBJECTIVE = 94,
        ASSET_TYPE_OBJECTIVE_LIST = 95,
        ASSET_TYPE_NAVMESH = 96,
        ASSET_TYPE_NAVVOLUME = 97,
        ASSET_TYPE_LASER = 98,
        ASSET_TYPE_BEAM = 99,
        ASSET_TYPE_STREAMER_HINT = 100,
        ASSET_TYPE_FLOWGRAPH = 101,
        ASSET_TYPE_POSTFXBUNDLE = 102,
        ASSET_TYPE_LUAFILE = 103,
        ASSET_TYPE_LUAFILE_DBG = 104,
        ASSET_TYPE_RENDEROVERRIDEBUNDLE = 105,
        ASSET_TYPE_STATIC_LEVEL_FX_LIST = 106,
        ASSET_TYPE_TRIGGER_LIST = 107,
        ASSET_TYPE_PLAYER_ROLE_TEMPLATE = 108,
        ASSET_TYPE_PLAYER_ROLE_CATEGORY_TABLE = 109,
        ASSET_TYPE_PLAYER_ROLE_CATEGORY = 110,
        ASSET_TYPE_charACTER_BODY_TYPE = 111,
        ASSET_TYPE_PLAYER_OUTFIT = 112,
        ASSET_TYPE_GAMETYPETABLE = 113,
        ASSET_TYPE_FEATURE = 114,
        ASSET_TYPE_FEATURETABLE = 115,
        ASSET_TYPE_UNLOCKABLE_ITEM = 116,
        ASSET_TYPE_UNLOCKABLE_ITEM_TABLE = 117,
        ASSET_TYPE_ENTITY_LIST = 118,
        ASSET_TYPE_PLAYLISTS = 119,
        ASSET_TYPE_PLAYLIST_GLOBAL_SETTINGS = 120,
        ASSET_TYPE_PLAYLIST_SCHEDULE = 121,
        ASSET_TYPE_MOTION_MATCHING_INPUT = 122,
        ASSET_TYPE_BLACKBOARD = 123,
        ASSET_TYPE_TACTICALQUERY = 124,
        ASSET_TYPE_PLAYER_MOVEMENT_TUNABLES = 125,
        ASSET_TYPE_HIERARCHICAL_TASK_NETWORK = 126,
        ASSET_TYPE_RAGDOLL = 127,
        ASSET_TYPE_STORAGEFILE = 128,
        ASSET_TYPE_STORAGEFILELIST = 129,
        ASSET_TYPE_charMIXER = 130,
        ASSET_TYPE_STOREPRODUCT = 131,
        ASSET_TYPE_STORECATEGORY = 132,
        ASSET_TYPE_STORECATEGORYLIST = 133,
        ASSET_TYPE_RANK = 134,
        ASSET_TYPE_RANKTABLE = 135,
        ASSET_TYPE_PRESTIGE = 136,
        ASSET_TYPE_PRESTIGETABLE = 137,
        ASSET_TYPE_FIRSTPARTYENTITLEMENT = 138,
        ASSET_TYPE_FIRSTPARTYENTITLEMENTLIST = 139,
        ASSET_TYPE_ENTITLEMENT = 140,
        ASSET_TYPE_ENTITLEMENTLIST = 141,
        ASSET_TYPE_SKU = 142,
        ASSET_TYPE_LABELSTORE = 143,
        ASSET_TYPE_LABELSTORELIST = 144,
        ASSET_TYPE_CPU_OCCLUSION_DATA = 145,
        ASSET_TYPE_LIGHTING = 146,
        ASSET_TYPE_STREAMERWORLD = 147,
        ASSET_TYPE_TALENT = 148,
        ASSET_TYPE_PLAYERTALENTTEMPLATE = 149,
        ASSET_TYPE_PLAYERANIMATION = 150,
        ASSET_TYPE_ERR_UNUSED = 151,
        ASSET_TYPE_TERRAINGFX = 152,
        ASSET_TYPE_HIGHLIGHTREELINFODEFINES = 153,
        ASSET_TYPE_HIGHLIGHTREELPROFILEWEIGHTING = 154,
        ASSET_TYPE_HIGHLIGHTREELSTARLEVELS = 155,
        ASSET_TYPE_DLOGEVENT = 156,
        ASSET_TYPE_RAWSTRING = 157,
        ASSET_TYPE_BALLISTICDESC = 158,
        ASSET_TYPE_STREAMKEY = 159,
        ASSET_TYPE_RENDERTARGETS = 160,
        ASSET_TYPE_DRAWNODES = 161,
        ASSET_TYPE_GROUPLODMODEL = 162,
        ASSET_TYPE_FXLIBRARYVOLUME = 163,
        ASSET_TYPE_ARENASEASONS = 164,
        ASSET_TYPE_SPRAYORGESTUREITEM = 165,
        ASSET_TYPE_SPRAYORGESTURELIST = 166,
        ASSET_TYPE_HWPLATFORM = 167,
        ASSET_TYPE_COUNT = 168,
        ASSET_TYPE_ASSETLIST = 168,
        ASSET_TYPE_REPORT = 169,
        ASSET_TYPE_FULL_COUNT = 170,
    };

    enum BGCacheTypes : unsigned __int8
    {
        BG_CACHE_TYPE_INVALID = 0x0,
        BG_CACHE_TYPE_VEHICLE = 0x1,
        BG_CACHE_TYPE_MODEL = 0x2,
        BG_CACHE_TYPE_AITYPE = 0x3,
        BG_CACHE_TYPE_charACTER = 0x4,
        BG_CACHE_TYPE_XMODELALIAS = 0x5,
        BG_CACHE_TYPE_WEAPON = 0x6,
        BG_CACHE_TYPE_GESTURE = 0x7,
        BG_CACHE_TYPE_GESTURETABLE = 0x8,
        BG_CACHE_TYPE_ZBARRIER = 0x9,
        BG_CACHE_TYPE_RUMBLE = 0xA,
        BG_CACHE_TYPE_SHELLSHOCK = 0xB,
        BG_CACHE_TYPE_STATUSEFFECT = 0xC,
        BG_CACHE_TYPE_XCAM = 0xD,
        BG_CACHE_TYPE_DESTRUCTIBLE = 0xE,
        BG_CACHE_TYPE_STREAMERHINT = 0xF,
        BG_CACHE_TYPE_FLOWGRAPH = 0x10,
        BG_CACHE_TYPE_XANIM = 0x11,
        BG_CACHE_TYPE_SANIM = 0x12,
        BG_CACHE_TYPE_SCRIPTBUNDLE = 0x13,
        BG_CACHE_TYPE_TALENT = 0x14,
        BG_CACHE_TYPE_STATUSICON = 0x15,
        BG_CACHE_TYPE_LOCATIONSELECTOR = 0x16,
        BG_CACHE_TYPE_MENU = 0x17,
        BG_CACHE_TYPE_MATERIAL = 0x18,
        BG_CACHE_TYPE_STRING = 0x19,
        BG_CACHE_TYPE_EVENTSTRING = 0x1A,
        BG_CACHE_TYPE_MOVIEFILE = 0x1B,
        BG_CACHE_TYPE_OBJECTIVE = 0x1C,
        BG_CACHE_TYPE_FX = 0x1D,
        BG_CACHE_TYPE_LUI_MENU_DATA = 0x1E,
        BG_CACHE_TYPE_LUI_ELEM = 0x1F,
        BG_CACHE_TYPE_LUI_ELEM_UID = 0x20,
        BG_CACHE_TYPE_RADIANT_EXPLODER = 0x21,
        BG_CACHE_TYPE_SOUNDALIAS = 0x22,
        BG_CACHE_TYPE_CLIENT_FX = 0x23,
        BG_CACHE_TYPE_CLIENT_TAGFXSET = 0x24,
        BG_CACHE_TYPE_CLIENT_LUI_ELEM = 0x25,
        BG_CACHE_TYPE_CLIENT_LUI_ELEM_UID = 0x26,
        BG_CACHE_TYPE_REQUIRES_IMPLEMENTS = 0x27,
        BG_CACHE_TYPE_COUNT
    };

    enum team_t : int32_t {
        TEAM_FREE,
        TEAM_ALLIES,
        TEAM_AXIS,
        TEAM_THREE,
        TEAM_FOUR,
        TEAM_FIVE,
        TEAM_SIX,
        TEAM_SEVEN,
        TEAM_EIGHT,
        TEAM_NINE,
        TEAM_TEN,
        TEAM_ELEVEN,
        TEAM_TWELVE,
        TEAM_THIRTEEN,
        TEAM_FOURTEEN,
        TEAM_FIFTEEN,
        TEAM_SIXTEEN,
        TEAM_SEVENTEEN,
        TEAM_EIGHTEEN,
        TEAM_NINETEEN,
        TEAM_TWENTY,
        TEAM_TWENTYONE,
        TEAM_TWENTYTWO,
        TEAM_TWENTYTHREE,
        TEAM_TWENTYFOUR,
        TEAM_TWENTYFIVE,
        TEAM_TWENTYSIX,
        TEAM_TWENTYSEVEN,
        TEAM_TWENTYEIGHT,
        TEAM_TWENTYNINE,
        TEAM_THIRTY,
        TEAM_THIRTYONE,
        TEAM_THIRTYTWO,
        TEAM_THIRTYTHREE,
        TEAM_THIRTYFOUR,
        TEAM_THIRTYFIVE,
        TEAM_THIRTYSIX,
        TEAM_THIRTYSEVEN,
        TEAM_THIRTYEIGHT,
        TEAM_THIRTYNINE,
        TEAM_FOURTY,
        TEAM_FOURTYONE,
        TEAM_FOURTYTWO,
        TEAM_FOURTYTHREE,
        TEAM_FOURTYFOUR,
        TEAM_FOURTYFIVE,
        TEAM_FOURTYSIX,
        TEAM_FOURTYSEVEN,
        TEAM_FOURTYEIGHT,
        TEAM_FOURTYNINE,
        TEAM_FIFTY,
        TEAM_FIFTYONE,
        TEAM_FIFTYTWO,
        TEAM_FIFTYTHREE,
        TEAM_FIFTYFOUR,
        TEAM_FIFTYFIVE,
        TEAM_FIFTYSIX,
        TEAM_FIFTYSEVEN,
        TEAM_FIFTYEIGHT,
        TEAM_FIFTYNINE,
        TEAM_SIXTY,
        TEAM_SIXTYONE,
        TEAM_SIXTYTWO,
        TEAM_SIXTYTHREE,
        TEAM_SIXTYFOUR,
        TEAM_SIXTYFIVE,
        TEAM_SIXTYSIX,
        TEAM_SIXTYSEVEN,
        TEAM_SIXTYEIGHT,
        TEAM_SIXTYNINE,
        TEAM_SEVENTY,
        TEAM_SEVENTYONE,
        TEAM_SEVENTYTWO,
        TEAM_SEVENTYTHREE,
        TEAM_SEVENTYFOUR,
        TEAM_SEVENTYFIVE,
        TEAM_SEVENTYSIX,
        TEAM_SEVENTYSEVEN,
        TEAM_SEVENTYEIGHT,
        TEAM_SEVENTYNINE,
        TEAM_EIGHTY,
        TEAM_EIGHTYONE,
        TEAM_EIGHTYTWO,
        TEAM_EIGHTYTHREE,
        TEAM_EIGHTYFOUR,
        TEAM_EIGHTYFIVE,
        TEAM_EIGHTYSIX,
        TEAM_EIGHTYSEVEN,
        TEAM_EIGHTYEIGHT,
        TEAM_EIGHTYNINE,
        TEAM_NINETY,
        TEAM_NINETYONE,
        TEAM_NINETYTWO,
        TEAM_NINETYTHREE,
        TEAM_NINETYFOUR,
        TEAM_NINETYFIVE,
        TEAM_NINETYSIX,
        TEAM_NINETYSEVEN,
        TEAM_NINETYEIGHT,
        TEAM_NINETYNINE,
        TEAM_ONEHUNDRED,
        TEAM_ONEHUNDRED_ONE,
        TEAM_ONEHUNDRED_TWO,
        TEAM_ONEHUNDRED_THREE,
        TEAM_ONEHUNDRED_FOUR,
        TEAM_ONEHUNDRED_FIVE,
        TEAM_ONEHUNDRED_SIX,
        TEAM_ONEHUNDRED_SEVEN,
        TEAM_ONEHUNDRED_EIGHT,
        TEAM_ONEHUNDRED_NINE,
        TEAM_ONEHUNDRED_TEN,
        TEAM_ONEHUNDRED_ELEVEN,
        TEAM_ONEHUNDRED_TWELVE,
        TEAM_ONEHUNDRED_THIRTEEN,
        TEAM_ONEHUNDRED_FOURTEEN,
        TEAM_ONEHUNDRED_FIFTEEN,
        TEAM_ONEHUNDRED_SIXTEEN,
        TEAM_ONEHUNDRED_SEVENTEEN,
        TEAM_ONEHUNDRED_EIGHTEEN,
        TEAM_ONEHUNDRED_NINETEEN,
        TEAM_ONEHUNDRED_TWENTY,
        TEAM_ONEHUNDRED_TWENTYONE,
        TEAM_ONEHUNDRED_TWENTYTWO,
        TEAM_ONEHUNDRED_TWENTYTHREE,
        TEAM_ONEHUNDRED_TWENTYFOUR,
        TEAM_NEUTRAL,
        TEAM_WORLD,
        TEAM_SPECTATOR,
        TEAM_INVALID,
    };

    enum AIWeaponFireType : int32_t {
        AI_WEAPON_FIRE_TYPE_FULLAUTO = 0x0,
        AI_WEAPON_FIRE_TYPE_BURST = 0x1,
        AI_WEAPON_FIRE_TYPE_SINGLE_SHOT = 0x2,
        AI_WEAPON_FIRE_TYPE_COUNT = 0x3,
        AI_WEAPON_FIRE_NUM_INVALID = 0x3,
    };
    enum AICategory : int32_t {
        AI_CATEGORY_NONE = 0x0,
        AI_CATEGORY_CIVILIAN = 0x1,
        AI_CATEGORY_ASSAULT = 0x2,
        AI_CATEGORY_CQB = 0x3,
        AI_CATEGORY_RPG = 0x4,
        AI_CATEGORY_SNIPER = 0x5,
        AI_CATEGORY_SUPPRESSOR = 0x6,
        AI_CATEGORY_RIOT_SHIELD = 0x7,
        AI_CATEGORY_COUNT = 0x8,
    };


    /*
     * Get the xasset type id from a name
     * @param name xasset name
     * @return xasset type
     */
    XAssetType XAssetIdFromName(const char* name);
    /*
     * Get the xasset type name from an id
     * @param name xasset id
     * @return xasset type name
     */
    const char* XAssetNameFromId(XAssetType id);
    /*
     * Get the bgcache type id from a name
     * @param name bgcache name
     * @return bgcache type
     */
    BGCacheTypes BGCacheIdFromName(const char* name);
    /*
     * Get the bgcache type name from an id
     * @param name bgcache id
     * @return bgcache type name
     */
    const char* BGCacheNameFromId(BGCacheTypes id);

    size_t GetAssetNameOffset(XAssetType type);
    XHash* GetAssetName(XAssetType type, void* asset, size_t size = 0);
    const char* GetTeamName(team_t team);
    team_t GetTeam(const char* name);
    const char* GetAIWeaponFireTypeName(AIWeaponFireType type);
    AIWeaponFireType GetAIWeaponFireType(const char* name);
    const char* GetAICategoryName(AICategory type);
    AICategory GetAICategory(const char* name);
}