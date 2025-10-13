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

struct pgReadData_pgReadRequest
{
	hook::FlexStruct* m_Device;
	void* m_Handle;
	unsigned __int64 m_Offset;
	char* m_Dest;
	int m_BufferIndex;
	int m_Count;
	unsigned int m_SortKey;
	bool m_Uncached;
	bool m_CloseWhenFinished;
};
static_assert(sizeof(pgReadData_pgReadRequest) == 0x30, "pgReadData_pgReadRequest size is wrong");

static int32_t g_getEntryFullnameOffset = 0;

unsigned int (*rage_CrashStreamer_Orig)(pgReadData_pgReadRequest* request);
unsigned int rage_CrashStreamer(pgReadData_pgReadRequest* request)
{
	if (!request)
	{
		FatalError("Streamer crashed with a null request");
		return 0;
	}

	unsigned int streamingIndex = rage_CrashStreamer_Orig(request);

	hook::FlexStruct* device = request->m_Device;

	if (!device)
	{
		FatalError("Streamer crashed with wrong device");
		return 0;
	}

	char fullName[0x100 + 1] = { 0 };
	device->CallVirtual<const char*, const void*, char*, int>(g_getEntryFullnameOffset, request->m_Handle, fullName, sizeof(fullName));

	const std::string errorMessage = fmt::format("m_Handle = {:x}\nm_Offset = {:x}\nm_Dest = {:x}\nm_BufferIndex = {}\nm_Count = {}\nm_SortKey = {}\nm_Uncached = {}\nm_CloseWhenFinished = {}\nstreamingIndex = {}",
		request->m_Handle, request->m_Offset, request->m_Dest, request->m_BufferIndex,
		request->m_Count, request->m_SortKey, request->m_Uncached ? "true" : "false",
		request->m_CloseWhenFinished ? "true" : "false", streamingIndex);

	if (fullName[0] == '\0')
	{
		FatalError("Streamer crashed. Can't extract any useful information for debugging.\n\n%s", errorMessage.c_str());
		return 0;
	}

	FatalError("Streamer crashed on requesting %s asset file.\n\n%s", fullName, errorMessage.c_str());
	return 0;
}

static HookFunction hookFunction([]
{
	rage_CrashStreamer_Orig = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 55 56 57 48 83 EC ? 83 64 24 ? ? 48 8B 51"), rage_CrashStreamer);
	g_getEntryFullnameOffset = *hook::get_pattern<int32_t>("FF 90 ? ? ? ? 44 8B 8D", 2);
});
