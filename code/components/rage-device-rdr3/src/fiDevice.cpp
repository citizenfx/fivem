#include "StdInc.h"
#include "Hooking.h"
#include "fiDevice.h"
#include "CrossLibraryInterfaces.h"

namespace rage
{
hook::cdecl_stub<rage::fiDevice*(const char*, bool, bool, bool)> fiDevice__GetDevice([] ()
{
	return hook::get_pattern("41 8A F0 40 8A EA 41 B8 07 00 00 00", -0x14);
});

fiDevice* fiDevice::GetDevice(const char* path, bool allowRoot) { return fiDevice__GetDevice(path, allowRoot, true, true); }

hook::cdecl_stub<bool(const char*, fiDevice*, bool)> fiDevice__MountGlobal([] ()
{
	return hook::get_pattern("48 83 EC 30 4C 8B FA 41 8A F0", -0x1A);
});

bool fiDevice::MountGlobal(const char* mountPoint, fiDevice* device, bool allowRoot)
{
	return fiDevice__MountGlobal(mountPoint, device, allowRoot);
}

// DCEC20
hook::cdecl_stub<void(const char*)> fiDevice__Unmount([] ()
{
	return hook::get_pattern("33 DB 85 C0 75 1F 48 39 1D", -0x2B);
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
