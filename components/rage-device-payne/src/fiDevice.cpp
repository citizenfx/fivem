#include "StdInc.h"
#include "Hooking.h"
#include "Hooking.Patterns.h"
#include "Hooking.Invoke.h"
#include "fiDevice.h"
#include "CrossLibraryInterfaces.h"

namespace rage
{
hook::cdecl_stub<rage::fiDevice*(const char*, bool)> fiDevice__GetDevice([] ()
{
	return hook::pattern("83 EC 0C 55 8B 6C 24 14 6A 07").count(1).get(0).get<void>();
});

fiDevice* fiDevice::GetDevice(const char* path, bool allowRoot) { return fiDevice__GetDevice(path, allowRoot); }

// DCEC20
hook::cdecl_stub<void(const char*)> fiDevice__Unmount([] ()
{
	return hook::pattern("EB 05 1B C0 83 D8 FF 85 C0 75 15 39").count(1).get(0).get<void>(0x59 - 0x20);
});

void fiDevice::Unmount(const char* rootPath) { fiDevice__Unmount(rootPath); }

rage::fiDevice::~fiDevice() {}

__declspec(dllexport) fwEvent<> fiDevice::OnInitialMount;
}