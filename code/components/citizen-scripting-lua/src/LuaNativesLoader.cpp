#include "StdInc.h"

#include <VFSManager.h>
#include <VFSZipFile.h>
#include <ResourceManager.h>

namespace fx
{
bool mountedAnyNatives = false;
}

static void MountNatives(const std::string& name)
{
	static std::map<std::string, std::vector<uint8_t>> storedFiles;

	const void* thisData = nullptr;
	size_t thisDataSize = 0;

	{
		auto stream = vfs::OpenRead(fmt::sprintf("citizen:/scripting/lua/%s.zip", name));

		if (stream.GetRef())
		{
			storedFiles[name] = stream->ReadToEnd();

			thisData = storedFiles[name].data();
			thisDataSize = storedFiles[name].size();
		}
	}

	if (thisData)
	{
		fwRefContainer<vfs::ZipFile> device = new vfs::ZipFile();

		if (device->OpenArchive(fmt::sprintf("memory:$%016llx,%d,0:%s", (uintptr_t)thisData, thisDataSize, name)))
		{
			fx::mountedAnyNatives = true;
			vfs::Mount(device, fmt::sprintf("nativesLua:/%s/", name));
		}
	}
}

class MarkerDevice : public vfs::Device
{
	// Inherited via Device
	virtual THandle Open(const std::string& fileName, bool readOnly) override
	{
		if (fileName == "n:m:a:r:k:e:r" && readOnly)
		{
			return 1;
		}

		return vfs::Device::InvalidHandle;
	}

	virtual size_t Read(THandle handle, void* outBuffer, size_t size) override
	{
		return -1;
	}

	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override
	{
		return -1;
	}

	virtual bool Close(THandle handle) override
	{
		return true;
	}

	virtual THandle FindFirst(const std::string& folder, vfs::FindData* findData) override
	{
		return vfs::Device::InvalidHandle;
	}

	virtual bool FindNext(THandle handle, vfs::FindData* findData) override
	{
		return false;
	}

	virtual void FindClose(THandle handle) override
	{
	}
};

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager*)
	{
		bool mountedFiles = false;

		if (auto device = vfs::GetDevice("nativesLua:/marker/"); device.GetRef())
		{
			mountedFiles = device->Open("n:m:a:r:k:e:r", true) != vfs::Device::InvalidHandle;
		}

		if (!mountedFiles)
		{
#if defined(IS_RDR3)
			MountNatives("rdr3_universal");
#elif defined(GTA_NY)
			MountNatives("ny_universal");
#elif defined(GTA_FIVE)
			MountNatives("natives_universal");
			MountNatives("natives_21e43a33");
			MountNatives("natives_0193d0af");
#elif defined(IS_FXSERVER)
			MountNatives("natives_server");
#endif

			vfs::Mount(new MarkerDevice(), "nativesLua:/marker/");
		}
	});
});
