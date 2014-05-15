#include "StdInc.h"
#include "CrossLibraryInterfaces.h"

IGameSpecToHooks* g_hooksDLL;

void GameSpecDLLInterface::SetHooksDLLCallback(IGameSpecToHooks* callback)
{
	g_hooksDLL = callback;

	InitFunctionBase::RunAll();
}