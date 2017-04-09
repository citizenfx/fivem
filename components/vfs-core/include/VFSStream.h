/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <VFSDevice.h>

#ifdef COMPILING_VFS_CORE
#define VFS_CORE_EXPORT DLL_EXPORT
#else
#define VFS_CORE_EXPORT DLL_IMPORT
#endif

namespace vfs
{
class VFS_CORE_EXPORT Stream : public fwRefCountable
{
private:
	fwRefContainer<Device> m_device;

	Device::THandle m_handle;

public:
	Stream(fwRefContainer<Device> device, Device::THandle handle);

	virtual ~Stream();

	std::vector<uint8_t> Read(size_t length);

	size_t Read(void* buffer, size_t length);

	template<typename TAlloc>
	inline size_t Read(std::vector<uint8_t, TAlloc>& buffer)
	{
		return Read(&buffer[0], buffer.size());
	}

	size_t Write(const void* buffer, size_t length);

	template<typename TAlloc>
	inline size_t Write(const std::vector<uint8_t, TAlloc>& buffer)
	{
		return Write(&buffer[0], buffer.size());
	}

	void Close();

	uint64_t GetLength();

	size_t Seek(intptr_t offset, int seekType);

	std::vector<uint8_t> ReadToEnd();

	inline Device::THandle GetHandle()
	{
		return m_handle;
	}
};
}