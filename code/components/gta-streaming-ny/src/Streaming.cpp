/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Streaming.h"

#include <Error.h>
#include <Hooking.h>

//uint32_t WRAPPER CStreamingInfoManager::registerIMGFile(const char* name, uint32_t offset, uint32_t size, uint8_t imgNum, uint32_t index, uint32_t resourceType) { EAXJMP(0xBCC2E0); }

CStreamingInfoManager* CStreamingInfoManager::GetInstance()
{
	static auto inst = *hook::get_pattern<CStreamingInfoManager*>("C7 47 04 00 00 00 00 C7 04 24 66 66 66 3F E8", 34);
	return inst;
}

StreamingItem** g_streamingItems;
int* g_nextStreamingItem;
uint32_t* g_streamMask;

static HookFunction hookFunc([]()
{
	g_streamingItems = *hook::get_pattern<StreamingItem**>("56 FF ? 83 C7 04", 16);
	g_nextStreamingItem = *hook::get_pattern<int*>("68 00 02 00 00 8d 44 24 1c", -38);
	g_streamMask = *hook::get_pattern<uint32_t*>("89 4C 24 24 83 F9 FF 0F 84 ? ? ? ? A1", 14);

	// increase allowed amount of filesystem mounts (fixes CreatePlayer exception @ 0x69ECB2 due to player:/ not being mounted)
	hook::put(hook::get_pattern("C7 05 ? ? ? ? 40 00 00 00 E8", 6), 256); // was 64
});
