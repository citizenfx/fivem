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

#include <CL2LaunchMode.h>
#include <CrossBuildRuntime.h>

#include <Error.h>

using namespace std::string_literals;

static int(__cdecl* origSetFunc)(void* extraContentMgr, void* a2, const char* deviceName);
int someFunc(void* a1, void* a2, const char* a3)
{
	int someResult = origSetFunc(a1, a2, a3);
	
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

	bool FilterFile(const std::string& fileName)
	{
		std::string relPath = fileName.substr(strlen("commonFilter:/"));

		boost::algorithm::replace_all(relPath, "\\", "/");
		boost::algorithm::to_lower(relPath);

		if (relPath == "data/levels/gta5/trains.xml" ||
			relPath == "data/materials/materials.dat" ||
			relPath == "data/relationships.dat" ||
			relPath == "data/dlclist.xml" ||
			relPath == "data/ai/scenarios.meta" ||
			relPath == "data/ai/conditionalanims.meta")
		{
			return true;
		}

		return false;
	}

	virtual THandle Open(const std::string& fileName, bool readOnly) override
	{
		if (FilterFile(fileName))
		{
			return InvalidHandle;
		}

		return RelativeDevice::Open(fileName, readOnly);
	}

	virtual uint32_t GetAttributes(const std::string& fileName) override
	{
		if (FilterFile(fileName))
		{
			return -1;
		}

		return RelativeDevice::GetAttributes(fileName);
	}
};

struct RelativeRedirection
{
	std::string mount;
	std::string targetPath;
	rage::fiPackfile* fiPackfile;
	fwRefContainer<vfs::Device> cfxDevice;

	inline RelativeRedirection(const std::string& mount, const std::string& relativePath)
		: mount(mount), targetPath(relativePath), fiPackfile(nullptr)
	{
	}

	inline RelativeRedirection(const std::string& mount, const fwRefContainer<vfs::Device>& cfxDevice)
		: mount(mount), cfxDevice(cfxDevice), fiPackfile(nullptr)
	{
	}

	inline RelativeRedirection(const std::string& mount, rage::fiPackfile* fiDevice)
		: mount(mount), fiPackfile(fiDevice)
	{
	}

	void Apply()
	{
		if (cfxDevice.GetRef())
		{
			vfs::Mount(cfxDevice, mount);
		}
		else if (fiPackfile)
		{
			fiPackfile->Mount(mount.c_str());
		}
		else
		{
			rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
			device->SetPath(targetPath.c_str(), true);
			device->Mount(mount.c_str());
		}
	}
};

template<typename TContainer>
static void ApplyPaths(TContainer& container)
{
	for (RelativeRedirection& value : container)
	{
		value.Apply();
	}

	container.clear();
}

static InitFunction initFunction([] ()
{
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		std::vector<RelativeRedirection> relativePaths;
		relativePaths.emplace_back("citizen:/", ToNarrow(MakeRelativeCitPath("citizen/")));
		relativePaths.emplace_back("cfx:/", ToNarrow(MakeRelativeCitPath("")));
		relativePaths.emplace_back("rescache:/", ToNarrow(MakeRelativeCitPath(fmt::sprintf(L"data/server-cache%s/", ToWide(launch::GetPrefixedLaunchModeKey("-"))))));

		if (!Is372())
		{
			std::string targetPath;

			if (!CfxIsSinglePlayer())
			{
				relativePaths.emplace_back("commonFilter:/", new PathFilteringDevice(ToNarrow(MakeRelativeCitPath(L"citizen\\common\\"s))));
				targetPath = "commonFilter:/";
			}
			else if (CfxIsSinglePlayer())
			{
				targetPath = ToNarrow(MakeRelativeCitPath(L"citizen\\common-sp"s));
			}

			relativePaths.emplace_back("common:/", targetPath);
			relativePaths.emplace_back("commoncrc:/", targetPath);
			relativePaths.emplace_back("gamecache:/", targetPath);
		}

		{
			auto targetPath = ToNarrow(MakeRelativeCitPath(L"citizen\\platform"s + (CfxIsSinglePlayer() ? L"-sp" : L"")));

			relativePaths.emplace_back("platform:/", targetPath);
			relativePaths.emplace_back("platformcrc:/", targetPath);
		}

		{
			auto targetPath = ToNarrow(MakeRelativeCitPath(fmt::sprintf(L"citizen\\platform-%d", xbr::GetGameBuild())));

			relativePaths.emplace_back("platform:/", targetPath);
			relativePaths.emplace_back("platformcrc:/", targetPath);
		}

		// we will try to fetch `cfx:/` soon, so apply current state
		ApplyPaths(relativePaths);

		{
			auto cfxDevice = rage::fiDevice::GetDevice("cfx:/", true);

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
							if (addonPack->OpenPackfile(fullFn.c_str(), true, 3, 0))
							{
								relativePaths.emplace_back(addonRoot, addonPack);

								relativePaths.emplace_back("platform:/", addonRoot + "platform/");
								relativePaths.emplace_back("platformcrc:/", addonRoot + "platform/");

								relativePaths.emplace_back("common:/", addonRoot + "common/");
								relativePaths.emplace_back("commoncrc:/", addonRoot + "common/");
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
				relativePaths.emplace_back("fxd:/", ToNarrow(profilePath));

				CoTaskMemFree(appDataPath);
			}
		}

		ApplyPaths(relativePaths);
	});
});
