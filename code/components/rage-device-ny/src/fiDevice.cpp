/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiDevice.h"
#include "CrossLibraryInterfaces.h"
#include <Hooking.h>

namespace rage
{
static hook::cdecl_stub<fiDevice*(const char*, bool)> _fiDevice__GetDevice([]()
{
	return hook::get_pattern("51 57 8B 7C 24 0C 6A 07");
});

static hook::cdecl_stub<void(const char*)> _fiDevice__Unmount([]()
{
	return hook::get_pattern("55 8B 6C 24 08 B9 ? ? ? ? 8B C5");
});

static hook::cdecl_stub<void(const char*, fiDevice*, bool)> _fiDevice__MountGlobal([]()
{
	return hook::get_pattern("81 EC 18 01 00 00 B9 ? ? ? ? 56");
});

fiDevice* fiDevice::GetDevice(const char* path, bool allowRoot) 
{ 
	return _fiDevice__GetDevice(path, allowRoot);
	//EAXJMP(0x5ABC80); 
}

void fiDevice::Unmount(const char* rootPath)
{ 
	return _fiDevice__Unmount(rootPath);
	//EAXJMP(0x5AC080);
}

void fiDevice::MountGlobal(const char* path, fiDevice* device, bool allowRoot) 
{
	return _fiDevice__MountGlobal(path, device, allowRoot);
	//EAXJMP(0x5ABE20);
}

rage::fiDevice::~fiDevice() {}

__declspec(dllexport) fwEvent<> fiDevice::OnInitialMount;
}

static BOOL WINAPI MoveFileWithEx(LPCSTR oldFn, LPCSTR newFn)
{
	return MoveFileExA(oldFn, newFn, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
}

static HookFunction hookFunction([]()
{
	// rage::fiDeviceLocal::Rename uses MoveFile in NY, newer games use MoveFileEx with MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING
	// LevelDB VFS code somewhat depends on this ability
	hook::iat("kernel32.dll", MoveFileWithEx, "MoveFileA");
});
