#pragma once

#define VFS_GET_DEVICE_LAST_ERROR 0x10004

#include <VFSDevice.h>

namespace vfs
{
	struct GetLastErrorExtension
	{
		std::string outError;
	};

	inline std::string GetLastError(const fwRefContainer<vfs::Device>& device)
	{
		GetLastErrorExtension ext;
		ext.outError = "Unsupported GetLastError call";

		device->ExtensionCtl(VFS_GET_DEVICE_LAST_ERROR, &ext, sizeof(ext));

		return ext.outError;
	}
}
