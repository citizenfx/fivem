/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <VFSManager.h>

namespace vfs
{
fwRefContainer<Stream> Manager::OpenRead(const std::string& path)
{
	auto device = GetDevice(path);

	if (device.GetRef())
	{
		auto handle = device->Open(path, true);

		if (handle != INVALID_DEVICE_HANDLE)
		{
			return new Stream(device, handle);
		}
	}

	return nullptr;
}

fwRefContainer<Stream> Manager::OpenWrite(const std::string& path, const bool append)
{
	auto device = GetDevice(path);

	if (device.GetRef())
	{
		auto handle = device->Open(path, false, append);

		if (handle != INVALID_DEVICE_HANDLE)
		{
			return new Stream(device, handle);
		}
	}

	return nullptr;
}

bool Manager::RemoveFile(const std::string& path)
{
	auto device = GetDevice(path);

	if (device.GetRef())
	{
		return device->RemoveFile(path);
	}

	return false;
}

bool Manager::RenameFile(const std::string& from, const std::string& to)
{
	auto device = GetDevice(from);

	if (device.GetRef())
	{
		return device->RenameFile(from, to);
	}

	return false;
}

fwRefContainer<Stream> Manager::Create(const std::string& path, const bool createIfExists, const bool append)
{
	auto device = GetDevice(path);

	if (device.GetRef())
	{
		auto handle = device->Create(path, createIfExists, append);

		if (handle != INVALID_DEVICE_HANDLE)
		{
			return new Stream(device, handle);
		}
	}

	return nullptr;
}

fwRefContainer<Device> Manager::GetNativeDevice(void* nativeDevice)
{
	return nullptr;
}

fwRefContainer<Stream> OpenRead(const std::string& path)
{
	return Instance<Manager>::Get()->OpenRead(path);
}

fwRefContainer<Stream> OpenWrite(const std::string& path, const bool append)
{
	return Instance<Manager>::Get()->OpenWrite(path, append);
}

bool RenameFile(const std::string& from, const std::string& to)
{
	return Instance<Manager>::Get()->RenameFile(from, to);
}

bool RemoveFile(const std::string& path)
{
	return Instance<Manager>::Get()->RemoveFile(path);
}

fwRefContainer<Stream> Create(const std::string& path, const bool createIfExists, const bool append)
{
	return Instance<Manager>::Get()->Create(path, createIfExists, append);
}

fwRefContainer<Device> GetDevice(const std::string& path)
{
	return Instance<Manager>::Get()->GetDevice(path);
}

fwRefContainer<Device> FindDevice(const std::string& absolutePath, std::string& transformedPath)
{
	return Instance<Manager>::Get()->FindDevice(absolutePath, transformedPath);
}

fwRefContainer<Device> GetNativeDevice(void* nativeDevice)
{
	return Instance<Manager>::Get()->GetNativeDevice(nativeDevice);
}

void Mount(fwRefContainer<Device> device, const std::string& path)
{
	return Instance<Manager>::Get()->Mount(device, path);
}

void Unmount(const std::string& path)
{
	return Instance<Manager>::Get()->Unmount(path);
}
}
