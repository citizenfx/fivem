#include "StdInc.h"
#include <strsafe.h>

static char g_worldDefinition[512];

void SetWorldDefinition(std::string worldDefinition)
{
	StringCbCopyA(g_worldDefinition, sizeof(g_worldDefinition), worldDefinition.c_str());

	hook::put(0x41FDC9, g_worldDefinition);
}