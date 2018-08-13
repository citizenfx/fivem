#include "StdInc.h"
#include "Hooking.h"

#include <mutex>

static std::unordered_map<void*, std::string> g_gamerInfoBuffer;
static std::mutex g_gamerInfoBufferMutex;

int SprintfToBuffer(char* buffer, size_t length, const char* format, const char* name)
{
	int rv = _snprintf(buffer, length, format, name);
	buffer[length - 1] = '\0';

	std::unique_lock<std::mutex> lock(g_gamerInfoBufferMutex);
	g_gamerInfoBuffer[buffer] = name;

	return rv;
}

static void(*g_origCallGfx)(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7);

static void WrapGfxCall(void* a1, char* a2, void* a3, void* a4, char* a5, void* a6, void* a7)
{
	std::unique_lock<std::mutex> lock(g_gamerInfoBufferMutex);

	// overwrite this argument
	// it's actually a GFx string, however it won't try to free
	*(const char**)(a5 + 16) = g_gamerInfoBuffer[*(char**)(a5 + 16)].c_str();

	g_origCallGfx(a1, a2, a3, a4, a5, a6, a7);
}

static HookFunction hookFunction([]()
{
	// gamer tag creation
	hook::call(hook::get_pattern("48 8D 8F ? ? 00 00 C6 87 B0 00 00 00 01 E8", 14), SprintfToBuffer);

	// gamer tag name setting
	hook::call(hook::get_pattern("48 8D 8D ? 02 00 00 4C 8B CE", 15), SprintfToBuffer);

	// gamer tag display call
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 44 88 A4 3B F2 00 00 00 8B 84 3B ? 02 00 00 48", 0);
		hook::set_call(&g_origCallGfx, location);
		hook::call(location, WrapGfxCall);
	}
});
