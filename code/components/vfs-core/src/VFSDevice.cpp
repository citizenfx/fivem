/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <VFSDevice.h>

namespace vfs
{
auto Device::OpenBulk(const std::string& fileName, uint64_t* ptr) -> THandle
{
	return INVALID_DEVICE_HANDLE;
}

auto Device::Create(const std::string& filename, bool createIfExists, bool append) -> THandle
{
	return INVALID_DEVICE_HANDLE;
}

size_t Device::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	return INVALID_DEVICE_HANDLE;
}

size_t Device::Write(THandle handle, const void* buffer, size_t size)
{
	return INVALID_DEVICE_HANDLE;
}

size_t Device::WriteBulk(THandle handle, uint64_t ptr, const void* buffer, size_t size)
{
	return INVALID_DEVICE_HANDLE;
}

bool Device::CloseBulk(THandle handle)
{
	return false;
}

bool Device::CreateDirectory(const std::string& name)
{
	return false;
}

bool Device::RemoveDirectory(const std::string& name)
{
	return false;
}

bool Device::RemoveFile(const std::string& name)
{
	return false;
}

bool Device::RenameFile(const std::string& from, const std::string& to)
{
	return false;
}

std::time_t Device::GetModifiedTime(const std::string& fileName)
{
	return 0;
}

size_t Device::GetLength(const std::string& fileName)
{
	auto handle = Open(fileName, true);
	size_t retval = -1;

	if (handle != INVALID_DEVICE_HANDLE)
	{
		retval = GetLength(handle);

		Close(handle);
	}

	return retval;
}

size_t Device::GetLength(THandle handle)
{
	size_t curOffset = Seek(handle, 0, SEEK_CUR);
	size_t retval = Seek(handle, 0, SEEK_END);
	Seek(handle, curOffset, SEEK_SET);

	return retval;
}

uint32_t Device::GetAttributes(const std::string& filename)
{
	uint32_t attributes = -1;

	uint64_t handle = Open(filename, true);

	if (handle != -1)
	{
		attributes = 0;

		Close(handle);
	}

	return attributes;
}

void Device::SetPathPrefix(const std::string& pathPrefix)
{

}

bool Device::ExtensionCtl(int controlIdx, void* controlData, size_t controlSize)
{
	return false;
}

bool Device::Flush(THandle handle)
{
	return false;	
}
}
