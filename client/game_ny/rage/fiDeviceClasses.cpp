#include "StdInc.h"
#include "fiDevice.h"

#define VTABLE_fiDeviceRelative 0xD53D0C
#define VTABLE_fiPackfile 0xEB9F84

namespace rage
{
fiDeviceRelative::fiDeviceRelative()
{
	*(uintptr_t*)this = VTABLE_fiDeviceRelative;
}

void WRAPPER fiDeviceRelative::SetPath(const char* relativeTo, bool allowRoot) { EAXJMP(0x5B43F0); }
void WRAPPER fiDeviceRelative::Mount(const char* mountPoint) { EAXJMP(0x5B4480); }

auto fiPackfile__ctor = (void(__thiscall*)(fiPackfile*))0x5BCCF0;

fiPackfile::fiPackfile()
{
	*(uintptr_t*)this = VTABLE_fiPackfile;

	fiPackfile__ctor(this);
}

void WRAPPER fiPackfile::OpenPackfile(const char* archive, bool bTrue, bool bFalse, int type) { EAXJMP(0x5BCE10); }
void WRAPPER fiPackfile::Mount(const char* mountPoint) { EAXJMP(0x5BD3A0); }
void WRAPPER fiPackfile::ClosePackfile() { EAXJMP(0x5BCD80); }
}