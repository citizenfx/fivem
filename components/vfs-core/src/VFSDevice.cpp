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
uint64_t Device::OpenBulk(const std::string& fileName, uint64_t* ptr)
{
	return INVALID_DEVICE_HANDLE;
}

size_t Device::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	return INVALID_DEVICE_HANDLE;
}

bool Device::CloseBulk(THandle handle)
{
	return false;
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

void Device::SetPathPrefix(const std::string& pathPrefix)
{

}
}