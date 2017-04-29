/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

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