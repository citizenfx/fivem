#include "StdInc.h"

IGameSpecToHooks* g_hooksDLL;

void GameSpecDLLInterface::SetHooksDLLCallback(IGameSpecToHooks* callback)
{
	g_hooksDLL = callback;
}