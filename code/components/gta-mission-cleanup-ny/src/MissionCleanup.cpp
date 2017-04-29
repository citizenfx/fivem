/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MissionCleanup.h"

#define VTABLE_CMissionCleanup 0xD735D8

CMissionCleanup::CMissionCleanup()
{
	*(uintptr_t*)this = VTABLE_CMissionCleanup;

	Initialize();
}

#define PURECALL() __asm { jmp _purecall }

WRAPPER CMissionCleanup::~CMissionCleanup()
{
	PURECALL();
}

void WRAPPER CMissionCleanup::CleanUp(GtaThread* scriptThread) { PURECALL(); }

// skip any early jump placed on here
void WRAPPER CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects()
{
	__asm
	{
		sub esp, 8
		push ebx
		push ebp
	}

	EAXJMP(0x9282D5);
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