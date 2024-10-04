#include <StdInc.h>
#include <Hooking.h>

#include <CrossBuildRuntime.h>

static void (*g_gameLogWriter)(const wchar_t* path);

static bool WriteGameLog(const char* path)
{
	if (g_gameLogWriter)
	{
		g_gameLogWriter(ToWide(path).c_str());
	}

	return true;
}

static HookFunction hookFunction([]
{
	if (!xbr::IsGameBuildOrGreater<2545>())
	{
		return;
	}

	g_gameLogWriter = (decltype(g_gameLogWriter))hook::get_pattern("BA 00 00 00 40 C7 44 24 20 02 00 00 00", -0x15);

	if (auto func = (void (*)(bool (*)(const char*)))GetProcAddress(GetModuleHandleW(L"CoreRT.dll"), "SetCrashLogHandler"))
	{
		func(&WriteGameLog);
	}
});
