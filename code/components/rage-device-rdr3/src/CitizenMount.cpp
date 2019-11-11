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

#include <VFSManager.h>
#include <VFSRagePackfile7.h>

#include <Error.h>

using namespace std::string_literals;

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

		// mount cfx:/ -> citizen folder
		std::string fxRoot = converter.to_bytes(MakeRelativeCitPath(L""));

		rage::fiDeviceRelative* cfxDevice = new rage::fiDeviceRelative();
		cfxDevice->SetPath(fxRoot.c_str(), true);
		cfxDevice->Mount("cfx:/");

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
			std::string narrowPath;

			if (CfxIsSinglePlayer() || true)
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
				narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\common"s + (CfxIsSinglePlayer() ? L"-sp" : L"")));
			}
			else
			{
				static fwRefContainer<vfs::RagePackfile7> citizenCommon = new vfs::RagePackfile7();
				if (!citizenCommon->OpenArchive("citizen:/citizen_common.rpf", true))
				{
					FatalError("Opening citizen_common.rpf failed!");
				}

				vfs::Mount(citizenCommon, "citizen_common:/");
				narrowPath = "citizen_common:/";
			}

			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDevice->Mount("common:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDeviceCrc->Mount("commoncrc:/");

			rage::fiDeviceRelative* relativeDeviceGc = new rage::fiDeviceRelative();
			relativeDeviceGc->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDeviceGc->Mount("gamecache:/");

		}

		{
			std::string narrowPath;

			if (CfxIsSinglePlayer() || true)
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
				narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\platform"s + (CfxIsSinglePlayer() ? L"-sp" : L"")));
			}
			else
			{
				static fwRefContainer<vfs::RagePackfile7> citizenPlatform = new vfs::RagePackfile7();
				if (!citizenPlatform->OpenArchive("citizen:/citizen_platform.rpf", true))
				{
					FatalError("Opening citizen_platform.rpf failed!");
				}

				vfs::Mount(citizenPlatform, "citizen_platform:/");
				narrowPath = "citizen_platform:/";
			}

			rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
			relativeDevice->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDevice->Mount("platform:/");

			rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
			relativeDeviceCrc->SetPath(narrowPath.c_str(), nullptr, true);
			relativeDeviceCrc->Mount("platformcrc:/");
		}

		if (CfxIsSinglePlayer() || true)
		{
			rage::fiFindData findData;
			auto handle = cfxDevice->FindFirst("cfx:/addons/", &findData);

			if (handle != -1)
			{
				do
				{
					if ((findData.fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
					{
						std::string fn = findData.fileName;

						if (boost::algorithm::ends_with(fn, ".rpf"))
						{
							std::string fullFn = "cfx:/addons/" + fn;
							std::string addonRoot = "addons:/" + fn.substr(0, fn.find_last_of('.')) + "/";

							/*rage::fiPackfile* addonPack = new rage::fiPackfile();
							addonPack->OpenPackfile(fullFn.c_str(), true, false, 0);
							addonPack->Mount(addonRoot.c_str());

							{
								rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
								relativeDevice->SetPath((addonRoot + "platform/").c_str(), nullptr, true);
								relativeDevice->Mount("platform:/");

								rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
								relativeDeviceCrc->SetPath((addonRoot + "platform/").c_str(), nullptr, true);
								relativeDeviceCrc->Mount("platformcrc:/");
							}

							{
								rage::fiDeviceRelative* relativeDevice = new rage::fiDeviceRelative();
								relativeDevice->SetPath((addonRoot + "common/").c_str(), nullptr, true);
								relativeDevice->Mount("common:/");

								rage::fiDeviceRelative* relativeDeviceCrc = new rage::fiDeviceRelative();
								relativeDeviceCrc->SetPath((addonRoot + "common/").c_str(), nullptr, true);
								relativeDeviceCrc->Mount("commoncrc:/");
							}*/
						}
					}
				} while (cfxDevice->FindNext(handle, &findData));

				cfxDevice->FindClose(handle);
			}
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
