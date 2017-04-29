#pragma once

#include "ResourceManager.h"

class VFSStream : public fwRefCountable
{
public:
	virtual ~VFSStream() = 0;

	virtual bool CanRead() = 0;

	virtual bool CanWrite() = 0;

	virtual bool CanSeek() = 0;

	virtual size_t Read(void* buffer, size_t length) = 0;

	virtual size_t Write(const void* buffer, size_t length) = 0;

	virtual void Seek(int position, int seekOrigin) = 0;

	virtual size_t GetLength() = 0;
};

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#endif
	ResourceVFS
{
private:
	ResourceVFS() {}
public:
	static fwRefContainer<VFSStream> OpenFileRead(const char* filePath, const ResourceIdentity& sourceIdentity);

	static fwRefContainer<VFSStream> OpenFileWrite(const char* filePath, const ResourceIdentity& sourceIdentity);

	static fwRefContainer<VFSStream> OpenFileAppend(const char* filePath, const ResourceIdentity& sourceIdentity);
};