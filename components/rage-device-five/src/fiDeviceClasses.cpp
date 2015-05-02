#include "StdInc.h"
#include "fiDevice.h"
#include "Hooking.h"

namespace rage
{
static uintptr_t g_vTable_fiDeviceRelative;
static uintptr_t g_vTable_fiPackfile;

fiDeviceRelative::fiDeviceRelative()
{
	*(uintptr_t*)this = g_vTable_fiDeviceRelative;

	this->m_pad[256] = '\0';
}

hook::thiscall_stub<void(fiDeviceRelative*, const char*, bool, fiDevice*)> fiDeviceRelative__setPath([] ()
{
	return hook::pattern("49 8B F9 48 8B D9 4C 8B CA").count(1).get(0).get<void>(-0x17);
});

void fiDeviceRelative::SetPath(const char* relativeTo, rage::fiDevice* baseDevice, bool allowRoot)
{
	return fiDeviceRelative__setPath(this, relativeTo, allowRoot, baseDevice);
}

hook::thiscall_stub<void(fiDeviceRelative*, const char*, bool)> fiDeviceRelative__mount([] ()
{
	return hook::pattern("44 8A 81 14 01 00 00 48  8B DA 48 8B F9 48 8B D1").count(1).get(0).get<void>(-0xD);
});

void fiDeviceRelative::Mount(const char* mountPoint)
{ 
	return fiDeviceRelative__mount(this, mountPoint, true);
}

//// ---- fiPackfile ---- ////
hook::thiscall_stub<void(fiPackfile*)> fiPackfile__ctor([] ()
{
	return hook::pattern("44 89 41 28 4C 89 41 38 4C 89 41 50 48 8D").count(1).get(0).get<void>(-0x1E);
});

fiPackfile::fiPackfile()
{
	*(uintptr_t*)this = g_vTable_fiPackfile;

	fiPackfile__ctor(this);
}

//// fiPackfile::openArchive ////
hook::thiscall_stub<void(fiPackfile*, const char*, bool, int, int)> fiPackfile__openArchive([] ()
{
	return hook::pattern("48 8D 68 98 48 81 EC 40 01 00 00 41 8B F9").count(1).get(0).get<void>(-0x18);
});

void fiPackfile::OpenPackfile(const char* archive, bool bTrue, bool bFalse, int type, int veryFalse)
{
	return fiPackfile__openArchive(this, archive, bTrue, type, veryFalse);
}

//// fiPackfile::mount ////
hook::thiscall_stub<void(fiPackfile*, const char*)> fiPackfile__mount([] ()
{
	return hook::pattern("84 C0 74 1D 48 85 DB 74 0F 48").count(1).get(0).get<void>(-0x1E);
});

void fiPackfile::Mount(const char* mountPoint) { return fiPackfile__mount(this, mountPoint); }

//// fiPackfile::closeArchive ////
hook::thiscall_stub<void(fiPackfile*)> fiPackfile__closeArchive([] ()
{
	//return hook::pattern("56 8B F1 8B 86 68 01 00 00 57 33").count(1).get(0).get<void>();
	return (void*)nullptr;
});

void fiPackfile::ClosePackfile()
{
	return fiPackfile__closeArchive(this);
}

static HookFunction hookFunction([] ()
{
	auto result = hook::pattern("48 85 C0 74 11 48 83 63 08 00 48").count(1).get(0).get<uint32_t>(13);

	uintptr_t endOffset = ((uintptr_t)result) + 4;

	g_vTable_fiDeviceRelative = endOffset + *result;

	result = hook::pattern("44 89 41 28 4C 89 41 38 4C 89 41 50 48 8D 05").count(1).get(0).get<uint32_t>(15);
	
	endOffset = ((uintptr_t)result) + 4;

	g_vTable_fiPackfile = endOffset + *result;
});
}