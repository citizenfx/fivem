#include "StdInc.h"
#include "fiDevice.h"

#define VTABLE_fiDeviceRelative 0xD53D0C

namespace rage
{
fiDeviceRelative::fiDeviceRelative()
{
	*(uintptr_t*)this = VTABLE_fiDeviceRelative;
}

void WRAPPER fiDeviceRelative::setPath(const char* relativeTo, bool allowRoot) { EAXJMP(0x5B43F0); }
void WRAPPER fiDeviceRelative::mount(const char* mountPoint) { EAXJMP(0x5B4480); }
}