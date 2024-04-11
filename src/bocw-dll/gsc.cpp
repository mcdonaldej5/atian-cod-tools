#include <dll_includes.hpp>
#include <hook/library.hpp>
#include <utils.hpp>
#include <scriptinstance.hpp>
#include <core/system.hpp>
#include "cw.hpp"


namespace {

	cw::ObjFileInfoStruct* gObjFileInfo;
	uint32_t* gObjFileInfoCount;
	void** gVmOpJumpTable;

	byte* FindExport(scriptinstance::ScriptInstance inst, uint64_t target_script, uint32_t name_space, uint32_t name) {

		cw::ObjFileInfo* end = gObjFileInfo[inst] + gObjFileInfoCount[inst];

		auto its = std::find_if(gObjFileInfo[inst], end,
			[target_script](const cw::ObjFileInfo& info) { return info.activeVersion->name == target_script; });

		if (its == end) {
			return nullptr;
		}

		cw::T9GSCOBJ* obj = its->activeVersion;

		auto exportEnd = reinterpret_cast<cw::T8GSCExport*>(obj->magic + obj->exports_tables) + obj->exports_count;
		auto ite = std::find_if(reinterpret_cast<cw::T8GSCExport*>(obj->magic + obj->exports_tables), exportEnd,
			[name_space, name](const cw::T8GSCExport& exp) { return exp.name == name && exp.name_space == name_space; });

		if (ite == exportEnd) {
			return nullptr;
		}

		return obj->magic + ite->address;
	}

	void VM_OP_LazyLink_Handler(scriptinstance::ScriptInstance inst, void* varInfo, cw::FunctionStack* fs_0, void* ctx, bool* terminate) {
		struct LazyLinkData {
			uint32_t nameSpace;
			uint32_t name;
			uint64_t script;
		};
		LazyLinkData* data = (LazyLinkData*)utils::Aligned<uint32_t>(fs_0->pos);

		fs_0->pos = (byte*)(data + 1);

		byte* exp = FindExport(inst, data->script, data->nameSpace, data->name);

		fs_0->top++;

		if (exp) {
			fs_0->top->type = cw::TYPE_SCRIPT_FUNCTION;
			fs_0->top->u.codePosValue = exp;
		}
		else {
			fs_0->top->type = cw::TYPE_UNDEFINED;
			fs_0->top->u.intValue = 0;
		}
	}

	void PostInit(uint64_t id) {
		// add lazylink operator

		// gVmOpJumpTable[]
		hook::library::Library main{};

		auto table = hook::library::QueryScanContainerSingle("gVmOpJumpTable", "41 FF 94 FC ? ? ? ? 80 7C 24");
		auto objFile = hook::library::QueryScanContainerSingle("gObjFileInfo", "4C 8D 2D ? ? ? ? 48 8D 15 ? ? ? ? 43 39 4C B5 00"); // count / table

		gObjFileInfoCount = objFile.GetRelative<int32_t, uint32_t*>(3);
		gObjFileInfo = objFile.GetRelative<int32_t, cw::ObjFileInfoStruct*>(10);
		gVmOpJumpTable = reinterpret_cast<void**>(process::Relativise(table.Get<int32_t>(4)));

		gVmOpJumpTable[0x13] = VM_OP_LazyLink_Handler;
	}

}
REGISTER_SYSTEM(gsc, nullptr, PostInit);