/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

struct ResBmInfoInt
{
	void* pad;
	uint8_t f8;
	uint8_t f9;
	uint8_t fA;
	uint8_t fB;
};

struct ResBmInfo
{
	void* pad;
	ResBmInfoInt* bm;
};

static void(*g_getBlockMap)(ResBmInfo*, void*);

void GetBlockMapWrap(ResBmInfo* info, void* bm)
{
	if (info->bm)
	{
		return g_getBlockMap(info, bm);
	}
	else
	{
		trace("tried to get a blockmap from a streaming entry without blockmap\n");
	}
}

static HookFunction hookFunction([] ()
{
	void* getBlockMapCall = hook::pattern("CC FF 50 48 48 85 C0 74 0D").count(1).get(0).get<void>(17);

	hook::set_call(&g_getBlockMap, getBlockMapCall);
	hook::call(getBlockMapCall, GetBlockMapWrap);
});