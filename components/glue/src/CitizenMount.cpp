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
	trace(va("somefunc found %s!", a3));
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	std::string dlcName = std::string(a3);
	dlcName = dlcName.substr(0, dlcName.find(":"));
	std::wstring dlcNameTwo(dlcName.begin(), dlcName.end());
	std::string dlcPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\dlc\\" + dlcNameTwo + L"\\"));
	if (GetFileAttributes(std::wstring(dlcPath.begin(), dlcPath.end()).c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		trace(va("dlc mounted: %s", dlcPath.c_str()));
		rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
		dlc->SetPath(dlcPath.c_str(), true);
		dlcName = dlcName + ":/";
		dlc->Mount(dlcName.c_str());
	}
	dlcPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\dlc\\" + dlcNameTwo + L"CRC\\"));
	if (GetFileAttributes(std::wstring(dlcPath.begin(), dlcPath.end()).c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		trace(va("dlc mounted: %s", dlcPath.c_str()));
		rage::fiDeviceRelative* dlc = new rage::fiDeviceRelative();
		dlc->SetPath(dlcPath.c_str(), true);
		dlcName = dlcName + "CRC:/";
		dlc->Mount(dlcName.c_str());
	}
	return someResult;
}
static HookFunction hookFunction([]()
{
	hook::set_call(&origSetFunc, hook::pattern("66 39 79 38 74 06 4C 8B  41 30 EB 07 4C 8D").count(1).get(0).get<void>(19));
	hook::call(hook::pattern("66 39 79 38 74 06 4C 8B  41 30 EB 07 4C 8D").count(1).get(0).get<void>(19), someFunc);
});

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

		std::string dlcPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\dlc\\update\\"));
		if (GetFileAttributes(std::wstring(dlcPath.begin(), dlcPath.end()).c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(dlcPath.c_str(), nullptr, true);
			relativeDevice->Mount("update:/");
		}
	});
});