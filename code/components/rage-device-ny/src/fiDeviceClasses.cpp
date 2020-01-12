/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiDevice.h"
#include <Hooking.h>

namespace rage
{
static hook::thiscall_stub<void(fiDeviceRelative*, const char*, bool)> _fiDeviceRelative__SetPath([]()
{
	return hook::get_pattern("81 EC 00 02 00 00 8D 04 24 56 57 6A");
});

static hook::thiscall_stub<void(fiDeviceRelative*, const char*)> _fiDeviceRelative__Mount([]()
{
	return hook::get_pattern("56 8B 74 24 08 57 8B F9 0F B6 87");
});

fiDeviceRelative::fiDeviceRelative()
{
	static auto loc = *(uintptr_t*)(hook::get_pattern<char>("C7 05 ? ? ? ? 00 00 00 00 E8 ? ? ? ? 83 C4 04 6A 01", -4));
	*(uintptr_t*)this = loc;
}

void fiDeviceRelative::SetPath(const char* relativeTo, bool allowRoot) 
{
	_fiDeviceRelative__SetPath(this, relativeTo, allowRoot);
}

void fiDeviceRelative::Mount(const char* mountPoint) 
{ 
	_fiDeviceRelative__Mount(this, mountPoint);
}



static hook::thiscall_stub<void(fiPackfile*)> _fiPackfile__ctor([]()
{
	return hook::get_pattern("C7 41 08 00 00 00 00 C7 41 0C FF FF FF FF C7 41 10 00", -13);
});

static hook::thiscall_stub<void(fiPackfile*, const char*, bool, bool, int)> _fiPackfile__Open([]()
{
	return hook::get_pattern("55 8B EC 83 E4 F0 81 EC 28 0C 00 00");
});

static hook::thiscall_stub<void(fiPackfile*, const char*)> _fiPackfile__Mount([]()
{
	return hook::get_pattern("56 8B 74 24 08 57 8B F9 6A 01 57 56");
});

static hook::thiscall_stub<void(fiPackfile*)> _fiPackfile__Close([]()
{
	return hook::get_pattern("56 8B F1 57 8B 96 70 04 00 00");
});

fiPackfile::fiPackfile()
{
	static auto loc = *(uintptr_t*)(hook::get_pattern<char>("C7 41 08 00 00 00 00 C7 41 0C FF FF FF FF C7 41 10 00", -11));
	*(uintptr_t*)this = loc;

	_fiPackfile__ctor(this);
}

void fiPackfile::OpenPackfile(const char* archive, bool bTrue, bool bFalse, int type) 
{
	_fiPackfile__Open(this, archive, bTrue, bFalse, type);
}

void fiPackfile::Mount(const char* mountPoint) 
{
	_fiPackfile__Mount(this, mountPoint);
}

void fiPackfile::ClosePackfile() 
{ 
	_fiPackfile__Close(this);
}
}
