/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceVFSInternal.h"

VFSStream::~VFSStream()
{

}

RageVFSStream::RageVFSStream(fiDevice* device, uint32_t handle, bool canWrite)
	: m_device(device), m_handle(handle), m_canWrite(canWrite)
{
	
}

RageVFSStream::~RageVFSStream()
{
	m_device->close(m_handle);
}

bool RageVFSStream::CanRead()
{
	return true;
}

bool RageVFSStream::CanSeek()
{
	return true;
}

bool RageVFSStream::CanWrite()
{
	return m_canWrite;
}

size_t RageVFSStream::GetLength()
{
	return m_device->fileLength(m_handle);
}

size_t RageVFSStream::Read(void* buffer, size_t length)
{
	return m_device->read(m_handle, buffer, length);
}

size_t RageVFSStream::Write(const void* buffer, size_t length)
{
	return m_device->write(m_handle, const_cast<void*>(buffer), length);
}

void RageVFSStream::Seek(int position, int seekOrigin)
{
	m_device->seek(m_handle, position, seekOrigin);
}