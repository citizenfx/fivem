/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiDevice.h"
#include "Hooking.h"
#include "boost/assign.hpp"

#include <boost/algorithm/string.hpp>

#include <LaunchMode.h>

#include <Error.h>

#include <ShlObj.h>
#include <boost/algorithm/string.hpp>


static InitFunction initFunction([] ()
{
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		using namespace std::string_literals;

		std::wstring citPath = MakeRelativeCitPath(L"citizen");

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string citRoot = converter.to_bytes(citPath) + "\\";

		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		device->SetPath(citRoot.c_str(), true);
		device->Mount("citizen:/");

		// mount cfx:/ -> citizen folder
		std::string fxRoot = converter.to_bytes(MakeRelativeCitPath(L""));

		rage::fiDeviceRelative* cfxDevice = new rage::fiDeviceRelative();
		cfxDevice->SetPath(fxRoot.c_str(), true);
		cfxDevice->Mount("cfx:/");

		std::wstring cachePath = MakeRelativeCitPath(L"data\\server-cache");

		if (GetFileAttributes(cachePath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			CreateDirectory(cachePath.c_str(), nullptr);
		}

		std::string cacheRoot = converter.to_bytes(cachePath) + "\\";

		rage::fiDeviceRelative* cacheDevice = new rage::fiDeviceRelative();
		cacheDevice->SetPath(cacheRoot.c_str(), true);
		cacheDevice->Mount("rescache:/");

		{
			std::string narrowPath;

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\common"s + (CfxIsSinglePlayer() ? L"-sp" : L"")));

			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(narrowPath.c_str(), true);
			relativeDevice->Mount("common:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath(narrowPath.c_str(), true);
			relativeDeviceCrc->Mount("commoncrc:/");

			rage::fiDeviceRelative* relativeDeviceGc = new rage::fiDeviceRelative();
			relativeDeviceGc->SetPath(narrowPath.c_str(), true);
			relativeDeviceGc->Mount("gamecache:/");

		}

		{
			std::string narrowPath;

			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\platform"s + (CfxIsSinglePlayer() ? L"-sp" : L"")));

			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(narrowPath.c_str(), true);
			relativeDevice->Mount("platform:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath(narrowPath.c_str(), true);
			relativeDeviceCrc->Mount("platformcrc:/");
		}

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

		// look for files in citizen\common\data
		rage::fiFindData findData;
		auto handle = device->FindFirst("citizen:/common/data/", &findData);

		if (handle != -1)
		{
			do 
			{
				if ((findData.fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					AddCrashometry(fmt::sprintf("common_data_%s", findData.fileName), "%d", findData.fileSize);
				}
			} while (device->FindNext(handle, &findData));

			device->FindClose(handle);
		}

		// look for files in citizen\platform\data too
		handle = device->FindFirst("citizen:/platform/data/", &findData);

		if (handle != -1)
		{
			do
			{
				if ((findData.fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
				{
					AddCrashometry(fmt::sprintf("platform_data_%s", findData.fileName), "%d", findData.fileSize);
				}
			} while (device->FindNext(handle, &findData));

			device->FindClose(handle);
		}
	});
});
