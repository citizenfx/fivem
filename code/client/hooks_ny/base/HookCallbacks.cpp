/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HookCallbacks.h"
#include <unordered_map>

static std::unordered_multimap<uint32_t, HookCallback_t> g_hookCallbacks;

void HookCallbacks::AddCallback(uint32_t hookCallbackId, HookCallback_t callback)
{
	g_hookCallbacks.insert({ hookCallbackId, callback });
}

void HookCallbacks::RunCallback(uint32_t hookCallbackId, void* argument)
{
	auto range = g_hookCallbacks.equal_range(hookCallbackId);

	std::for_each(range.first, range.second, [&] (std::pair<const uint32_t, HookCallback_t>& entry)
	{
		entry.second(argument);
	});
}