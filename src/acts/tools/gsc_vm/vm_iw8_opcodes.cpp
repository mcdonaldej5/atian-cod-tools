#include <includes.hpp>
#include <tools/gsc_vm.hpp>
#include <tools/gsc.hpp>
#include <tools/gsc_opcodes.hpp>

namespace {
	enum ScriptOpcode : byte {
		OP_CastFieldObject = 0x0,
		OP_SetLocalVariableFieldCached = 0x1,
		OP_plus = 0x2,
		OP_RemoveLocalVariables = 0x3,
		OP_EvalSelfFieldVariableRef = 0x4,
		OP_ScriptFarMethodChildThreadCall = 0x5,
		OP_GetGameRef = 0x6,
		OP_EvalAnimFieldVariable = 0x7,
		OP_EvalLevelFieldVariableRef = 0x8,
		OP_GetThisthread = 0x9,
		OP_greater = 0xA,
		OP_waittillmatch = 0xB,
		OP_shift_right = 0xC,
		OP_dec = 0xD,
		OP_JumpOnTrue = 0xE,
		OP_bit_or = 0xF,
		OP_equality = 0x10,
		OP_ClearLocalVariableFieldCached0 = 0x11,
		OP_notify = 0x12,
		OP_GetVector = 0x13,
		OP_ScriptMethodChildThreadCallPointer = 0x14,
		OP_PreScriptCall = 0x15,
		OP_GetByte = 0x16,
		OP_ScriptFarThreadCall = 0x17,
		OP_SetSelfFieldVariableField = 0x18,
		OP_JumpOnFalseExpr = 0x19,
		OP_GetUndefined = 0x1A,
		OP_jumpback = 0x1B,
		OP_JumpOnTrueExpr = 0x1C,
		OP_CallBuiltin0 = 0x1D,
		OP_CallBuiltin1 = 0x1E,
		OP_CallBuiltin2 = 0x1F,
		OP_CallBuiltin3 = 0x20,
		OP_CallBuiltin4 = 0x21,
		OP_CallBuiltin5 = 0x22,
		OP_CallBuiltin = 0x23,
		OP_SetLocalVariableFieldCached0 = 0x24,
		OP_ClearFieldVariable = 0x25,
		OP_GetLevel = 0x26,
		OP_size = 0x27,
		OP_SafeSetWaittillVariableFieldCached = 0x28,
		OP_ScriptLocalMethodThreadCall = 0x29,
		OP_AddArray = 0x2A,
		OP_endon = 0x2B,
		OP_EvalFieldVariable = 0x2C,
		OP_shift_left = 0x2D,
		OP_EvalLocalArrayRefCached0 = 0x2E,
		OP_Return = 0x2F,
		OP_CreateLocalVariable = 0x30,
		OP_SafeSetVariableFieldCached0 = 0x31,
		OP_GetBuiltinFunction = 0x32,
		OP_ScriptLocalMethodCall = 0x33,
		OP_CallBuiltinMethodPointer = 0x34,
		OP_ScriptLocalChildThreadCall = 0x35,
		OP_GetSelfObject = 0x36,
		OP_GetGame = 0x37,
		OP_SetLevelFieldVariableField = 0x38,
		OP_EvalArray = 0x39,
		OP_GetSelf = 0x3A,
		OP_End = 0x3B,
		OP_EvalSelfFieldVariable = 0x3C,
		OP_less_equal = 0x3D,
		OP_EvalLocalVariableCached0 = 0x3E,
		OP_EvalLocalVariableCached1 = 0x3F,
		OP_EvalLocalVariableCached2 = 0x40,
		OP_EvalLocalVariableCached3 = 0x41,
		OP_EvalLocalVariableCached4 = 0x42,
		OP_EvalLocalVariableCached5 = 0x43,
		OP_EvalLocalVariableCached = 0x44,
		OP_EvalNewLocalArrayRefCached0 = 0x45,
		OP_ScriptChildThreadCallPointer = 0x46,
		OP_EvalLocalVariableObjectCached = 0x47,
		OP_ScriptLocalThreadCall = 0x48,
		OP_GetInteger = 0x49,
		OP_ScriptMethodCallPointer = 0x4A,
		OP_checkclearparams = 0x4B,
		OP_SetAnimFieldVariableField = 0x4C,
		OP_waittillmatch2 = 0x4D,
		OP_minus = 0x4E,
		OP_ScriptLocalFunctionCall2 = 0x4F,
		OP_GetNegUnsignedShort = 0x50,
		OP_GetNegByte = 0x51,
		OP_SafeCreateVariableFieldCached = 0x52,
		OP_greater_equal = 0x53,
		OP_vector = 0x54,
		OP_GetBuiltinMethod = 0x55,
		OP_endswitch = 0x56,
		OP_ClearArray = 0x57,
		OP_DecTop = 0x58,
		OP_CastBool = 0x59,
		OP_EvalArrayRef = 0x5A,
		OP_SetNewLocalVariableFieldCached0 = 0x5B,
		OP_GetZero = 0x5C,
		OP_wait = 0x5D,
		OP_waittill = 0x5E,
		OP_GetIString = 0x5F,
		OP_ScriptFarFunctionCall = 0x60,
		OP_GetAnimObject = 0x61,
		OP_GetAnimTree = 0x62,
		OP_EvalLocalArrayCached = 0x63,
		OP_mod = 0x64,
		OP_ScriptFarMethodThreadCall = 0x65,
		OP_GetUnsignedShort = 0x66,
		OP_clearparams = 0x67,
		OP_ScriptMethodThreadCallPointer = 0x68,
		OP_ScriptFunctionCallPointer = 0x69,
		OP_EmptyArray = 0x6A,
		OP_SafeSetVariableFieldCached = 0x6B,
		OP_ClearVariableField = 0x6C,
		OP_EvalFieldVariableRef = 0x6D,
		OP_ScriptLocalMethodChildThreadCall = 0x6E,
		OP_EvalNewLocalVariableRefCached0 = 0x6F,
		OP_GetFloat = 0x70,
		OP_EvalLocalVariableRefCached = 0x71,
		OP_JumpOnFalse = 0x72,
		OP_BoolComplement = 0x73,
		OP_ScriptThreadCallPointer = 0x74,
		OP_ScriptFarFunctionCall2 = 0x75,
		OP_less = 0x76,
		OP_BoolNot = 0x77,
		OP_waittillFrameEnd = 0x78,
		OP_waitframe = 0x79,
		OP_GetString = 0x7A,
		OP_EvalLevelFieldVariable = 0x7B,
		OP_GetLevelObject = 0x7C,
		OP_inc = 0x7D,
		OP_CallBuiltinMethod0 = 0x7E,
		OP_CallBuiltinMethod1 = 0x7F,
		OP_CallBuiltinMethod2 = 0x80,
		OP_CallBuiltinMethod3 = 0x81,
		OP_CallBuiltinMethod4 = 0x82,
		OP_CallBuiltinMethod5 = 0x83,
		OP_CallBuiltinMethod = 0x84,
		OP_GetAnim = 0x85,
		OP_switch = 0x86,
		OP_SetVariableField = 0x87,
		OP_divide = 0x88,
		OP_GetLocalFunction = 0x89,
		OP_ScriptFarChildThreadCall = 0x8A,
		OP_multiply = 0x8B,
		OP_ClearLocalVariableFieldCached = 0x8C,
		OP_EvalAnimFieldVariableRef = 0x8D,
		OP_EvalLocalArrayRefCached = 0x8E,
		OP_EvalLocalVariableRefCached0 = 0x8F,
		OP_bit_and = 0x90,
		OP_GetAnimation = 0x91,
		OP_GetFarFunction = 0x92,
		OP_CallBuiltinPointer = 0x93,
		OP_jump = 0x94,
		OP_voidCodepos = 0x95,
		OP_ScriptFarMethodCall = 0x96,
		OP_inequality = 0x97,
		OP_ScriptLocalFunctionCall = 0x98,
		OP_bit_ex_or = 0x99,
		OP_NOP = 0x9A,
		OP_abort = 0x9B,
		OP_object = 0x9C,
		OP_thread_object = 0x9D,
		OP_EvalLocalVariable = 0x9E,
		OP_EvalLocalVariableRef = 0x9F,
		OP_prof_begin = 0xA0,
		OP_prof_end = 0xA1,
		OP_breakpoint = 0xA2,
		OP_assignmentBreakpoint = 0xA3,
		OP_manualAndAssignmentBreakpoint = 0xA4,
		OP_BoolNotAfterAnd = 0xA5,
		OP_FormalParams = 0xA6,
		OP_IsDefined = 0xA7,
		OP_IsTrue = 0xA8,
		OP_NativeGetLocalFunction = 0xA9,
		OP_NativeLocalFunctionCall = 0xAA,
		OP_NativeLocalFunctionCall2 = 0xAB,
		OP_NativeLocalMethodCall = 0xAC,
		OP_NativeLocalFunctionThreadCall = 0xAD,
		OP_NativeLocalMethodThreadCall = 0xAE,
		OP_NativeLocalFunctionChildThreadCall = 0xAF,
		OP_NativeLocalMethodChildThreadCall = 0xB0,
		OP_NativeGetFarFunction = 0xB1,
		OP_NativeFarFunctionCall = 0xB2,
		OP_NativeFarFunctionCall2 = 0xB3,
		OP_NativeFarMethodCall = 0xB4,
		OP_NativeFarFunctionThreadCall = 0xB5,
		OP_NativeFarMethodThreadCall = 0xB6,
		OP_NativeFarFunctionChildThreadCall = 0xB7,
		OP_NativeFarMethodChildThreadCall = 0xB8,
		OP_EvalNewLocalArrayRefCached0_Precompiled = 0xB9,
		OP_SetNewLocalVariableFieldCached0_Precompiled = 0xBA,
		OP_CreateLocalVariable_Precompiled = 0xBB,
		OP_SafeCreateVariableFieldCached_Precompiled = 0xBC,
		OP_FormalParams_Precompiled = 0xBD,
		OP_count = 0xBE,
	};


	using namespace tool::gsc::opcode;
	void OpCode() {
		VmInfo* gscbin = RegisterVM(VMI_IW_BIN_MW19, "Call of Duty: Modern Warfare (2019)", "iw8", "mw19", VmFlags::VMF_GSCBIN | VmFlags::VMF_NO_MAGIC | VmFlags::VMF_VAR_ID | VmFlags::VMF_NO_PARAM_FLAGS | VmFlags::VMF_IW_LIKE | VmFlags::VMF_IW_CALLS | VmFlags::VMF_CALL_NO_PARAMS);
		gscbin->AddPlatform(PLATFORM_PC);
		// GetOpaqueStringCount / Scr_InitStringConstants =  useReducedSpGoldLimits ? 0xE2C0 : 0x1472F 
		gscbin->SetOpaqueStringCount(0x1472F);
		// VM_Execute: sub_37EA880, seems to be similar to jup without devblockbegin for start
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_CastFieldObject, 0x00);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_SetLocalVariableCached, 0x01);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Plus, 0x02);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetThread, 0x09);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GreaterThan, 0x0A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_ShiftRight, 0x0C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetVector, 0x13);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetByte, 0x16);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_ShiftLeft, 0x2D);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_End, 0x3B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EvalLocalVariableCached0, 0x3E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EvalLocalVariableCached1, 0x3F);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EvalLocalVariableCached2, 0x40);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EvalLocalVariableCached3, 0x41);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EvalLocalVariableCached4, 0x42);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EvalLocalVariableCached5, 0x43);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_EvalLocalVariableCached, 0x44);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetInteger, 0x49);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_CheckClearParams, 0x4B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Minus, 0x4E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetNegByte, 0x51);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Vector, 0x54);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetAnimationTree, 0x62);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Modulus, 0x64);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_BoolComplement, 0x73);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Divide, 0x88);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Multiply, 0x8B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Bit_And, 0x90);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IsDefined, 0xA7);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_IsTrue, 0xA8);

		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x03);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x04);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x05);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetGameRef, 0x06);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x07);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x08);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_2, 0x0B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x0D);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_JumpOnTrue, 0x0E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x0F);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Equal, 0x10);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x11);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x12);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x14);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x15);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Undefined, 0x17);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x18);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_JumpOnFalseExpr, 0x19);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x1A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_2, 0x1B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_JumpOnTrueExpr, 0x1C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinFunction0, 0x1D);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinFunction1, 0x1E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinFunction2, 0x1F);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinFunction3, 0x20);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinFunction4, 0x21);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinFunction5, 0x22);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_CallBuiltinFunction, 0x23);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x24);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x25);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetLevel, 0x26);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x27);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x28);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_4, 0x29);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x2A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x2B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x2C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x2E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Return, 0x2F);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x30);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x31);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_2, 0x32);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_3, 0x33);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x34);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_4, 0x35);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x36);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetGame, 0x37);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x38);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_EvalArray, 0x39);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetSelf, 0x3A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x3C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_LessThanOrEqualTo, 0x3D);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x45);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x46);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_EvalLocalVariableRefCached, 0x47);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_LocalThreadCall, OP_ScriptLocalThreadCall);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x4A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x4C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetNegUnsignedShort, 0x50);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x52);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x53);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_2, 0x55);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EndSwitch, 0x56);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x57);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_DecTop, 0x58);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_CastBool, OP_CastBool);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_EvalArrayRef, 0x5A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x5B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetZero, 0x5C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Wait, 0x5D);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_SingleWaitTill, 0x5E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetIString, 0x5F);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Undefined, 0x60);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetAnimGRef, 0x61);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_EvalLocalArrayCached, 0x63);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Undefined, 0x65);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetUnsignedShort, 0x66);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_ClearParams, 0x67);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x68);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x69);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_EmptyArray, 0x6A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x6B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x6D);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_4, 0x6E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetFloat, 0x70);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x71);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_JumpOnFalse, 0x72);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x74);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_ScriptFunctionCall2, 0x75);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_LessThan, 0x76);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_BoolNot, 0x77);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_WaitTillFrameEnd, 0x78);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_WaitFrame, 0x79);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GetString, 0x7A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x7B);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetLevelGRef, 0x7C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Inc, OP_inc);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinMethod0, 0x7E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinMethod1, 0x7F);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinMethod2, 0x80);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinMethod3, 0x81);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinMethod4, 0x82);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_CallBuiltinMethod5, 0x83);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_CallBuiltinMethod, 0x84);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x85);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_4, 0x86);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_SetVariableField, 0x87);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Undefined, 0x8A);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x8C);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_STR_TOKEN, 0x8D);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x8E);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x8F);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetAnimation, 0x91);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Undefined, 0x92);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0x93);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_Jump32, OP_jump);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x95);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_ScriptMethodCall, OP_ScriptFarMethodCall);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_NotEqual, OP_inequality);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0x99);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Nop, OP_NOP);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_Abort, OP_abort);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_5, 0xA0);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0xA1);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_0, 0xA5);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_SafeCreateLocalVariables, OP_FormalParams);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_3, 0xA9);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_3, 0xAA);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_3, 0xAB);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_3, 0xAC);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_4, 0xAD);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_4, 0xB0);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0xB9);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0xBA);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0xBB);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_1, 0xBC);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_GSCBIN_SKIP_N, 0xBD);

		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_GetLocal, OP_GetLocalFunction);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_LocalCall, OP_ScriptLocalFunctionCall);
		gscbin->RegisterOpCode(PLATFORM_PC, OPCODE_IW_LocalCall2, OP_ScriptLocalFunctionCall2);


	}




}
REGISTER_GSC_VM_OPCODES(gscbin, OpCode);