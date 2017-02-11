/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiDevice.h"
#include "Hooking.h"
#include "boost\assign.hpp"

static int(__cdecl* origSetFunc)(void* extraContentMgr, void* a2, const char* deviceName);
int someFunc(void* a1, void* a2, const char* a3)
{
	int someResult = origSetFunc(a1, a2, a3);

	// add a fiDeviceRelative for a3 (f.i. dlc_dick:/, breakpoint to be sure) to wherever-the-fuck-you-want, removing the :/ at the end for the target path)
	trace("somefunc found %s!\n", a3);
	
	std::string dlcName = std::string(a3);
	dlcName = dlcName.substr(0, dlcName.find(":"));

	auto exists = [] (const std::string& path)
	{
		auto device = rage::fiDevice::GetDevice(path.c_str(), true);

		if (device)
		{
			return (device->GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES);
		}

		return false;
	};

	{
		std::string name = "citizen:/dlc/" + dlcName + "/";

		if (exists(name))
		{
			trace("dlc mounted: %s\n", name.c_str());

			{
				rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
				dlc->SetPath(name.c_str(), true);
				dlc->Mount((dlcName + ":/").c_str());
			}

			{
				rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
				dlc->SetPath(name.c_str(), true);
				dlc->Mount((dlcName + "CRC:/").c_str());
			}
		}
	}

	{
		std::string name = "citizen:/dlc/" + dlcName + "CRC/";

		if (exists(name))
		{
			trace("dlc mounted: %s\n", name.c_str());

			rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
			dlc->SetPath(name.c_str(), true);
			dlc->Mount((dlcName + "CRC:/").c_str());
		}
	}

	return someResult;
}
static HookFunction hookFunction([]()
{
	hook::set_call(&origSetFunc, hook::pattern("66 39 79 38 74 06 4C 8B  41 30 EB 07 4C 8D").count(1).get(0).get<void>(19));
	hook::call(hook::pattern("66 39 79 38 74 06 4C 8B  41 30 EB 07 4C 8D").count(1).get(0).get<void>(19), someFunc);
});

#include <ShlObj.h>

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

		std::string cacheRoot = converter.to_bytes(cachePath) + "\\";

		rage::fiDeviceRelative* cacheDevice = new rage::fiDeviceRelative();
		cacheDevice->SetPath(cacheRoot.c_str(), true);
		cacheDevice->Mount("rescache:/");

		{
			PWSTR appDataPath;
			if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
			{
				// create the directory if not existent
				std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
				CreateDirectory(cfxPath.c_str(), nullptr);

				std::wstring profilePath = cfxPath + L"\\";

				rage::fiDeviceRelative* fxUserDevice = new rage::fiDeviceRelative();
				fxUserDevice->SetPath(converter.to_bytes(profilePath).c_str(), true);
				fxUserDevice->Mount("fxd:/");

				CoTaskMemFree(appDataPath);
			}
		}
	});
});