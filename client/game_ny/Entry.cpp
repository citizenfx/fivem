#include "StdInc.h"
#include "CrossLibraryInterfaces.h"

IGameSpecToHooks* g_hooksDLL;
INetLibrary* g_netLibrary;

void GameSpecDLLInterface::SetHooksDLLCallback(IGameSpecToHooks* callback)
{
	g_hooksDLL = callback;

	InitFunctionBase::RunAll();
}

void GameSpecDLLInterface::SetNetLibrary(INetLibrary* netLibrary)
{
	g_netLibrary = netLibrary;
}