#include "StdInc.h"
#include "Hooking.h"
#include "Hooking.Patterns.h"
#include "Hooking.Invoke.h"
#include "fiDevice.h"
#include "CrossLibraryInterfaces.h"

namespace rage
{
hook::cdecl_stub<rage::fiDevice*(const char*, bool, bool, bool)> fiDevice__GetDevice([] ()
{
	return (void*)0x1425050B4;
});

fiDevice* fiDevice::GetDevice(const char* path, bool allowRoot) { return fiDevice__GetDevice(path, allowRoot, false, true); }

hook::cdecl_stub<bool(const char*, fiDevice*, bool)> fiDevice__MountGlobal([] ()
{
	return (void*)0x1425080CC;
});

bool fiDevice::MountGlobal(const char* mountPoint, fiDevice* device, bool allowRoot)
{
	return fiDevice__MountGlobal(mountPoint, device, allowRoot);
}

// DCEC20
hook::cdecl_stub<void(const char*)> fiDevice__Unmount([] ()
{
	// TODO: PATTERN
	return (void*)0x14250B7E8;
});

void fiDevice::Unmount(const char* rootPath) { fiDevice__Unmount(rootPath); }

rage::fiDevice::~fiDevice() {}

__declspec(dllexport) fwEvent<> fiDevice::OnInitialMount;

class fiCollection : public fiDevice
{
public:
	virtual void a1() = 0;
	virtual void a2() = 0;
	virtual void a3() = 0;
	virtual void a4() = 0;
	virtual void a5() = 0;
	virtual void a6() = 0;
	virtual void a7() = 0;
	virtual void a8() = 0;
	virtual void a9() = 0;
	virtual void a10() = 0;
	virtual void a11() = 0;
	virtual void a12() = 0;
	virtual void a13() = 0;
	virtual void aaa() = 0;
	virtual uint32_t GetEntryCount() = 0;
	virtual void a15() = 0;
	virtual uint32_t GetEntryNameHash(uint32_t id) = 0;
	virtual int GetEntryFileExtId(uint32_t id) = 0;
};
}

#include <boost/algorithm/string.hpp>

static InitFunction initFunction([] ()
{
});
