/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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