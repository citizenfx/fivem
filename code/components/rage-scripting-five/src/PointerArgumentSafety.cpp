#include <StdInc.h>

#include "PointerArgumentHints.h"
#include "scrEngine.h"

#include <CL2LaunchMode.h>
#include <ICoreGameInit.h>

#include <psapi.h>

bool storyMode;

namespace rage
{
extern uint64_t MapNative(uint64_t inNative);
}

static std::unordered_map<uint64_t, const uint32_t*> g_nativeTypes;

static void RegisterNativeTypeInfo(uint64_t hash, const uint32_t* typeInfo)
{
	g_nativeTypes[rage::MapNative(hash)] = typeInfo;
}

namespace fx::scripting
{

const uint32_t* fx::scripting::GetNativeTypeInfo(uint64_t hash)
{
	hash = rage::MapNative(hash);

	if (auto find = g_nativeTypes.find(hash); find != g_nativeTypes.end())
	{
		return find->second;
	}

	return nullptr;
}

}

#if __has_include("PASPriv.h")
#include "PASPriv.h"
#endif

#include "PASGen.h"

void PointerArgumentSafety()
{
	PointerArgumentSafety_Impl();
}

static HookFunction hookFunction([]
{
	Instance<ICoreGameInit>::Get()->OnSetVariable.Connect([](const std::string& name, bool value)
	{
		if (name == "storyMode")
		{
			storyMode = value;
		}
	});
});
