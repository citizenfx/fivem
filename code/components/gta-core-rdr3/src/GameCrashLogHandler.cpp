#include <StdInc.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

struct LogBuffer
{
	char* buffer;
	size_t length;
	size_t offset;
};

static char logData[0x80000];
static LogBuffer logBuffer{
	&logData[0],
	std::size(logData),
	0
};

static void (*g_gameLogWriter)(LogBuffer* buffer);

static bool WriteGameLog(const char* path)
{
	if (g_gameLogWriter)
	{
		g_gameLogWriter(&logBuffer);

		FILE* f = _wfopen(ToWide(path).c_str(), L"wb");
		if (f)
		{
			fwrite(logBuffer.buffer, 1, logBuffer.offset, f);
			fclose(f);
		}
	}

	return true;
}

static HookFunction hookFunction([]
{
	if (!xbr::IsGameBuildOrGreater<1311>())
	{
		return;
	}

	g_gameLogWriter = (decltype(g_gameLogWriter))hook::get_pattern("48 8B D9 E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 48 8D 15", -0xD);

	if (auto func = (void (*)(bool (*)(const char*)))GetProcAddress(GetModuleHandleW(L"CoreRT.dll"), "SetCrashLogHandler"))
	{
		func(&WriteGameLog);
	}
});
