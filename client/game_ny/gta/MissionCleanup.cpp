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