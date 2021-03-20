#include "StdInc.h"
#include <strsafe.h>

static char g_worldDefinition[512];

void SetWorldDefinition(std::string worldDefinition)
{
	StringCbCopyA(g_worldDefinition, sizeof(g_worldDefinition), worldDefinition.c_str());

	hook::put(hook::pattern("E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 04 E8").count(2).get(1).get<void*>(16), g_worldDefinition);
}
