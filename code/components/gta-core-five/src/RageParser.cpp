#include <StdInc.h>
#include <Hooking.h>
#include <RageParser.h>

static void** g_parser;

static hook::cdecl_stub<rage::parStructure* (void* parser, uint32_t)> _parser_getStructure([]()
{
	return hook::get_pattern("74 30 44 0F B7 41 38 33 D2", -0xB);
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
	g_parser = hook::get_address<void**>(hook::get_pattern("48 8B 0D ? ? ? ? 48 8D 54 24 48 41 B0 01", 3));
});
