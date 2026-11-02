#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

static int64_t (*g_origConstructFromNodeList)(int64_t followRoute, int64_t vehicle, int64_t nodeList, char a4, int64_t a5, char a6);
static int64_t ConstructFromNodeList(int64_t followRoute, int64_t vehicle, int64_t nodeList, char a4, int64_t a5, char a6)
{
	if (!vehicle)
	{
		return 0;
	}

	return g_origConstructFromNodeList(followRoute, vehicle, nodeList, a4, a5, a6);
}

static HookFunction hookFunction([]()
{
	g_origConstructFromNodeList = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 44 88 48 ? 48 89 50 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8"), ConstructFromNodeList);
});
