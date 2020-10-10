#include "StdInc.h"
#include <VFSDevice.h>
#include <VFSManager.h>

namespace vfs
{
class MemoryDevice : public Device
{
public:
	virtual ~MemoryDevice() override;

	virtual THandle Open(const std::string& fileName, bool readOnly) override;

	virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr) override;

	virtual size_t Read(THandle handle, void* outBuffer, size_t size) override;

	virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size) override;

	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override;

	virtual bool Close(THandle handle) override;

	virtual bool CloseBulk(THandle handle) override;

	virtual size_t GetLength(THandle handle) override;

	virtual size_t GetLength(const std::string& fileName) override;

	virtual THandle FindFirst(const std::string& folder, FindData* findData) override;

	virtual bool FindNext(THandle handle, FindData* findData) override;

	virtual void FindClose(THandle handle) override;

	// Sets the path prefix for the device, which implementations should strip for generating a local path portion.
	virtual void SetPathPrefix(const std::string& pathPrefix) override;
};

struct MemoryHandle
{
	const char* buffer;
	size_t offset;
	uint32_t length;
};

MemoryDevice::~MemoryDevice()
{

}

Device::THandle MemoryDevice::Open(const std::string& fileName, bool readOnly)
{
	if (!readOnly)
	{
		return InvalidHandle;
	}

	char* basePointer;
	uint32_t size;

	if (sscanf(fileName.c_str(), "memory:$%p,%d", &basePointer, &size) != 2)
	{
		return InvalidHandle;
	}

	MemoryHandle* handle = new MemoryHandle;
	handle->buffer = basePointer;
	handle->length = size;
	handle->offset = 0;

	return reinterpret_cast<THandle>(handle);
}

Device::THandle MemoryDevice::OpenBulk(const std::string& fileName, uint64_t* ptr)
{
	*ptr = 0;

	return Open(fileName, true);
}

size_t MemoryDevice::Read(THandle handle, void* outBuffer, size_t size)
{
	MemoryHandle* hndl = reinterpret_cast<MemoryHandle*>(handle);

	// copy data
	size_t toRead = std::min(size, hndl->length - hndl->offset);
	memcpy(outBuffer, hndl->buffer + hndl->offset, toRead);

	hndl->offset += toRead;

	return toRead;
}

size_t MemoryDevice::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	MemoryHandle* hndl = reinterpret_cast<MemoryHandle*>(handle);

	// copy data
	size_t toRead = std::min(size, static_cast<size_t>(hndl->length - ptr));
	memcpy(outBuffer, hndl->buffer + ptr, toRead);

	return toRead;
}

size_t MemoryDevice::Seek(THandle handle, intptr_t offset, int seekType)
{
	MemoryHandle* hndl = reinterpret_cast<MemoryHandle*>(handle);

	size_t newOffset = hndl->offset;

	switch (seekType)
	{
		case SEEK_SET:
			newOffset = std::max(static_cast<intptr_t>(0), std::min(static_cast<intptr_t>(hndl->length), offset));
			break;

		case SEEK_CUR:
			newOffset = std::max(static_cast<intptr_t>(0), std::min(static_cast<intptr_t>(hndl->length), static_cast<intptr_t>(hndl->offset) + offset));
			break;

		case SEEK_END:
			newOffset = std::max(static_cast<intptr_t>(0), std::min(static_cast<intptr_t>(hndl->length), static_cast<intptr_t>(hndl->length) - offset));
			break;
	}

	hndl->offset = newOffset;

	return newOffset;
}

bool MemoryDevice::Close(THandle handle)
{
	MemoryHandle* hndl = reinterpret_cast<MemoryHandle*>(handle);
	delete hndl;

	return true;
}

bool MemoryDevice::CloseBulk(THandle handle)
{
	return Close(handle);
}

size_t MemoryDevice::GetLength(THandle handle)
{
	MemoryHandle* hndl = reinterpret_cast<MemoryHandle*>(handle);
	return hndl->length;
}

size_t MemoryDevice::GetLength(const std::string& fileName)
{
	char* basePointer;
	uint32_t size;

	if (sscanf(fileName.c_str(), "memory:$%p,%d", &basePointer, &size) != 2)
	{
		return -1;
	}

	return size;
}

Device::THandle MemoryDevice::FindFirst(const std::string& folder, FindData* findData)
{
	return InvalidHandle;
}

bool MemoryDevice::FindNext(THandle handle, FindData* findData)
{
	return false;
}

void MemoryDevice::FindClose(THandle handle)
{

}

void MemoryDevice::SetPathPrefix(const std::string& pathPrefix)
{

}

std::string MakeMemoryFilename(const void* buffer, size_t size)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "memory:$%p,%d", buffer, static_cast<uint32_t>(size));

	return buf;
}

fwRefContainer<Device> MakeMemoryDevice()
{
	return new MemoryDevice();
}
}
