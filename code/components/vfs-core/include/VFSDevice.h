/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_VFS_CORE
#define VFS_CORE_EXPORT DLL_EXPORT
#else
#define VFS_CORE_EXPORT DLL_IMPORT
#endif

#include <array>
#include <ctime>

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x10
#endif

namespace vfs
{
struct FindData
{
	std::string name;
	uint32_t attributes;
	size_t length;
};

#define INVALID_DEVICE_HANDLE (vfs::Device::InvalidHandle)

class VFS_CORE_EXPORT Device : public fwRefCountable
{
public:
	typedef uintptr_t THandle;

	static const THandle InvalidHandle = -1;

public:
	virtual THandle Open(const std::string& fileName, bool readOnly, bool append = false) = 0;

	virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr);

	virtual THandle Create(const std::string& filename, bool createIfExists = true, bool append = false);

	virtual size_t Read(THandle handle, void* outBuffer, size_t size) = 0;

	virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size);

	virtual size_t Write(THandle handle, const void* buffer, size_t size);

	virtual size_t WriteBulk(THandle handle, uint64_t ptr, const void* buffer, size_t size);

	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) = 0;

	virtual bool Close(THandle handle) = 0;

	virtual bool CloseBulk(THandle handle);

	virtual bool RemoveFile(const std::string& filename);

	virtual bool RenameFile(const std::string& from, const std::string& to);

	virtual bool CreateDirectory(const std::string& name);

	virtual bool RemoveDirectory(const std::string& name);

	virtual uint32_t GetAttributes(const std::string& filename);

	virtual size_t GetLength(THandle handle);

	virtual size_t GetLength(const std::string& fileName);

	virtual std::time_t GetModifiedTime(const std::string& fileName);

	virtual THandle FindFirst(const std::string& folder, FindData* findData) = 0;

	virtual bool FindNext(THandle handle, FindData* findData) = 0;

	virtual void FindClose(THandle handle) = 0;

	virtual void SetPathPrefix(const std::string& pathPrefix);

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize);

	virtual std::string GetAbsolutePath() const = 0;

	virtual bool Flush(THandle handle) = 0;
};

#define VFS_FLUSH_BUFFERS 0x10001

struct FlushBuffersExtension
{
	Device::THandle handle;
};

using FileId = std::array<uint8_t, 16>;

#define VFS_GET_FILE_ID 0x10003

struct GetFileIdExtension
{
	// in
	Device::THandle handle;

	// out
	FileId fileId;
};
}
