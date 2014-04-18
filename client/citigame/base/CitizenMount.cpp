#include "StdInc.h"
#include "fiDevice.h"

static InitFunction initFunction([] ()
{
	rage::fiDevice::SetInitialMountHook([] (void*)
	{
		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		device->setPath("citizen/", true);
		device->mount("citizen:/");
	});
});