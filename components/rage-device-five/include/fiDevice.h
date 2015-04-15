#pragma once

#include "sysAllocator.h"

#ifdef COMPILING_RAGE_DEVICE_FIVE
#define DEVICE_EXPORT __declspec(dllexport)
#define DEVICE_IMPORT
#else
#define DEVICE_IMPORT __declspec(dllimport)
#define DEVICE_EXPORT
#endif

namespace rage
{
struct fiFindData
{
	char fileName[256];
	uint64_t fileSize;
	FILETIME lastWriteTime;
	DWORD fileAttributes;
};

// since Payne, RAGE devices are thread-safe (might not apply to V?)
// in V, RAGE devices use UTF-8
class DEVICE_EXPORT __declspec(novtable) fiDevice : public sysUseAllocator
{
public:
	static fiDevice* GetDevice(const char* path, bool allowRoot);

	static void Unmount(const char* rootPath);

	static DEVICE_IMPORT fwEvent<> OnInitialMount;

public:
	virtual ~fiDevice() = 0;

	virtual uint64_t Open(const char* fileName, bool readOnly) = 0;

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr) = 0;

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*) = 0;

	virtual uint64_t CreateLocal(const char* fileName) = 0;

	virtual uint64_t Create(const char* fileName) = 0;

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int) = 0;

	virtual uint32_t Write(uint64_t, void*, int) = 0;

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method) = 0;

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method) = 0;

	virtual void Close(uint64_t handle) = 0;

	virtual void CloseBulk(uint64_t handle) = 0;

	virtual int GetFileLength(uint64_t handle) = 0;

	virtual uint64_t GetFileLengthUInt64(uint64_t handle) = 0;

	// dummy!
	virtual int m_40(int) = 0;

	virtual bool RemoveFile(const char* file) = 0;
	virtual int RenameFile(const char* from, const char* to) = 0;
	virtual int CreateDirectory(const char* dir) = 0;

	virtual int RemoveDirectory(const char * dir) = 0;

	virtual void m_xx() = 0;

	virtual uint64_t GetFileLengthLong(const char* fileName) = 0;

	virtual uint32_t GetFileTime(const char* file) = 0;
	virtual bool SetFileTime(const char* file, FILETIME fileTime) = 0;

	virtual uint64_t FindFirst(const char* path, fiFindData* findData) = 0;
	virtual bool FindNext(uint64_t handle, fiFindData* findData) = 0;
	virtual int FindClose(uint64_t handle) = 0;

	virtual rage::fiDevice* GetUnkDevice() = 0;

	virtual void m_xy() = 0;

	virtual bool Truncate(uint64_t handle) = 0;

	virtual uint32_t GetFileAttributes(const char* path) = 0;

	virtual bool m_xz() = 0;

	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes) = 0;

	virtual int m_yx() = 0;

	// read even if read() returns less than length
	virtual bool ReadFull(uint64_t handle, void* buffer, uint32_t length) = 0;

	virtual bool WriteFull(uint64_t handle, void* buffer, uint32_t length) = 0;

	virtual bool GetResourceVersion(const char* fileName, uint32_t* version) = 0;

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

	virtual uint64_t Open(const char* fileName, bool readOnly);

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr);

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*);

	virtual uint64_t CreateLocal(const char* fileName);

	virtual uint64_t Create(const char* fileName);

	virtual uint32_t Read(uint64_t handle, void* buffer, uint32_t toRead);

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead);

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int);

	virtual uint32_t Write(uint64_t, void*, int);

	virtual uint32_t Seek(uint64_t handle, int32_t distance, uint32_t method);

	virtual uint64_t SeekLong(uint64_t handle, int64_t distance, uint32_t method);

	virtual void Close(uint64_t handle);

	virtual void CloseBulk(uint64_t handle);

	virtual int GetFileLength(uint64_t handle);

	virtual uint64_t GetFileLengthUInt64(uint64_t handle);

	// dummy!
	virtual int m_40(int);

	virtual bool RemoveFile(const char* file);
	virtual int RenameFile(const char* from, const char* to);
	virtual int CreateDirectory(const char* dir);

	virtual int RemoveDirectory(const char * dir);

	virtual void m_xx();

	virtual uint64_t GetFileLengthLong(const char* fileName);

	virtual uint32_t GetFileTime(const char* file);
	virtual bool SetFileTime(const char* file, FILETIME fileTime);

	virtual uint64_t FindFirst(const char* path, fiFindData* findData);
	virtual bool FindNext(uint64_t handle, fiFindData* findData);
	virtual int FindClose(uint64_t handle);

	virtual rage::fiDevice* GetUnkDevice();

	virtual void m_xy();

	virtual bool Truncate(uint64_t handle);

	virtual uint32_t GetFileAttributes(const char* path);

	virtual bool m_xz();

	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes);

	virtual int m_yx();

	// read even if read() returns less than length
	virtual bool ReadFull(uint64_t handle, void* buffer, uint32_t length);

	virtual bool WriteFull(uint64_t handle, void* buffer, uint32_t length);

	virtual bool GetResourceVersion(const char* fileName, uint32_t* version);
};

class DEVICE_EXPORT __declspec(novtable) fiDeviceRelative : public fiDeviceImplemented
{
private:
	char m_pad[272];
public:
	fiDeviceRelative();

	// any RAGE path can be used; including root-relative paths
	void SetPath(const char* relativeTo, rage::fiDevice* baseDevice, bool allowRoot);

	// mounts the device in the device stack
	void Mount(const char* mountPoint);
};

class DEVICE_EXPORT __declspec(novtable) fiPackfile : public fiDeviceImplemented
{
private:
	char m_pad[368 + (0x650 - 0x590)];
public:
	fiPackfile();

	// any RAGE path can be used; including root-relative paths
	void OpenPackfile(const char* archive, bool bTrue, bool bFalse, int type, int veryFalse);

	// mounts the device in the device stack
	void Mount(const char* mountPoint);

	// closes the package file
	void ClosePackfile();
};
}