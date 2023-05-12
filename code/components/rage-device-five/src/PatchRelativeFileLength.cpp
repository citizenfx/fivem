#include "StdInc.h"
#include "Hooking.h"

#include "fiDevice.h"

struct fiDeviceRelative_data
{
	void* vtbl;
	rage::fiDevice* parent;
};

static int fiDeviceRelative_GetLength(fiDeviceRelative_data* self, uintptr_t handle)
{
	return self->parent->GetFileLength(handle);
}

static int64_t fiDeviceRelative_GetLengthInt64(fiDeviceRelative_data* self, uintptr_t handle)
{
	return self->parent->GetFileLengthUInt64(handle);
}

static HookFunction hookFunction([]
{
	auto vtable_fiDeviceRelative = hook::get_address<void**>(hook::get_pattern("48 85 C0 74 11 48 83 63 08 00 48", 10), 3, 7);
	hook::put(&vtable_fiDeviceRelative[14], fiDeviceRelative_GetLength);
	hook::put(&vtable_fiDeviceRelative[15], fiDeviceRelative_GetLengthInt64);
});
