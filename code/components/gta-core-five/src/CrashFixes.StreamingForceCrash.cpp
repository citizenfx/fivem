#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>

#include <CoreConsole.h>

#include "CrossBuildRuntime.h"
#include "Error.h"

static uint32_t g_pgReadRequestDeviceOffset = 0;
static uint32_t g_pgReadRequestHandleOffset = 8;

static int32_t g_getEntryFullnameOffset = 0;

unsigned int rage_CrashStreamer(hook::FlexStruct* request)
{
	if (!request)
	{
		FatalError("Streamer crashed with a null request");
		return 0;
	}

	const uint32_t handle = request->Get<uint32_t>(g_pgReadRequestHandleOffset);
	hook::FlexStruct* device = request->Get<hook::FlexStruct*>(g_pgReadRequestDeviceOffset);

	if (!device)
	{
		FatalError("Streamer crashed with wrong device");
		return 0;
	}

	char fullName[0x100 + 1] = { 0 };
	const char* requestedFileName = device->CallVirtual<const char*, const uint32_t, char*, int>(g_getEntryFullnameOffset, handle, fullName, 0x100);

	if (!requestedFileName || !*requestedFileName)
	{
		FatalError("Streamer crashed. Can't extract any useful information for debugging.");
		return 0;
	}
	FatalError("Streamer crashed on requesting %s asset file", requestedFileName);
	return 0;
}

static HookFunction hookFunction([]
{
	hook::trampoline(hook::get_pattern("48 89 5C 24 ? 55 56 57 48 83 EC ? 83 64 24 ? ? 48 8B 51"), rage_CrashStreamer);
	g_getEntryFullnameOffset = *hook::get_pattern<int32_t>("FF 90 ? ? ? ? 44 8B 8D", 2);
});
