/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <fiDevice.h>

#ifdef COMPILING_RAGE_DEVICE_RDR3
#define DEVICE_EXPORT __declspec(dllexport)
#define DEVICE_IMPORT
#else
#define DEVICE_IMPORT __declspec(dllimport)
#define DEVICE_EXPORT
#endif

namespace rage
{
//
// Base class to base custom fiDevice classes off of - will use RAGE's native fiDevice base virtual table as base.
//
class DEVICE_EXPORT __declspec(novtable) fiCustomDevice : public fiDevice
{
private:
	uint8_t m_parentDeviceData[16];

	rage::fiDevice* m_parentDeviceRef;

public:
	fiCustomDevice();

	virtual ~fiCustomDevice();

	virtual uint64_t OpenFlags(const char* fileName, bool readOnly, uint32_t shareFlags, uint32_t openFlags) override;

	virtual uint64_t OpenHash(uint32_t* hashValue) override;

	virtual uint64_t OpenHashExt(uint32_t* hashValue, int extension) override;

	virtual uint64_t Open(const char* fileName, bool readOnly) override;

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) override;

	// OpenBulkOverlapped
	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) override;

	// CreateBulk
	virtual uint64_t CreateLocal(const char* fileName) override;

	virtual uint64_t CreateFlags(const char* fileName, uint32_t shareFlags, uint32_t openFlags) override;

	virtual uint64_t Create(const char* fileName) override;

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) override;

	virtual int ReadFile(const char* path, void* buffer, int size) override;

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override;

	virtual uint32_t ReadBulkOverlapped(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) override;

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int) override;

	virtual uint32_t Write(uint64_t, void*, int) override;

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) override;

	// Seek64
	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) override;

	virtual int32_t Close(uint64_t handle) override;

	virtual int32_t CloseBulk(uint64_t handle) override;

	virtual int32_t CloseBulkOverlapped(uint64_t handle) override;

	// Size
	virtual int GetFileLength(uint64_t handle) override;

	// Size64
	virtual uint64_t GetFileLengthUInt64(uint64_t handle) override;

	// dummy!
	virtual int Flush(int) override;

	// Delete
	virtual bool RemoveFile(const char* file) override;

	// Rename
	virtual int RenameFile(const char* from, const char* to) override;

	virtual void RenameWithProgress(void*) override;

	virtual uint64_t GetAvailableDiskSpace(const char*) override;

	// MakeDirectory
	virtual int CreateDirectory(const char* dir) override;

	// UnmakeDirectory
	virtual int RemoveDirectory(const char* dir) override;

	virtual void Sanitize() override;

	virtual int64_t GetFileSize(const atArray<const char*>& files, atArray<uint64_t>& sizes) override;

	// GetFileSize
	virtual uint64_t GetFileLengthLong(const char* fileName) override;

	virtual int64_t GetFileTime(const atArray<const char*>& files, atArray<uint64_t>& times) override;

	virtual uint64_t GetFileTime(const char* file) override;
	virtual bool SetFileTime(const char* file, FILETIME fileTime) override;

	virtual uint64_t FindFileBegin(const char* path, const char* pattern, fiFindData* findData) override;

	// FindFileBegin
	virtual uint64_t FindFirst(const char* path, fiFindData* findData) override;

	// FindFileNext
	virtual bool FindNext(uint64_t handle, fiFindData* findData) override;

	// FindFileClose
	virtual int FindClose(uint64_t handle) override;

	virtual rage::fiDevice* GetLowLevelDevice() override;

	virtual void* m_xy(void*, int, void*) override;

	// SetEndOfFile
	virtual bool Truncate(uint64_t handle) override;

	// GetAttributes
	virtual uint32_t GetFileAttributes(const char* path) override;

	//virtual bool m_xz() override;

	// SetAttributes
	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes) override;

	// GetRootDeviceId
	virtual int m_yx() override;

	virtual bool IsMemoryMappedDevice() override;

	// read even if read() returns less than length
	// SafeRead
	virtual bool ReadFull(uint64_t handle, void* buffer, uint32_t length) override;

	// SafeWrite
	virtual bool WriteFull(uint64_t handle, void* buffer, uint32_t length) override;

	virtual int32_t GetResourceVersion(const atArray<const char*>& files, atArray<ResourceFlags>& flags) override;

	virtual int32_t GetResourceVersion(const char* fileName, ResourceFlags* flags) override;

	virtual int32_t GetEncryptionKey() override;

	virtual int32_t IsValidHandle(uint64_t) override;

	virtual int64_t GetBulkOffset(const char*) override;

	virtual int32_t GetPhysicalSortKey(void*) override; // return 0x40000000

	// IsRpf
	virtual bool IsCollection() override;

	virtual int GetRpfVersion() override;

	// GetRpfDevice
	virtual fiDevice* GetCollection() override; // return this

	virtual bool IsCloud() override;

	virtual bool IsZip() override;

	// GetPackfileIndex
	virtual int32_t GetCollectionId() override;

	// GetDebugName
	virtual const char* GetName() override;

	virtual bool SupportsOverlappedIO() override;

	virtual uint32_t GetClampedBufferSize(uint64_t, int) override;

	virtual uint64_t GetFinalOffset(uint64_t) override;

	virtual bool IsOverlappedRequestFinished(uint64_t, int) override;
};
}
