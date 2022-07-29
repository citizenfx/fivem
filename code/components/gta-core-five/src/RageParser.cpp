#include <StdInc.h>
#include <Hooking.h>
#include <RageParser.h>

static void** g_parser;

static hook::cdecl_stub<rage::parStructure* (void* parser, uint32_t)> _parser_getStructure([]()
{
	return hook::get_call(hook::get_pattern("8B 10 E8 ? ? ? ? 4C 8D 5C 24", 2));
});

namespace rage
{
	DLL_EXPORT rage::parStructure* GetStructureDefinition(const char* structType)
	{
		return _parser_getStructure(*g_parser, HashRageString(structType));
	}

	DLL_EXPORT rage::parStructure* GetStructureDefinition(uint32_t structHash)
	{
		return _parser_getStructure(*g_parser, structHash);
	}
}

static HookFunction hookFunction([]()
{
	g_parser = hook::get_address<void**>(hook::get_pattern("45 33 C9 E8 ? ? ? ? 48 8B F0 48 85 C0 74", -18));
});
