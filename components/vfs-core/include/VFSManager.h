/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <VFSDevice.h>
#include <VFSStream.h>

#ifdef COMPILING_VFS_CORE
#define VFS_CORE_EXPORT DLL_EXPORT
#else
#define VFS_CORE_EXPORT DLL_IMPORT
#endif

namespace vfs
{
class VFS_CORE_EXPORT Manager : public fwRefCountable
{
public:
	virtual fwRefContainer<Stream> OpenRead(const std::string& path);

	virtual fwRefContainer<Device> GetDevice(const std::string& path) = 0;

	virtual void Mount(fwRefContainer<Device> device, const std::string& path) = 0;

	virtual void Unmount(const std::string& path) = 0;

	virtual fwRefContainer<Device> GetNativeDevice(void* nativeDevice);
};

VFS_CORE_EXPORT fwRefContainer<Stream> OpenRead(const std::string& path);

VFS_CORE_EXPORT fwRefContainer<Device> GetDevice(const std::string& path);

VFS_CORE_EXPORT fwRefContainer<Device> GetNativeDevice(void* nativeDevice);

VFS_CORE_EXPORT void Mount(fwRefContainer<Device> device, const std::string& path);

VFS_CORE_EXPORT void Unmount(const std::string& path);
}

DECLARE_INSTANCE_TYPE(vfs::Manager);