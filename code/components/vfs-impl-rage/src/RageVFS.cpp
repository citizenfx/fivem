/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <fiCustomDevice.h>

#include <VFSDevice.h>
#include <VFSManager.h>

#include <LocalDevice.h>
#include <RelativeDevice.h>

#include <IteratorView.h>

#include <strsafe.h>

#include <Error.h>

#include <optional>
#include <queue>

static bool g_vfsInitStarted;
static bool g_vfsInitDone;

class RageVFSDeviceAdapter : public rage::fiCustomDevice
{
private:
	fwRefContainer<vfs::Device> m_cfxDevice;

	std::optional<bool> m_collectionCache;

public:
	RageVFSDeviceAdapter(fwRefContainer<vfs::Device> device);

	virtual size_t Open(const char* fileName, bool readOnly) override;

	virtual size_t OpenBulk(const char* fileName, uint64_t* ptr) override;

	virtual size_t Create(const char* fileName) override;

	virtual uint32_t Read(size_t handle, void* buffer, uint32_t toRead) override;

	virtual uint32_t ReadBulk(size_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override;

	virtual uint32_t Write(size_t, void*, int) override;

	virtual uint32_t Seek(size_t handle, int32_t distance, uint32_t method) override;

#ifndef GTA_NY
	virtual uint64_t SeekLong(size_t handle, int64_t distance, uint32_t method) override;
#endif

	virtual int32_t Close(size_t handle) override;

	virtual int32_t CloseBulk(size_t handle) override;

	virtual int GetFileLength(size_t handle) override;

	virtual uint64_t GetFileLengthLong(const char* fileName) override;

#ifndef GTA_NY
	virtual uint64_t GetFileLengthUInt64(size_t handle) override;
#endif

	virtual bool RemoveFile(const char* file) override;

	virtual int RenameFile(const char* from, const char* to) override;

	virtual int CreateDirectory(const char* dir) override;

	virtual int RemoveDirectory(const char* dir) override;

	virtual uint64_t GetFileTime(const char* file) override;

	virtual bool SetFileTime(const char* file, FILETIME fileTime) override;

	virtual uint32_t GetFileAttributes(const char* path) override;

	virtual size_t FindFirst(const char* path, rage::fiFindData* findData) override;

	virtual bool FindNext(size_t handle, rage::fiFindData* findData) override;

	virtual int FindClose(size_t handle) override;

	virtual int GetResourceVersion(const char* fileName, rage::ResourceFlags* version) override;
	/*{
		version->flag1 = 0;
		version->flag2 = 0;

		trace(__FUNCTION__ " not implemented");

		return 0;
	}*/

#ifndef GTA_NY
	virtual int m_yx() override
	{
		return 2;
	}

	virtual bool IsCollection() override;

	virtual const char* GetName() override
	{
		return "RageVFSDeviceAdapter";
	}
#endif
};

RageVFSDeviceAdapter::RageVFSDeviceAdapter(fwRefContainer<vfs::Device> device)
	: m_cfxDevice(device)
{

}

size_t RageVFSDeviceAdapter::Open(const char* fileName, bool readOnly)
{
	return m_cfxDevice->Open(fileName, readOnly);
}

size_t RageVFSDeviceAdapter::OpenBulk(const char* fileName, uint64_t* ptr)
{
	return m_cfxDevice->OpenBulk(fileName, ptr);
}

size_t RageVFSDeviceAdapter::Create(const char* fileName)
{
	return m_cfxDevice->Create(fileName);
}

uint32_t RageVFSDeviceAdapter::Read(size_t handle, void* buffer, uint32_t toRead)
{
	return m_cfxDevice->Read(handle, buffer, toRead);
}

uint32_t RageVFSDeviceAdapter::ReadBulk(size_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	return m_cfxDevice->ReadBulk(handle, ptr, buffer, toRead);
}

uint32_t RageVFSDeviceAdapter::Write(size_t handle, void* buffer, int length)
{
	return m_cfxDevice->Write(handle, buffer, length);
}

int32_t RageVFSDeviceAdapter::Close(size_t handle)
{
	return m_cfxDevice->Close(handle) ? 0 : -1;
}

int32_t RageVFSDeviceAdapter::CloseBulk(size_t handle)
{
	return m_cfxDevice->CloseBulk(handle) ? 0 : -1;
}

bool RageVFSDeviceAdapter::RemoveFile(const char* file)
{
	return m_cfxDevice->RemoveFile(file);
}

int RageVFSDeviceAdapter::CreateDirectory(const char* dir)
{
	return m_cfxDevice->CreateDirectory(dir);
}

int RageVFSDeviceAdapter::RemoveDirectory(const char* dir)
{
	return m_cfxDevice->RemoveDirectory(dir);
}

int RageVFSDeviceAdapter::RenameFile(const char* from, const char* to)
{
	return m_cfxDevice->RenameFile(from, to);
}

int RageVFSDeviceAdapter::GetFileLength(size_t handle)
{
	return m_cfxDevice->GetLength(handle);
}

uint64_t RageVFSDeviceAdapter::GetFileLengthLong(const char* fileName)
{
	return m_cfxDevice->GetLength(fileName);
}

#ifndef GTA_NY
uint64_t RageVFSDeviceAdapter::GetFileLengthUInt64(uint64_t handle)
{
	return m_cfxDevice->GetLength(handle);
}
#endif

uint32_t RageVFSDeviceAdapter::Seek(size_t handle, int32_t distance, uint32_t method)
{
	return m_cfxDevice->Seek(handle, distance, method);
}

#ifndef GTA_NY
uint64_t RageVFSDeviceAdapter::SeekLong(size_t handle, int64_t distance, uint32_t method)
{
	return m_cfxDevice->Seek(handle, distance, method);
}
#endif

uint64_t RageVFSDeviceAdapter::GetFileTime(const char* file)
{
	// returning 0 is bad because some subsystems use 0 as a sentinel value
	// we return the modification time of README.txt on the GTA1 EU release:
	// 1997-10-15T08:31:50Z
	return 125213779100000000;
}

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

struct GetRagePageFlagsExtension
{
	const char* fileName; // in
	int version;
	rage::ResourceFlags flags; // out
};

int RageVFSDeviceAdapter::GetResourceVersion(const char* file, rage::ResourceFlags* version)
{
	GetRagePageFlagsExtension ext;
	ext.fileName = file;

	if (m_cfxDevice->ExtensionCtl(VFS_GET_RAGE_PAGE_FLAGS, &ext, sizeof(ext)))
	{
		*version = ext.flags;
#if defined(GTA_NY)
		// pgStreamer expects this in NY - has to be -1 or a version assertion will fail (North laziness)
		return -1;
#else
		return ext.version;
#endif
	}

#if defined(GTA_NY)
	*version = 0;
#elif !defined(IS_RDR3)
	version->flag1 = 0;
	version->flag2 = 0;
#else
	memset(version, 0, sizeof(*version));
#endif

	trace(__FUNCTION__ " not implemented\n");

	return 0;
}

bool RageVFSDeviceAdapter::SetFileTime(const char* file, FILETIME fileTime)
{
	return false;
}

uint32_t RageVFSDeviceAdapter::GetFileAttributes(const char* path)
{
	return m_cfxDevice->GetAttributes(path);
}

size_t RageVFSDeviceAdapter::FindFirst(const char* folder, rage::fiFindData* findData)
{
	vfs::FindData findDataOrig;
	auto handle = m_cfxDevice->FindFirst(folder, &findDataOrig);

	if (handle != vfs::Device::InvalidHandle)
	{
		StringCbCopyA(findData->fileName, sizeof(findData->fileName), findDataOrig.name.c_str());
		findData->fileAttributes = findDataOrig.attributes;
		findData->fileSize = findDataOrig.length;
	}

	return handle;
}

bool RageVFSDeviceAdapter::FindNext(size_t handle, rage::fiFindData* findData)
{
	vfs::FindData findDataOrig;
	bool valid = m_cfxDevice->FindNext(handle, &findDataOrig);

	if (valid)
	{
		StringCbCopyA(findData->fileName, sizeof(findData->fileName), findDataOrig.name.c_str());
		findData->fileAttributes = findDataOrig.attributes;
		findData->fileSize = findDataOrig.length;
	}

	return valid;
}

int RageVFSDeviceAdapter::FindClose(size_t handle)
{
	m_cfxDevice->FindClose(handle);
	return 0;
}

using THandle = vfs::Device::THandle;

class RageVFSDevice : public vfs::Device
{
private:
	rage::fiDevice* m_device;

	size_t m_pathPrefixLength;

public:
	RageVFSDevice(rage::fiDevice* device);

	virtual THandle Open(const std::string& fileName, bool readOnly) override;

	virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr) override;

	virtual THandle Create(const std::string& filename) override;

	virtual size_t Read(THandle handle, void* outBuffer, size_t size) override;

	virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size) override;

	virtual size_t Write(THandle handle, const void* buffer, size_t size) override;

	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override;

	virtual bool Close(THandle handle) override;

	virtual bool CloseBulk(THandle handle) override;

	virtual bool RemoveFile(const std::string& filename) override;

	virtual bool RenameFile(const std::string& from, const std::string& to) override;

	virtual bool CreateDirectory(const std::string& name) override;

	virtual bool RemoveDirectory(const std::string& name) override;

	virtual size_t GetLength(THandle handle) override;

	virtual size_t GetLength(const std::string& fileName) override;

	virtual THandle FindFirst(const std::string& folder, vfs::FindData* findData) override;

	virtual bool FindNext(THandle handle, vfs::FindData* findData) override;

	virtual void FindClose(THandle handle) override;

	virtual uint32_t GetAttributes(const std::string& filename) override;

	virtual void SetPathPrefix(const std::string& pathPrefix) override;

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override;

	bool IsCollection();
};

RageVFSDevice::RageVFSDevice(rage::fiDevice* device)
	: m_device(device), m_pathPrefixLength(0)
{

}

template<typename T>
static std::enable_if_t<sizeof(size_t) <= sizeof(T), size_t> WrapInt(T value)
{
	return value;
}

template<typename T>
static std::enable_if_t<(sizeof(size_t) > sizeof(T)), size_t> WrapInt(T value)
{
	auto unsignedValue = (std::make_unsigned_t<T>)value;

	if (unsignedValue == std::numeric_limits<decltype(unsignedValue)>::max())
	{
		return std::numeric_limits<size_t>::max();
	}

	return static_cast<size_t>(value);
}

THandle RageVFSDevice::Open(const std::string& fileName, bool readOnly)
{
	return m_device->Open(fileName.substr(m_pathPrefixLength).c_str(), readOnly);
}

THandle RageVFSDevice::OpenBulk(const std::string& fileName, uint64_t* ptr)
{
	return m_device->OpenBulk(fileName.substr(m_pathPrefixLength).c_str(), ptr);
}

THandle RageVFSDevice::Create(const std::string& filename)
{
	return m_device->Create(filename.substr(m_pathPrefixLength).c_str());
}

size_t RageVFSDevice::Read(THandle handle, void* outBuffer, size_t size)
{
	return WrapInt(m_device->Read(handle, outBuffer, size));
}

size_t RageVFSDevice::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	return WrapInt(m_device->ReadBulk(handle, ptr, outBuffer, size));
}

size_t RageVFSDevice::Write(THandle handle, const void* buffer, size_t size)
{
	return WrapInt(m_device->Write(handle, const_cast<void*>(buffer), size));
}

size_t RageVFSDevice::Seek(THandle handle, intptr_t offset, int seekType)
{
#ifndef GTA_NY
	return WrapInt(m_device->SeekLong(handle, offset, seekType));
#else
	return m_device->Seek(handle, offset, seekType);
#endif
}


bool RageVFSDevice::Close(THandle handle)
{
	return m_device->Close(handle) == 0;
}

bool RageVFSDevice::CloseBulk(THandle handle)
{
	return m_device->CloseBulk(handle) == 0;
}

bool RageVFSDevice::RemoveFile(const std::string& filename)
{
	return m_device->RemoveFile(filename.substr(m_pathPrefixLength).c_str());
}

bool RageVFSDevice::RenameFile(const std::string& from, const std::string& to)
{
	m_device->RenameFile(from.substr(m_pathPrefixLength).c_str(), to.substr(m_pathPrefixLength).c_str());
	return true; // should ignore return value
}

bool RageVFSDevice::RemoveDirectory(const std::string& name)
{
	return m_device->RemoveDirectory(name.substr(m_pathPrefixLength).c_str());
}

bool RageVFSDevice::CreateDirectory(const std::string& name)
{
	return m_device->CreateDirectory(name.substr(m_pathPrefixLength).c_str());
}

size_t RageVFSDevice::GetLength(THandle handle)
{
#ifndef GTA_NY
	return m_device->GetFileLengthUInt64(handle);
#else
	return m_device->GetFileLength(handle);
#endif
}

size_t RageVFSDevice::GetLength(const std::string& fileName)
{
	return m_device->GetFileLengthLong(fileName.substr(m_pathPrefixLength).c_str());
}

uint32_t RageVFSDevice::GetAttributes(const std::string& fileName)
{
	return m_device->GetFileAttributes(fileName.substr(m_pathPrefixLength).c_str());
}

THandle RageVFSDevice::FindFirst(const std::string& folder, vfs::FindData* findData)
{
	rage::fiFindData findDataOrig;
	auto handle = m_device->FindFirst(folder.substr(m_pathPrefixLength).c_str(), &findDataOrig);

	if (handle != INVALID_DEVICE_HANDLE)
	{
		findData->name = findDataOrig.fileName;
		findData->attributes = findDataOrig.fileAttributes;
		findData->length = findDataOrig.fileSize;
	}

	return handle;
}

bool RageVFSDevice::FindNext(THandle handle, vfs::FindData* findData)
{
	rage::fiFindData findDataOrig;
	bool valid = m_device->FindNext(handle, &findDataOrig);

	if (valid)
	{
		findData->name = findDataOrig.fileName;
		findData->attributes = findDataOrig.fileAttributes;
		findData->length = findDataOrig.fileSize;
	}

	return valid;
}

void RageVFSDevice::FindClose(THandle handle)
{
	m_device->FindClose(handle);
}

bool RageVFSDevice::IsCollection()
{
#ifndef GTA_NY
	return m_device->IsCollection();
#else
	return false;
#endif
}

void RageVFSDevice::SetPathPrefix(const std::string& pathPrefix)
{
	m_pathPrefixLength = pathPrefix.length();
}

bool RageVFSDevice::ExtensionCtl(int controlIdx, void* controlData, size_t controlSize)
{
	if (controlIdx == VFS_FLUSH_BUFFERS)
	{
		auto data = reinterpret_cast<vfs::FlushBuffersExtension*>(controlData);

		return FlushFileBuffers(reinterpret_cast<HANDLE>(data->handle));
	}
#ifndef GTA_NY
	else if (controlIdx == VFS_GET_RAGE_PAGE_FLAGS)
	{
		auto data = (GetRagePageFlagsExtension*)controlData;

		std::string fileName = data->fileName;
		data->version = m_device->GetResourceVersion(fileName.substr(m_pathPrefixLength).c_str(), &data->flags);

		return true;
	}
#endif

	return false;
}

#ifndef GTA_NY
bool RageVFSDeviceAdapter::IsCollection()
{
	if (m_collectionCache)
	{
		return *m_collectionCache;
	}

	try
	{
		auto rageDevice = dynamic_cast<RageVFSDevice*>(m_cfxDevice.GetRef());

		if (rageDevice != nullptr)
		{
			bool isCollection = rageDevice->IsCollection();
			m_collectionCache = isCollection;
			
			return isCollection;
		}
	}
	catch (std::bad_typeid)
	{

	}

	m_collectionCache = false;

	return false;
}
#endif

#include <mutex>

class RageVFSManager : public vfs::Manager
{
private:
	std::unordered_map<rage::fiDevice*, fwRefContainer<vfs::Device>> m_deviceCache;
	
	std::multimap<std::string, RageVFSDeviceAdapter*> m_mountedDevices;

	std::recursive_mutex m_managerLock;

public:
	virtual fwRefContainer<vfs::Device> GetDevice(const std::string& path) override;

	virtual fwRefContainer<vfs::Device> GetNativeDevice(void* nativeDevice) override;

	virtual void Mount(fwRefContainer<vfs::Device> device, const std::string& path) override;

	virtual void Unmount(const std::string& path) override;
};

static std::multimap<std::string, fwRefContainer<vfs::Device>> g_mountCache;

namespace vfs
{
fwRefContainer<Device> MakeMemoryDevice();
}

fwRefContainer<vfs::Device> RageVFSManager::GetDevice(const std::string& path)
{
	std::unique_lock<std::recursive_mutex> lock(m_managerLock);

	if (!g_vfsInitDone)
	{
		// logic from server device
		for (const auto& mount : g_mountCache)
		{
			// if the prefix patches
			if (strncmp(path.c_str(), mount.first.c_str(), mount.first.length()) == 0)
			{
				vfs::Device::THandle handle;

				auto device = mount.second;

				if ((handle = device->Open(path, true)) != vfs::Device::InvalidHandle)
				{
					device->Close(handle);

					return device;
				}
			}
		}

		if (path.find("citizen:/") == 0)
		{
			static fwRefContainer<vfs::RelativeDevice> g_citizenDevice = new vfs::RelativeDevice(ToNarrow(MakeRelativeCitPath(L"citizen/")));
			g_citizenDevice->SetPathPrefix("citizen:/");

			return g_citizenDevice;
		}
	}

	if (!g_vfsInitStarted)
	{
		if (_strnicmp(path.c_str(), "memory:", 7) == 0)
		{
			static fwRefContainer<vfs::Device> memoryDevice = vfs::MakeMemoryDevice();

			return memoryDevice;
		}

		static fwRefContainer<vfs::LocalDevice> g_localDevice = new vfs::LocalDevice;
		return g_localDevice;
	}

	rage::fiDevice* nativeDevice = rage::fiDevice::GetDevice(path.c_str(), true);

	return (nativeDevice) ? GetNativeDevice(nativeDevice) : nullptr;
}

fwRefContainer<vfs::Device> RageVFSManager::GetNativeDevice(void* nativeDevice)
{
	std::unique_lock<std::recursive_mutex> lock(m_managerLock);

	rage::fiDevice* nativeDevicePtr = reinterpret_cast<rage::fiDevice*>(nativeDevice);

	auto it = m_deviceCache.find(nativeDevicePtr);

	if (it == m_deviceCache.end())
	{
		it = m_deviceCache.insert({ nativeDevicePtr, new RageVFSDevice(reinterpret_cast<rage::fiDevice*>(nativeDevice)) }).first;
	}

	return it->second;
}

static std::queue<std::function<void()>> g_onInitQueue;

void RageVFSManager::Mount(fwRefContainer<vfs::Device> device, const std::string& path)
{
	std::unique_lock<std::recursive_mutex> lock(m_managerLock);

	auto run = [this, path, device]()
	{
		auto adapter = new RageVFSDeviceAdapter(device);

		m_mountedDevices.insert({ path, adapter });

		// track the owner of the VFS device so it won't have to go
		// through a VFS adapter (which loses refcounts) if the caller is the VFS.
		m_deviceCache.insert({ adapter, device });

		// ensure the allocator is defined
		rage::sysMemAllocator::UpdateAllocatorValue();

		rage::fiDevice::MountGlobal(path.c_str(), adapter, true);
	};

	if (g_vfsInitStarted)
	{
		run();
	}
	else
	{
		g_mountCache.insert({ path, device });

		g_onInitQueue.push(run);
	}

	device->SetPathPrefix(path);
}

void RageVFSManager::Unmount(const std::string& path)
{
	std::unique_lock<std::recursive_mutex> lock(m_managerLock);

	// ensure the allocator is defined
	rage::sysMemAllocator::UpdateAllocatorValue();

	rage::fiDevice::Unmount(path.c_str());

	// destroy all adapters
	for (auto& entry : fx::GetIteratorView(m_mountedDevices.equal_range(path)))
	{
		m_deviceCache.erase(entry.second);
		delete entry.second;
	}

	m_mountedDevices.erase(path);
}

static InitFunction initFunction([]()
{
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		g_vfsInitStarted = true;
	}, INT32_MIN);

	rage::fiDevice::OnInitialMount.Connect([]()
	{
		std::function<void()> fn;

		while (!g_onInitQueue.empty())
		{
			fn = std::move(g_onInitQueue.front());
			g_onInitQueue.pop();

			fn();
		}

		g_mountCache.clear();
		g_vfsInitDone = true;
	}, 1);

	Instance<vfs::Manager>::Set(new RageVFSManager());
});
