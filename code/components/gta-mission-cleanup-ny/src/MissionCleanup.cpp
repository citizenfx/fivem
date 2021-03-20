/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MissionCleanup.h"
#include <Hooking.h>

CMissionCleanup::CMissionCleanup()
{
	static auto vtbl = *hook::get_pattern<uintptr_t>("8B 01 6A 10 68 64 4E 00 00", 27);
	*(uintptr_t*)this = vtbl;

	Initialize();
}

#define PURECALL() __asm { jmp _purecall }

WRAPPER CMissionCleanup::~CMissionCleanup()
{
	PURECALL();
}

void WRAPPER CMissionCleanup::CleanUp(GtaThread* scriptThread) 
{
	PURECALL(); 
}

// skip any early jump placed on here
extern int(__fastcall* g_origCheckIfCollisionHasLoadedForMissionObjects)(CMissionCleanup* a1);

void CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects()
{
	g_origCheckIfCollisionHasLoadedForMissionObjects(this);
}

void CMissionCleanup::Initialize()
{
	
}

CMissionCleanupEntry::CMissionCleanupEntry()
{
	Reset();
}

void CMissionCleanupEntry::Reset()
{
	memset(this, 0, sizeof(*this));
	m_data = -1;
}
