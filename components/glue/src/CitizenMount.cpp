/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiDevice.h"
#include "ResourceManager.h"

static InitFunction initFunction([] ()
{
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		std::wstring citPath = MakeRelativeCitPath(L"citizen");

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string citRoot = converter.to_bytes(citPath) + "\\";

		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		device->SetPath(citRoot.c_str(), true);
		device->Mount("citizen:/");

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

		std::string cacheRoot = converter.to_bytes(cachePath) + "\\";
		
		rage::fiDeviceRelative* cacheDevice = new rage::fiDeviceRelative();
		cacheDevice->SetPath(cacheRoot.c_str(), true);
		cacheDevice->Mount("rescache:/");

		TheResources.GetCache()->LoadCache(cacheDevice);
	});
});