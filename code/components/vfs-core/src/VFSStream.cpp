/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <VFSStream.h>

namespace vfs
{
Stream::Stream(fwRefContainer<Device> device, Device::THandle handle)
	: m_device(device), m_handle(handle)
{

}

Stream::~Stream()
{
	Close();
}

size_t Stream::Read(void* buffer, size_t length)
{
	return m_device->Read(m_handle, buffer, length);
}

std::vector<uint8_t> Stream::Read(size_t length)
{
	std::vector<uint8_t> retval(length);
	length = Read(retval);

	retval.resize(length);

	return retval;
}

size_t Stream::Write(const void* buffer, size_t length)
{
	return m_device->Write(m_handle, buffer, length);
}

uint64_t Stream::GetLength()
{
	return m_device->GetLength(m_handle);
}

size_t Stream::Seek(intptr_t offset, int seekType)
{
	return m_device->Seek(m_handle, offset, seekType);
}

void Stream::Close()
{
	if (m_handle != INVALID_DEVICE_HANDLE)
	{
		m_device->Close(m_handle);
		m_handle = INVALID_DEVICE_HANDLE;
	}
}

std::vector<uint8_t> Stream::ReadToEnd()
{
	size_t fileLength = m_device->GetLength(m_handle);
	size_t curSize = Seek(0, SEEK_CUR);
	
	return Read(fileLength - curSize);
}
}