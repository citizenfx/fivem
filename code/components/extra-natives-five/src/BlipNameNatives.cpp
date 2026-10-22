/*
 * This file is part of the Cfx project - https://cfx.re/
 * See LICENSE in the root of the source tree for licensing information.
 */

#include "StdInc.h"
#include <CrossBuildRuntime.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <cstdint>
#include <cstring>

namespace
{
// GTA Legacy handles contain a 16-bit pool index and a 16-bit generation.
// The pool stores pointers; the generation is at offset 8 in a radar blip.
// Layout reference: ScriptHookVDotNet NativeMemory.GetBlipAddress (zlib).
void* ResolveBlip(void* const* pool, int count, uint32_t handle)
{
	if (!pool || count <= 0 || count > 65536 || handle == 0)
	{
		return nullptr;
	}

	const uint32_t index = handle & 0xFFFF;
	if (index >= static_cast<uint32_t>(count) || !pool[index])
	{
		return nullptr;
	}

	uint16_t generation;
	std::memcpy(&generation, static_cast<const uint8_t*>(pool[index]) + 8, sizeof(generation));
	return generation == (handle >> 16) ? pool[index] : nullptr;
}

void** g_blipPool;
int* g_blipCount;
const char* (*g_getBlipName)(void*);

bool IsAvailable()
{
	return g_blipPool && g_blipCount && g_getBlipName;
}
}

static HookFunction hookFunction([]()
{
	// Pool/count signatures are documented by ScriptHookVDotNet NativeMemory.
	// Resolve the RIP-relative globals rather than hard-coding addresses.
	// Request up to two matches so multiple returned matches are
	// rejected instead of selecting an arbitrary address.

	auto pool = hook::pattern("3B 35 ? ? ? ? 74 ? 48 81 FD").count_hint(2);
	auto count = hook::pattern("FF C6 49 83 C6 08 3B 35 ? ? ? ? 7C 9B").count_hint(2);
	// Same name function used by gta-core-five/src/PatchBlipCategories.cpp.
	auto name = hook::pattern("48 83 EC ? 33 D2 38 51 ? 75").count_hint(2);

	if (pool.size() != 1 || count.size() != 1 || name.size() != 1)
	{
		trace("[blip-names] Signature mismatch; native unavailable.\n");
		return;
	}

	g_blipPool = hook::get_address<void**>(pool.get(0).get<char>(-4));
	g_blipCount = hook::get_address<int*>(count.get(0).get<char>(8));
	g_getBlipName = reinterpret_cast<const char* (*)(void*)>(name.get(0).get<void>());
	trace("[blip-names] Name reader initialized for game build %d.\n", xbr::GetGameBuild());
});

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("IS_BLIP_DISPLAY_NAME_AVAILABLE", [](fx::ScriptContext& context)
	{
		context.SetResult<bool>(IsAvailable());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_BLIP_DISPLAY_NAME", [](fx::ScriptContext& context)
	{
		const char* result = nullptr;
		if (IsAvailable() && context.GetArgumentCount() > 0)
		{
			auto handle = context.GetArgument<uint32_t>(0);
			auto blip = ResolveBlip(g_blipPool, *g_blipCount, handle);
			// Removed blips can retain their slot and generation. Ask GTA for
			// liveness before its name function can fall back to the player blip.
			auto exists = fx::ScriptEngine::GetNativeHandler(0xA6DB27D19ECBB7DA);
			if (blip && exists && FxNativeInvoke::Invoke<bool>(exists, handle))
			{
				// Read current game state. No setter hooks, resource bridges, or cache.
				result = g_getBlipName(blip);
			}
		}
		context.SetResult<const char*>(result);
	});
});
