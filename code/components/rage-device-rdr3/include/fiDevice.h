#pragma once

#include "sysAllocator.h"

#ifdef COMPILING_RAGE_DEVICE_RDR3
#define DEVICE_EXPORT __declspec(dllexport)
#define DEVICE_IMPORT
#else
#define DEVICE_IMPORT __declspec(dllimport)
#define DEVICE_EXPORT
#endif

#include <array>
#include <atArray.h>

namespace rage
{
struct fiFindData
{
	char fileName[256];
	uint64_t fileSize;
	FILETIME lastWriteTime;
	DWORD fileAttributes;
};

struct ResourceFlags
{
	// TODO: figure out which is physical and which is virtual
	uint32_t flag1;
	uint32_t flag2;
};

class DEVICE_EXPORT __declspec(novtable) fiDevice : public sysUseAllocator
{
public:
	static fiDevice* GetDevice(const char* path, bool allowRoot);

	static bool MountGlobal(const char* mountPoint, fiDevice* device, bool allowRoot);

	static void Unmount(const char* rootPath);

	static DEVICE_IMPORT fwEvent<> OnInitialMount;

public:
	virtual ~fiDevice() = 0;

	virtual uint64_t OpenFlags(const char* fileName, bool readOnly, uint32_t shareFlags, uint32_t openFlags) = 0;

	virtual uint64_t OpenHash(uint32_t* hashValue) = 0;

	virtual uint64_t OpenHashExt(uint32_t* hashValue, int extension) = 0;

	virtual uint64_t Open(const char* fileName, bool readOnly) = 0;

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) = 0;

	// OpenBulkOverlapped
	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) = 0;

	// CreateBulk
	virtual uint64_t CreateLocal(const char* fileName) = 0;

	virtual uint64_t CreateFlags(const char* fileName, uint32_t shareFlags, uint32_t openFlags) = 0;

	virtual uint64_t Create(const char* fileName) = 0;

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) = 0;

	virtual int ReadFile(const char* path, void* buffer, int size) = 0;

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t ReadBulkOverlapped(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int) = 0;

	virtual uint32_t Write(uint64_t, void*, int) = 0;

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) = 0;

	// Seek64
	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) = 0;

	virtual int32_t Close(uint64_t handle) = 0;

	virtual int32_t CloseBulk(uint64_t handle) = 0;

	virtual int32_t CloseBulkOverlapped(uint64_t handle) = 0;

	// Size
	virtual int GetFileLength(uint64_t handle) = 0;

	// Size64
	virtual uint64_t GetFileLengthUInt64(uint64_t handle) = 0;

	// dummy!
	virtual int Flush(int) = 0;

	// Delete
	virtual bool RemoveFile(const char* file) = 0;

	// Rename
	virtual int RenameFile(const char* from, const char* to) = 0;

	virtual void RenameWithProgress(void*) = 0;

	// MakeDirectory
	virtual int CreateDirectory(const char* dir) = 0;

	// UnmakeDirectory
	virtual int RemoveDirectory(const char* dir) = 0;

	virtual uint64_t GetAvailableDiskSpace(const char*) = 0;

	virtual void Sanitize() = 0;

	virtual int64_t GetFileSize(const atArray<const char*>& files, atArray<uint64_t>& sizes) = 0;

	// GetFileSize
	virtual uint64_t GetFileLengthLong(const char* fileName) = 0;

	virtual int64_t GetFileTime(const atArray<const char*>& files, atArray<uint64_t>& times) = 0;

	virtual uint64_t GetFileTime(const char* file) = 0;
	virtual bool SetFileTime(const char* file, FILETIME fileTime) = 0;

	virtual uint64_t FindFileBegin(const char* path, const char* pattern, fiFindData* findData) = 0;

	// FindFileBegin
	virtual uint64_t FindFirst(const char* path, fiFindData* findData) = 0;

	// FindFileNext
	virtual bool FindNext(uint64_t handle, fiFindData* findData) = 0;

	// FindFileClose
	virtual int FindClose(uint64_t handle) = 0;

	virtual rage::fiDevice* GetLowLevelDevice() = 0;

	virtual void* m_xy(void*, int, void*) = 0;

	// SetEndOfFile
	virtual bool Truncate(uint64_t handle) = 0;

	// GetAttributes
	virtual uint32_t GetFileAttributes(const char* path) = 0;

	//virtual bool m_xz() = 0;

	// SetAttributes
	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes) = 0;

	// GetRootDeviceId
	virtual int m_yx() = 0;

	virtual bool IsMemoryMappedDevice() = 0;

	// read even if read() returns less than length
	// SafeRead
	virtual bool ReadFull(uint64_t handle, void* buffer, uint32_t length) = 0;

	// SafeWrite
	virtual bool WriteFull(uint64_t handle, void* buffer, uint32_t length) = 0;

	virtual int32_t GetResourceVersion(const atArray<const char*>& files, atArray<ResourceFlags>& flags) = 0;

	virtual int32_t GetResourceVersion(const char* fileName, ResourceFlags* flags) = 0;

	virtual int32_t GetEncryptionKey() = 0;

	virtual int32_t IsValidHandle(uint64_t) = 0;

	virtual int64_t GetBulkOffset(const char*) = 0;

	virtual int32_t GetPhysicalSortKey(void*) = 0; // return 0x40000000

	// IsRpf
	virtual bool IsCollection() = 0;

	virtual int GetRpfVersion() = 0;

	// GetRpfDevice
	virtual fiDevice* GetCollection() = 0; // return this

	virtual bool IsCloud() = 0;

	virtual bool IsZip() = 0;

	// GetPackfileIndex
	virtual int32_t GetCollectionId() = 0;

	// GetDebugName
	virtual const char* GetName() = 0;

	virtual bool SupportsOverlappedIO() = 0;

	virtual uint32_t GetClampedBufferSize(uint64_t, int) = 0;

	virtual uint64_t GetFinalOffset(uint64_t) = 0;

	virtual bool IsOverlappedRequestFinished(uint64_t, int) = 0;

	/*virtual uint64_t m_84(int a1) = 0;

	virtual uint32_t m_88(int a1) = 0;

	virtual bool m_8C() = 0;

	virtual int m_90() = 0;

	virtual const char* getName() = 0;

	virtual rage::fiDevice* getUnkDevice() = 0;

	virtual int m_9C(int, int, int) = 0;

	virtual void acquireLock() = 0;
	virtual void releaseLock() = 0;*/
};

class DEVICE_EXPORT __declspec(novtable) fiDeviceImplemented : public fiDevice
{
protected:
	fiDeviceImplemented();

public:
	virtual ~fiDeviceImplemented();

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

class DEVICE_EXPORT __declspec(novtable) fiDeviceRelative : public fiDeviceImplemented
{
private:
	char m_pad[272];
public:
	fiDeviceRelative();

	// Init
	// any RAGE path can be used; including root-relative paths
	void SetPath(const char* relativeTo, rage::fiDevice* baseDevice, bool allowRoot);

	// compatibility wrapper for NY code
	inline void SetPath(const char* relativeTo, bool allowRoot)
	{
		SetPath(relativeTo, nullptr, allowRoot);
	}

	// MountAs
	// mounts the device in the device stack
	void Mount(const char* mountPoint);
};

// fiPackfile8
#if 0
class DEVICE_EXPORT __declspec(novtable) fiPackfile : public fiDeviceImplemented
{
private:
	char m_pad[368 + (0x650 - 0x590)];
public:
	fiPackfile();

	// any RAGE path can be used; including root-relative paths
	bool OpenPackfile(const char* archive, bool bTrue, int type, intptr_t veryFalse);

	// compatibility wrappers
	inline void OpenPackfile(const char* archive, bool bTrue, bool bFalse, int type, intptr_t veryFalse)
	{
		OpenPackfile(archive, bTrue, type, veryFalse);
	}

	inline void OpenPackfile(const char* archive, bool bTrue, bool bFalse, intptr_t veryFalse)
	{
		OpenPackfile(archive, bTrue, 3, veryFalse);
	}

	// mounts the device in the device stack
	void Mount(const char* mountPoint);

	// closes the package file
	void ClosePackfile();
};
#endif
}
