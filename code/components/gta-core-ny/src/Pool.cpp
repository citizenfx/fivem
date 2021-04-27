/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Pool.h"
#include <Hooking.h>

CPool** CPools::ms_pBuildingPool;// = *(CPool**)0x168FED0;
CPool** CPools::ms_pQuadTreePool;// = *(CPool**)0x13504D0;

static hook::thiscall_stub<void*(CPool*)> _allocate([]()
{
	return hook::get_pattern("53 8B D1 56 8B 42 10");
});

void* CPool::Allocate() 
{ 
	return _allocate(this);
	//EAXJMP(0x96D520); 
}

static HookFunction hookFunc([]()
{
	CPools::ms_pBuildingPool = *hook::get_pattern<CPool**>("33 FF C6 44 24 17 01", -24);
	CPools::ms_pQuadTreePool = *hook::get_pattern<CPool**>("68 20 03 00 00 8B C8", 13);
});
