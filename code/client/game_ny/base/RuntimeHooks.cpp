#include "StdInc.h"
#include "RuntimeHooks.h"
#include "CrossLibraryInterfaces.h"

bool RuntimeHooks::InstallRuntimeHook(const char* key)
{
	return g_hooksDLL->InstallRuntimeHook(key);
}

bool RuntimeHooks::SetLimit(const char* limit, int value)
{
	return g_hooksDLL->SetLimit(limit, value);
}

bool RuntimeHooks::SetWorldDefinition(const char* worldDefinition)
{
	return g_hooksDLL->SetWorldDefinition(worldDefinition);
}