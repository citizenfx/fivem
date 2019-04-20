#include "StdInc.h"

#include <atArray.h>
#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>

static HookFunction initFunction([]()
{
	auto location = hook::get_pattern("33 C0 0F 57 C0 ? 0D", 0);

	uint64_t* expandedRadar = hook::get_address<uint64_t*>((char*)location + 7);
	uint64_t* revealFullMap = hook::get_address<uint64_t*>((char*)location + 37);

	fx::ScriptEngine::RegisterNativeHandler("IS_BIGMAP_ACTIVE", [=](fx::ScriptContext& context)
	{
		auto result = *(uint8_t*)expandedRadar == 1;
		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_BIGMAP_FULL", [=](fx::ScriptContext& context)
	{
		auto result = *(uint8_t*)revealFullMap == 1;
		context.SetResult<bool>(result);
	});
});
