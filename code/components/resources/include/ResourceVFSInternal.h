#pragma once

#include "ResourceVFS.h"

using rage::fiDevice;

class RageVFSStream : public VFSStream
{
private:
	fiDevice* m_device;

	uint32_t m_handle;

	bool m_canWrite;

public:
	RageVFSStream(fiDevice* device, uint32_t handle, bool canWrite);

	virtual ~RageVFSStream();

	virtual bool CanRead();

	virtual bool CanWrite();

	virtual bool CanSeek();

	virtual size_t Read(void* buffer, size_t length);

	virtual size_t Write(const void* buffer, size_t length);

	virtual void Seek(int position, int seekOrigin);

	virtual size_t GetLength();
};