/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiDevice.h"

#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>

#if __has_include(<openssl/sha.h>)
#include <openssl/sha.h>
#endif

#include <VFSManager.h>
#include <VFSRagePackfile7.h>
#include <RelativeDevice.h>

#include <CL2LaunchMode.h>
#include <CrossBuildRuntime.h>
#include <PureModeState.h>

#include <Error.h>

using namespace std::string_literals;

#include <ShlObj.h>

#include <unordered_set>

// RDR3 isn't compatible with vfs::RelativeDevice here
#ifdef GTA_FIVE
static std::unordered_set<std::string> g_basePaths{
	// common/
	"data/gameconfig.xml",
	"data/gta5_cache_y.dat",
	"data/ui/pausemenu.xml",

	// platform/
	"audio/config/categories.dat22.rel",
	"data/control/default.meta",
	"data/control/settings.meta",
	"data/control/keyboard layout/da.meta",
};

class PathFilteringDevice : public vfs::RelativeDevice
{
public:
	PathFilteringDevice(const std::string& path)
		: vfs::RelativeDevice(path)
	{
	}

	bool FilterFile(const std::string& fileName)
	{
		std::string relPath = fileName.substr(fileName.find(":/") + 2);
		boost::algorithm::replace_all(relPath, "\\", "/");
		boost::algorithm::to_lower(relPath);

		if (fx::client::GetPureLevel() >= 1)
		{
			if (g_basePaths.find(relPath) == g_basePaths.end())
			{
				return true;
			}
		}

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

	virtual THandle Open(const std::string& fileName, bool readOnly, bool append = false) override
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

inline auto MakePathFilteringDevice(const std::string& path)
{
	return fwRefContainer(new PathFilteringDevice(path));
}
#else
inline auto MakePathFilteringDevice(const std::string& path)
{
	return path;
}
#endif

struct RelativeRedirection
{
	std::string mount;
	std::string targetPath;
	bool relativeFlag = false;
#if !defined(IS_RDR3)
	rage::fiPackfile* fiPackfile = nullptr;
#endif
	fwRefContainer<vfs::Device> cfxDevice;

	inline RelativeRedirection(const std::string& mount, const std::string& relativePath, bool relativeFlag = true)
		: mount(mount), targetPath(relativePath), relativeFlag(relativeFlag)
	{
	}

	inline RelativeRedirection(const std::string& mount, const fwRefContainer<vfs::Device>& cfxDevice)
		: mount(mount), cfxDevice(cfxDevice)
	{
	}

#if !defined(IS_RDR3)
	inline RelativeRedirection(const std::string& mount, rage::fiPackfile* fiPackfile)
		: mount(mount), fiPackfile(fiPackfile)
	{
	}
#endif

	void Apply()
	{
		if (cfxDevice.GetRef())
		{
			vfs::Mount(cfxDevice, mount);
		}
#if !defined(IS_RDR3)
		else if (fiPackfile)
		{
			fiPackfile->Mount(mount.c_str());
		}
#endif
		else
		{
			rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
			device->SetPath(targetPath.c_str(), relativeFlag);
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

static InitFunction initFunction([]()
{
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		std::vector<RelativeRedirection> relativePaths;
		relativePaths.emplace_back("citizen:/", ToNarrow(MakeRelativeCitPath("citizen/")));
		relativePaths.emplace_back("cfx:/", ToNarrow(MakeRelativeCitPath("")));

		auto cacheRoot = MakeRelativeCitPath(fmt::sprintf(L"data/server-cache%s/", ToWide(launch::GetPrefixedLaunchModeKey("-"))));
		CreateDirectoryW(cacheRoot.c_str(), NULL);

		relativePaths.emplace_back("rescache:/", ToNarrow(cacheRoot));

		{
			std::string targetPath = "commonFilter:/";
			relativePaths.emplace_back("commonFilter:/", MakePathFilteringDevice(ToNarrow(MakeRelativeCitPath(L"citizen\\common\\"))));

			relativePaths.emplace_back("common:/", targetPath);
			relativePaths.emplace_back("commoncrc:/", targetPath);
			relativePaths.emplace_back("gamecache:/", targetPath);
		}

		{
			std::string targetPath = "platformFilter:/";
			relativePaths.emplace_back("platformFilter:/", MakePathFilteringDevice(ToNarrow(MakeRelativeCitPath(L"citizen\\platform"))));

			relativePaths.emplace_back("platform:/", targetPath);
			relativePaths.emplace_back("platformcrc:/", targetPath);
		}

		{
			std::string targetPath = "platformFilterV:/";
			auto platformPath = ToNarrow(MakeRelativeCitPath(fmt::sprintf(L"citizen\\platform-%d\\", xbr::GetGameBuild())));
			relativePaths.emplace_back("platformFilterV:/", MakePathFilteringDevice(platformPath));

			relativePaths.emplace_back("platform:/", targetPath);
			relativePaths.emplace_back("platformcrc:/", targetPath);

			if (xbr::IsGameBuildOrGreater<2060>() && GetFileAttributes(ToWide(platformPath).c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				trace("game build %d is expected to have a platform directory, but it doesn't\n", xbr::GetGameBuild());

#ifdef _DEBUG
				__debugbreak();
#endif
			}
		}

		// we will try to fetch `cfx:/` soon, so apply current state
		ApplyPaths(relativePaths);

		// no fiPackfile for RDR
#if defined(GTA_FIVE)
		if (fx::client::GetPureLevel() == 0)
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
#endif

#if defined(IS_RDR3)
		std::string settingsPath = fmt::sprintf("rdr3_settings%s/", (!xbr::IsGameBuildOrGreater<1436>()) ? fmt::sprintf("_b%d", xbr::GetGameBuild()) : "");
#endif

		{
			PWSTR appDataPath;
			if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
			{
				// create the directory if not existent
				std::wstring cfxPath = std::wstring(appDataPath) + L"\\VMP";
				CreateDirectory(cfxPath.c_str(), nullptr);

				std::wstring profilePath = cfxPath + L"\\";
				relativePaths.emplace_back("fxd:/", ToNarrow(profilePath), false);

#if defined(IS_RDR3)
				CreateDirectoryW((profilePath + ToWide(settingsPath)).c_str(), NULL);
				relativePaths.emplace_back("settings:/", "fxd:/" + settingsPath, false);
#endif

				CoTaskMemFree(appDataPath);
			}
		}

		ApplyPaths(relativePaths);

#if defined(GTA_FIVE) && __has_include(<openssl/sha.h>)
		// validate some game files
		auto validateFile = [](const std::wstring& fileName, std::string_view hash)
		{
			bool valid = false;
			FILE* f = _wfopen(MakeRelativeCitPath(fileName).c_str(), L"rb");
			if (f)
			{
				SHA256_CTX sha;
				SHA256_Init(&sha);

				uint8_t hashBuffer[16384];
				size_t read = 0;
				do
				{
					read = fread(hashBuffer, 1, sizeof(hashBuffer), f);

					if (read > 0)
					{
						SHA256_Update(&sha, hashBuffer, read);
					}
				} while (read > 0);

				uint8_t sha256[256 / 8];
				SHA256_Final(sha256, &sha);

				fclose(f);

				std::string tgtHash;
				boost::algorithm::unhex(hash, std::back_inserter(tgtHash));

				if (tgtHash == std::string_view{ (char*)sha256, sizeof(sha256) })
				{
					valid = true;
				}
			}

			if (!valid)
			{
				trace("%s was invalid - removing/verifying next launch\n", ToNarrow(fileName));

				_wunlink(MakeRelativeCitPath(fileName).c_str());
				_wunlink(MakeRelativeCitPath(L"content_index.xml").c_str());
			}
		};

		validateFile(L"citizen/common/data/gta5_cache_y.dat", "10a43796e911a7aa5b53111a7a91c30bb8b99539f46bd0e6a755fe55de819590");
#endif
	});
});
