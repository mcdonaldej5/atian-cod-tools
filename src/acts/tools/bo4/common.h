#include <includes.hpp>

static auto info_response_hostname = ""s;
static auto info_response_hostxuid = ""s;

extern uintptr_t dwProcessBase;
extern uintptr_t cgArray;
extern uintptr_t cg_entitiesArray;

extern float boldness;
extern float flHue;
extern float rgb_speed;

const static auto Cbuf_AddText = reinterpret_cast<std::uintptr_t(__fastcall*)(int, const char*)>(dwProcessBase + 0x3CDF020);
const static auto Cmd_ExecuteSingleCommand = reinterpret_cast<std::uintptr_t(__fastcall*)(int, int, const char* text, bool)>(dwProcessBase + 0x3CDFC30);

typedef bool(__fastcall* dwNetadrToCommonAddrT)(netadr_t, void*, const unsigned int, bdSecurityID*);
typedef bool(__fastcall* NET_OutOfBandPrintT)(netsrc_t, netadr_t, const char*);
typedef __int64(__fastcall* Com_GetClientDObjT)(int* handle, int a2);
typedef __int64(__fastcall* CG_DObjGetWorldTagPosInternalT)(__int64 centity, __int64 DObj, int tag, float* whatever, float* pos, int something);
typedef bool(__fastcall* tCG_CanSeeFriendlyHeadTrace)(int localClientNum, __int64 centity_t, __int64 start, __int64 end);
typedef bool(__fastcall* CG_IsEntityFriendlyNotEnemyT)(int localClientNum, __int64 centity_t);
typedef char(__fastcall* draw2DInternalT)(int);
typedef void(__fastcall* CG_BulletHitEventT)(LocalClientNum_t localClientNum, int sourceEntityNum, int targetEntityNum, __int64 weapon, vec3_t* startPos, vec3_t* position, vec3_t* normal, vec3_t* seeThruDecalNormal, int surfType, int* _event, __int64 eventParam);
typedef bool(__fastcall* WorldPosToScreenPosT)(int localClientNum, float* worldPos, float* out);
typedef __int64(__fastcall* CG_PredictPlayerStateT)(unsigned int localClientNum);
typedef __int64(__fastcall* CL_WritePacketT)(unsigned int localClientNum);
typedef const char* (__fastcall* CL_GetConfigStringT)(int);
extern dwNetadrToCommonAddrT dwNetadrToCommonAddr;
extern NET_OutOfBandPrintT NET_OutOfBandPrint;
extern Com_GetClientDObjT Com_GetClientDObj;
extern CG_DObjGetWorldTagPosInternalT CG_DObjGetWorldTagPosInternal;
extern tCG_CanSeeFriendlyHeadTrace CG_CanSeeFriendlyHeadTrace;
extern CG_IsEntityFriendlyNotEnemyT CG_IsEntityFriendlyNotEnemy;
extern draw2DInternalT draw2DInternal;
extern CG_PredictPlayerStateT CG_PredictPlayerState;
extern CL_WritePacketT CL_WritePacket;
extern CG_BulletHitEventT CG_BulletHitEvent;
extern WorldPosToScreenPosT WorldPosToScreenPos;
extern CL_GetConfigStringT CL_GetConfigString;
extern DWORD_PTR nameBuffer;