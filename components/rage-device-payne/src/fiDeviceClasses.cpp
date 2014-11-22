#include "StdInc.h"
#include "fiDevice.h"
#include "Hooking.h"

#define VTABLE_fiDeviceRelative 0xD53D0C
#define VTABLE_fiPackfile 0xEB9F84

namespace rage
{
static uintptr_t g_vTable_fiDeviceRelative;
static uintptr_t g_vTable_fiPackfile;

fiDeviceRelative::fiDeviceRelative()
{
	*(uintptr_t*)this = VTABLE_fiDeviceRelative;

	this->m_pad[256] = '\0';
}

hook::thiscall_stub<void(fiDeviceRelative*, const char*, bool, bool)> fiDeviceRelative__setPath([] ()
{
	return hook::pattern("8B F9 68 00 01 00 00 8D 4C 24 14").count(1).get(0).get<void>(-0x13);
});

void fiDeviceRelative::setPath(const char* relativeTo, bool onlyPhysical, bool allowRoot)
{
	return fiDeviceRelative__setPath(this, relativeTo, onlyPhysical, allowRoot);
}

hook::thiscall_stub<void(fiDeviceRelative*, const char*)> fiDeviceRelative__mount([] ()
{
	return hook::pattern("8B F9 0F B6 87 0C 01 00 00").count(1).get(0).get<void>(-6);
});

void fiDeviceRelative::mount(const char* mountPoint)
{ 
	return fiDeviceRelative__mount(this, mountPoint);
}

//// ---- fiPackfile ---- ////
hook::thiscall_stub<void(fiPackfile*)> fiPackfile__ctor([] ()
{
	return hook::pattern("66 89 86 5E 01 00 00 C7 06").count(1).get(0).get<void>(-7);
});

fiPackfile::fiPackfile()
{
	*(uintptr_t*)this = VTABLE_fiPackfile;

	fiPackfile__ctor(this);
}

//// fiPackfile::openArchive ////
hook::thiscall_stub<void(fiPackfile*, const char*, bool, bool, int, int)> fiPackfile__openArchive([] ()
{
	return hook::pattern("81 EC 20 0B 00 00 53 55 8B").count(1).get(0).get<void>();
});

void fiPackfile::openArchive(const char* archive, bool bTrue, bool bFalse, int type, int veryFalse)
{
	return fiPackfile__openArchive(this, archive, bTrue, bFalse, type, veryFalse);
}

//// fiPackfile::mount ////
hook::thiscall_stub<void(fiPackfile*, const char*)> fiPackfile__mount([] ()
{
	return hook::pattern("8B F9 6A 01 57 56 E8").count(1).get(0).get<void>(-6);
});

void fiPackfile::mount(const char* mountPoint) { return fiPackfile__mount(this, mountPoint); }

//// fiPackfile::closeArchive ////
hook::thiscall_stub<void(fiPackfile*)> fiPackfile__closeArchive([] ()
{
	return hook::pattern("56 8B F1 8B 86 68 01 00 00 57 33").count(1).get(0).get<void>();
});

void fiPackfile::closeArchive()
{
	return fiPackfile__closeArchive(this);
}

static HookFunction hookFunction([] ()
{
	g_vTable_fiDeviceRelative = *hook::pattern("68 10 01 00 00 E8 ? ? ? ? 83 C4 1C 85 C0").count(2).get(0).get<uintptr_t>(19);
	g_vTable_fiPackfile = *hook::pattern("66 89 86 5E 01 00 00 C7 06").count(1).get(0).get<uintptr_t>(9);
});
}