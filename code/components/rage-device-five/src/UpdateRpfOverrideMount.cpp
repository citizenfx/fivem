/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "CrossBuildRuntime.h"
#include "fiDevice.h"
#include "fiCustomDevice.h"

#include <string>
#include <vector>


// It is somewhat similar to PathFilteringDevice in CitizenMount.Shared.cpp.
// However the PathFilteringDevice results in undefined behaviors when coupled with fiPackfile and fiDeviceRelative.
class FilterDevice : public rage::fiCustomDevice
{
private:
	std::string parentRoot;
	std::vector<std::string> approvedPrefixes;
	std::shared_ptr<fiDevice> underlyingDevice;

	bool IsApproved(const char* path) const
	{
		// TODO: If search time is of concern - implement a Trie / Prefix tree (https://en.wikipedia.org/wiki/Trie).
		for (const auto& prefix : approvedPrefixes)
		{
			if (std::string(path).find(prefix) == 0)
			{
				return true;
			}
		}

		return false;
	}

	std::string ReplaceRoot(const char* path) const
	{
		std::string newPath = path;
		int rootEndIndex = newPath.find(":/");
		if (rootEndIndex != std::string::npos)
		{
			newPath = newPath.replace(0, rootEndIndex + 2, parentRoot);
		}

		return newPath;
	}

public:
	FilterDevice(rage::fiPackfile* parent, const char* root, std::vector<std::string> approvedPrefixes)
		: approvedPrefixes(std::move(approvedPrefixes))
	{
		// For some reason, when working fiPackfile directly - we get undefined behavior.
		// Create a fiDeviceRelative wrapper as an ad-hoc solution.
		rage::fiDeviceRelative* defaultScenarioDevice = new rage::fiDeviceRelative();
		defaultScenarioDevice->SetPath(root, parent, true);

		parentRoot = va("wrapper_%s", root);
		defaultScenarioDevice->Mount(parentRoot.c_str());

		m_parentDeviceRef = defaultScenarioDevice;
	}

	virtual uint64_t Open(const char* fileName, bool readOnly) override
	{
		if (IsApproved(fileName))
		{
			std::string newFileName = ReplaceRoot(fileName);
			return m_parentDeviceRef->Open(newFileName.c_str(), readOnly);
		}

		return -1;
	}

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) override
	{
		if (IsApproved(fileName))
		{
			std::string newFileName = ReplaceRoot(fileName);
			return rage::fiCustomDevice::OpenBulk(newFileName.c_str(), ptr);
		}

		return -1;
	}

	virtual uint32_t GetFileAttributes(const char* path) override
	{
		if (IsApproved(path))
		{
			std::string newPath = ReplaceRoot(path);
			auto x = rage::fiCustomDevice::GetFileAttributes(newPath.c_str());
			return x;
		}

		return -1;
	}

	virtual const char* GetName() override
	{
		return "RageVFSDeviceAdapter";
	}

	virtual uint64_t Create(const char* fileName) override
	{
		return -1;
	}

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) override
	{
		return m_parentDeviceRef->Read(handle, buffer, toRead);
	}

	virtual uint32_t Write(uint64_t, void*, int) override
	{
		return -1;
	}

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) override
	{
		return m_parentDeviceRef->Seek(handle, distance, method);
	}

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) override
	{
		return m_parentDeviceRef->SeekLong(handle, distance, method);
	}

	virtual int32_t Close(uint64_t handle) override
	{
		return m_parentDeviceRef->Close(handle);
	}

	virtual uint64_t GetFileLengthLong(const char* fileName) override
	{
		std::string newFileName = ReplaceRoot(fileName);
		return m_parentDeviceRef->GetFileLengthLong(newFileName.c_str());
	}


	virtual uint64_t GetFileTime(const char* file) override
	{
		std::string newFileName = ReplaceRoot(file);
		return m_parentDeviceRef->GetFileTime(newFileName.c_str());
	}

	virtual bool SetFileTime(const char* file, FILETIME fileTime) override
	{
		return false;
	}

	virtual int m_yx() override
	{
		return m_parentDeviceRef->m_yx();
	}

	virtual int GetResourceVersion(const char* fileName, rage::ResourceFlags* version) override
	{
		std::string newFileName = ReplaceRoot(fileName);
		return m_parentDeviceRef->GetResourceVersion(newFileName.c_str(), version);
	}

	virtual uint64_t CreateLocal(const char* fileName) override
	{
		std::string newFileName = ReplaceRoot(fileName);
		return m_parentDeviceRef->CreateLocal(newFileName.c_str());
	}

	virtual void* m_xy(void* a, int len, void* c) override
	{
		std::string newFileName = ReplaceRoot((char*)c);
		return m_parentDeviceRef->m_xy(a, len, (void*)newFileName.c_str());
	}
};

void ReMountDefaultDevice(const char* rpfFile, const char* intermediateMount, const char* targetMount, std::vector<std::string> approvedPrefixes)
{
	// These classes are allocated and never deleted by design. We create them on game load and they are used until the game exists.
	rage::fiPackfile* device = new rage::fiPackfile();
	device->OpenPackfile(ToNarrow(MakeRelativeGamePath(rpfFile)).c_str(), true, 0, 0);
	device->Mount(intermediateMount);

	FilterDevice* filterDevice = new FilterDevice(device, intermediateMount, std::move(approvedPrefixes));
	rage::fiDevice::MountGlobal(targetMount, filterDevice, true);
}

static InitFunction initFunction([]()
{
#if defined(GTA_FIVE)
	// Either feature flag for the new build system is not set or requested build above latest stable.
	// Do not override .rpf files. We will load .exe and .rpf files for the requested game build in GameCache.cpp instead.
	if (xbr::GetReplaceExecutable() || xbr::GetRequestedGameBuild() >= xbr::GetGameBuild())
	{
		return;
	}

	rage::fiDevice::OnInitialMount.Connect([]()
	{
		// Mount some of the original game again, so they have higher priority than the update.rpf for the latest game build.
		ReMountDefaultDevice("x64a.rpf", "platform_default:/", "platform:/", {
			"platform:/levels/gta5/scenario/",
			// Match "sp_manifest." with the dot because it is an individual file. But the base game may try to access it with .ymt and .#mt extensions.
			"platform:/levels/gta5/sp_manifest."
		});

		// Load basic game only.
		if (xbr::GetRequestedGameBuild() == 1)
		{
			ReMountDefaultDevice("x64e.rpf", "platform_default:/", "platform:/", {
				"platform:/levels/gta5/paths.rpf"
			});
			ReMountDefaultDevice("common.rpf", "common_default:/", "common:/", {
				"common:/data/dlclist.xml"
			});
			return;
		}

		static rage::fiPackfile* updateOverrideDevice = new rage::fiPackfile();
		// This file don't exist among the original game files but GameCache.cpp will map it to the correct update.rpf in the cache folder.
		// We have to use different folders in the path instead of the different file names because rpf files are encoded based on their names.
		// And files we are downloading in GameCache are encoded with the "update.rpf" name, so if we pass any other name during mount -
		// game will not be able to decode the files which leads to undefined behavior.
		static std::string sourcePath = ToNarrow(MakeRelativeGamePath("override\\update\\update.rpf"));
		bool fileFound = updateOverrideDevice->OpenPackfile(sourcePath.c_str(), true, 0, 0);
		updateOverrideDevice->Mount("data_override:/");

		static FilterDevice* filterLevelsDevice = new FilterDevice(updateOverrideDevice, "data_override:/x64/", {
			"platform:/levels/gta5/scenario/",
			"platform:/levels/gta5/sp_manifest.",
			"platform:/levels/gta5/lodlights.rpf",
			"platform:/levels/gta5/paths.rpf"
		});
		rage::fiDevice::MountGlobal("platform:/", filterLevelsDevice, true);

		static FilterDevice* filterCommonDevice = new FilterDevice(updateOverrideDevice, "data_override:/common/", {
			"common:/data/dlclist.xml"
		});
		rage::fiDevice::MountGlobal("common:/", filterCommonDevice, true);

		static FilterDevice* filterUpdateDevice = new FilterDevice(updateOverrideDevice, "data_override:/", {
			"update:/content.xml",
			"update:/setup2.xml",
			"update:/common/data/ExtraTitleUpdateData.meta",
			"update:/x64/patch/"
		});
		rage::fiDevice::MountGlobal("update:/", filterUpdateDevice, true);

		// When using FilterDevice device here we get undefined behavior.
		// This is probably connected to the chain of mounts that is done by the game on top of the update:/dlc_patch/.
		// Use fiDeviceRelative because we want to override the whole dlc_patch folder anyway.
		static rage::fiDeviceRelative* buildSpecificDlcDevice = new rage::fiDeviceRelative();
		buildSpecificDlcDevice->SetPath("data_override:/dlc_patch/", updateOverrideDevice, true);
		buildSpecificDlcDevice->Mount("update:/dlc_patch/");
	});
#endif
});
