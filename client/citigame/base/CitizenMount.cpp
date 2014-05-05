#include "StdInc.h"
#include "fiDevice.h"
#include "ResourceManager.h"

static InitFunction initFunction([] ()
{
	rage::fiDevice::SetInitialMountHook([] (void*)
	{
		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		device->setPath("citizen/", true);
		device->mount("citizen:/");

		static char cacheRoot[512];
		std::wstring cachePath = MakeRelativeCitPath(L"cache");

		if (GetFileAttributes(cachePath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory(cachePath.c_str(), nullptr);
		}

		std::wstring unconfPath = MakeRelativeCitPath(L"cache/unconfirmed");

		if (GetFileAttributes(unconfPath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory(unconfPath.c_str(), nullptr);
		}

		size_t offset = wcstombs(cacheRoot, cachePath.c_str(), sizeof(cacheRoot));
		cacheRoot[offset] = '\\';
		cacheRoot[offset + 1] = '\0';

		rage::fiDeviceRelative* cacheDevice = new rage::fiDeviceRelative();
		cacheDevice->setPath(cacheRoot, true);
		cacheDevice->mount("rescache:/");

		TheResources.GetCache()->LoadCache(cacheDevice);
	});
});