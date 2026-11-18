/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

using namespace std::string_literals;

#include <ShlObj.h>

#include <unordered_set>

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

	// platform(rdr3)
	"data/startup.ymt",
	"boot_launcher_flow.ymt",
	"landing_launcher_flow.ymt"
};

inline bool IsPathFiltered(const std::string& fileName)
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

// RDR3 isn't compatible with vfs::RelativeDevice here
#ifdef GTA_FIVE
class PathFilteringDevice : public vfs::RelativeDevice
{
public:
	PathFilteringDevice(const std::string& path)
		: vfs::RelativeDevice(path)
	{
	}

	bool FilterFile(const std::string& fileName)
	{
		return IsPathFiltered(fileName);
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

#if defined(IS_RDR3)
namespace
{
	bool ShouldBlockMount(const char* mountPath)
	{
		return strcmp(mountPath, "commonFilter:") == 0 ||
			   strcmp(mountPath, "platformFilter:") == 0 ||
			   strcmp(mountPath, "platformFilterV:") == 0;
	}

	const char* GetDeviceMountPath(void* self)
	{
		return reinterpret_cast<char*>(self) + 0x10;
	}
}

static uintptr_t (*g_origFiDeviceRelativeOpen)(void* self, const char* path, bool readOnly);
uintptr_t FiDeviceRelativeOpen(void* self, const char* path, bool readOnly)
{
	const char* mountPath = GetDeviceMountPath(self);

	if (ShouldBlockMount(mountPath) && IsPathFiltered(path))
	{
		return static_cast<uintptr_t>(-1);
	}

	return g_origFiDeviceRelativeOpen(self, path, readOnly);
}

static uint32_t (*g_origFiDeviceRelativeGetAttributes)(void* self, const char* path);
uint32_t FiDeviceRelativeGetAttributes(void* self, const char* path)
{
	const char* mountPath = GetDeviceMountPath(self);

	if (ShouldBlockMount(mountPath) && IsPathFiltered(path))
	{
		return static_cast<uint32_t>(-1);
	}

	return g_origFiDeviceRelativeGetAttributes(self, path);
}

static HookFunction hookFunction([]()
{
	g_origFiDeviceRelativeOpen = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 57 48 81 EC ? ? ? ? 41 8A F8"), FiDeviceRelativeOpen);
	g_origFiDeviceRelativeGetAttributes = hook::trampoline(hook::get_pattern("E8 31 ? ? ? 48 8B 4B 08 48 8D 54 24 20 48 8B 01 FF 90 58 01 00 00", -26), FiDeviceRelativeGetAttributes);
});

// The lowest 'Grass Level of Detail' the in-game menu offers is stored as 0.5. Values below that (0, negative)
// can only be set by hand-editing system.xml, and they stop grass from rendering at all, which gives an unfair
// advantage in multiplayer. The game never corrects them on its own, so bring them back to the menu minimum
// before the game loads the file.
static void ClampGrassLod(const std::wstring& fileName)
{
	constexpr float kMinGrassLod = 0.5f;
	constexpr std::string_view kTag = "<grassLod value=\"";

	std::string data;

	{
		FILE* f = _wfopen(fileName.c_str(), L"rb");

		if (!f)
		{
			return;
		}

		char buffer[8192];
		size_t read = 0;

		while ((read = fread(buffer, 1, sizeof(buffer), f)) > 0)
		{
			data.append(buffer, read);
		}

		fclose(f);
	}

	size_t valueStart = data.find(kTag);

	if (valueStart == std::string::npos)
	{
		return;
	}

	valueStart += kTag.size();
	size_t valueEnd = data.find('"', valueStart);

	if (valueEnd == std::string::npos)
	{
		return;
	}

	std::string value = data.substr(valueStart, valueEnd - valueStart);

	char* parseEnd = nullptr;
	float grassLod = strtof(value.c_str(), &parseEnd);

	// NaN fails this comparison as well
	if (parseEnd != value.c_str() && *parseEnd == '\0' && grassLod >= kMinGrassLod)
	{
		return;
	}

	data.replace(valueStart, valueEnd - valueStart, fmt::sprintf("%f", kMinGrassLod));

	// a read-only flag is what keeps such an edit from being overwritten by the game
	DWORD attributes = GetFileAttributesW(fileName.c_str());

	if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_READONLY))
	{
		SetFileAttributesW(fileName.c_str(), attributes & ~FILE_ATTRIBUTE_READONLY);
	}

	std::wstring tempName = fileName + L".tmp";
	bool written = false;

	if (FILE* f = _wfopen(tempName.c_str(), L"wb"))
	{
		written = (fwrite(data.data(), 1, data.size(), f) == data.size());
		written = (fclose(f) == 0) && written;
	}

	if (!written || !MoveFileExW(tempName.c_str(), fileName.c_str(), MOVEFILE_REPLACE_EXISTING))
	{
		_wunlink(tempName.c_str());

		FatalError("Your RedM graphics settings file contains an invalid grassLod value (%s) and could not be corrected:\n%s\n\n"
			"Make sure the file is not write-protected, or delete it and restart RedM.",
			value, ToNarrow(fileName));
	}

	trace("Corrected grassLod %s -> %f in %s\n", value, kMinGrassLod, ToNarrow(fileName));
}
#endif

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
				std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
				CreateDirectory(cfxPath.c_str(), nullptr);

				std::wstring profilePath = cfxPath + L"\\";
				relativePaths.emplace_back("fxd:/", ToNarrow(profilePath), false);

#if defined(IS_RDR3)
				CreateDirectoryW((profilePath + ToWide(settingsPath)).c_str(), NULL);
				relativePaths.emplace_back("settings:/", "fxd:/" + settingsPath, false);

				ClampGrassLod(profilePath + ToWide(settingsPath) + L"system.xml");
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
