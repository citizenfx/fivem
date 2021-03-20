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

StreamingFile::~StreamingFile()
{

}

CStreamingInfoManager* CStreamingInfoManager::GetInstance()
{
	static auto inst = *hook::get_pattern<CStreamingInfoManager*>("C7 47 04 00 00 00 00 C7 04 24 66 66 66 3F E8", 34);
	return (CStreamingInfoManager*)0xF21C60;
}

static int g_curHandle = 0;
static StreamingFile* g_files[65535];
static StreamingModule* g_streamingModule;

void CStreaming::SetStreamingModule(StreamingModule* module)
{
	g_streamingModule = module;
}

void CStreaming::ScanImgEntries()
{
	if (g_streamingModule) g_streamingModule->ScanEntries();
}

uint32_t CStreaming::OpenImgEntry(uint32_t handle)
{
	if (!g_streamingModule)
	{
		return 0;
	}

	auto file = g_streamingModule->GetEntryFromIndex(handle);

	if (!file)
	{
		FatalError("Unknown streaming file %d from streaming module", handle);
	}

	file->Open();

	auto increment = [&] ()
	{
		g_curHandle++;

		if (g_curHandle >= _countof(g_files))
		{
			g_curHandle = 0;
		}
	};

	increment();

	// find if it's free
	while (g_files[g_curHandle])
	{
		increment();
	}

	int outHandle = g_curHandle;

	g_files[outHandle] = file;

	return outHandle;
}

StreamingFile* CStreaming::GetImgEntry(uint32_t handle)
{
	if (!g_files[handle])
	{
		FatalError("Undefined streaming file for handle 0x%08x.", handle);
	}

	return g_files[handle];
}

uint32_t CStreaming::CloseImgEntry(uint32_t handle)
{
	g_files[handle]->Close();
	g_files[handle] = nullptr;

	return -1;
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
