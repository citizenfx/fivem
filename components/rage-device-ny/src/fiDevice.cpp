#include "StdInc.h"
#include "fiDevice.h"
#include "CrossLibraryInterfaces.h"

namespace rage
{
WRAPPER fiDevice* fiDevice::GetDevice(const char* path, bool allowRoot) { EAXJMP(0x5ABC80); }

WRAPPER void fiDevice::Unmount(const char* rootPath) { EAXJMP(0x5AC080); }

rage::fiDevice::~fiDevice() {}

__declspec(dllexport) fwEvent<> fiDevice::OnInitialMount;
}