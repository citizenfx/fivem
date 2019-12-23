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
#include <RelativeDevice.h>

#include <Error.h>

using namespace std::string_literals;

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
#include <boost/algorithm/string.hpp>

class PathFilteringDevice : public vfs::RelativeDevice
{
public:
	PathFilteringDevice(const std::string& path)
		: vfs::RelativeDevice(path)
	{

	}

	virtual THandle Open(const std::string& fileName, bool readOnly) override
	{
		std::string relPath = fileName.substr(strlen("commonFilter:/"));

		boost::algorithm::replace_all(relPath, "\\", "/");
		boost::algorithm::to_lower(relPath);

		if (relPath == "data/levels/gta5/trains.xml" ||
			relPath == "data/materials/materials.dat" ||
			relPath == "data/relationships.dat")
		{
			return InvalidHandle;
		}

		return RelativeDevice::Open(fileName, readOnly);
	}
};

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

			if (!CfxIsSinglePlayer())
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
				narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\common\\"s));

				fwRefContainer<PathFilteringDevice> filterDevice = new PathFilteringDevice(narrowPath);
				vfs::Mount(filterDevice, "commonFilter:/");

				narrowPath = "commonFilter:/";
			}
			else if (CfxIsSinglePlayer())
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
				narrowPath = converter.to_bytes(MakeRelativeCitPath(L"citizen\\common"s + (CfxIsSinglePlayer() ? L"-sp" : L"")));
			}
#if 0
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
#endif

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

							rage::fiPackfile* addonPack = new rage::fiPackfile();
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
							}
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
