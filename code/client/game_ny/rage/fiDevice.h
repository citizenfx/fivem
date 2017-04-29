#pragma once

#include "sysAllocator.h"

namespace rage
{
struct fiFindData
{
	char fileName[512];
	uint64_t fileSize;
	FILETIME lastWriteTime;
	DWORD fileAttributes;
};

class GAMESPEC_EXPORT_VMT __declspec(novtable) fiDevice : public sysUseAllocator
{
public:
	static fiDevice* GetDevice(const char* path, bool allowRoot);

	static void Unmount(const char* rootPath);

	static void SetInitialMountHook(void(*callback)(void*));

public:
	virtual ~fiDevice() = 0;

	virtual uint32_t Open(const char* fileName, bool readOnly) = 0;

	virtual uint32_t OpenBulk(const char* fileName, uint64_t* ptr) = 0;

	virtual uint32_t CreateLocal(const char* fileName) = 0;

	virtual uint32_t Create(const char* fileName);

	virtual uint32_t Read(uint32_t handle, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t ReadBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t WriteBulk(int, int, int, int, int) = 0;

	virtual uint32_t Write(int, void*, int) = 0;

	virtual uint32_t Seek(uint32_t handle, int32_t distance, uint32_t method) = 0;

	virtual void Close(uint32_t handle) = 0;

	virtual void CloseBulk(uint32_t handle) = 0;

	virtual int GetFileLength(uint32_t handle) = 0;

	virtual int m_34() = 0;
	virtual bool RemoveFile(const char* file) = 0;
	virtual int RenameFile(const char* from, const char* to) = 0;
	virtual int CreateDirectory(const char* dir) = 0;

	virtual int RemoveDirectory(const char * dir) = 0;
	virtual LARGE_INTEGER GetFileLengthLong(const char* file) = 0;
	virtual uint64_t GetFileTime(const char* file) = 0;
	virtual bool SetFileTime(const char* file, FILETIME fileTime) = 0;

	virtual int FindFirst(const char* path, fiFindData* findData) = 0;
	virtual bool FindNext(int handle, fiFindData* findData) = 0;
	virtual int FindClose(int handle) = 0;
	virtual bool Truncate(uint32_t handle) = 0;

	virtual uint32_t GetFileAttributes(const char* path) = 0;
	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes) = 0;
	virtual int m_6C() = 0;
	virtual uint32_t GetParentHandle() = 0;
};

class GAMESPEC_EXPORT_VMT fiDeviceImplemented : public fiDevice
{
protected:
	fiDeviceImplemented();

public:
	virtual ~fiDeviceImplemented();

	virtual uint32_t Open(const char* fileName, bool);

	virtual uint32_t OpenBulk(const char* fileName, uint64_t* ptr);

	virtual uint32_t CreateLocal(const char* fileName);

	virtual uint32_t Create(const char*);

	virtual uint32_t Read(uint32_t handle, void* buffer, uint32_t toRead);

	virtual uint32_t ReadBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead);

	virtual uint32_t WriteBulk(int, int, int, int, int);

	virtual uint32_t Write(int, void*, int);

	virtual uint32_t Seek(uint32_t handle, int32_t distance, uint32_t method);

	virtual void Close(uint32_t handle);

	virtual void CloseBulk(uint32_t handle);

	virtual int GetFileLength(uint32_t handle);

	virtual int m_34();
	virtual bool RemoveFile(const char* file);
	virtual int RenameFile(const char* from, const char* to);
	virtual int CreateDirectory(const char* dir);

	virtual int RemoveDirectory(const char * dir);
	virtual LARGE_INTEGER GetFileLengthLong(const char* file);
	virtual uint64_t GetFileTime(const char* file);
	virtual bool SetFileTime(const char* file, FILETIME fileTime);

	virtual int FindFirst(const char* path, fiFindData* findData);
	virtual bool FindNext(int handle, fiFindData* findData);
	virtual int FindClose(int handle);
	virtual bool Truncate(uint32_t handle);

	virtual uint32_t GetFileAttributes(const char* path);
	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes);
	virtual int m_6C();
	virtual uint32_t GetParentHandle();
};

class GAMESPEC_EXPORT_VMT fiDeviceRelative : public fiDeviceImplemented
{
private:
	char m_pad[524];
public:
	fiDeviceRelative();

	// any RAGE path can be used; including root-relative paths
	void SetPath(const char* relativeTo, bool allowRoot);

	// mounts the device in the device stack
	void Mount(const char* mountPoint);
};

class GAMESPEC_EXPORT_VMT fiPackfile : public fiDeviceImplemented
{
private:
	char m_pad[1144];
public:
	fiPackfile();

	// any RAGE path can be used; including root-relative paths
	void OpenPackfile(const char* archive, bool bTrue, bool bFalse, int type);

	// mounts the device in the device stack
	void Mount(const char* mountPoint);

	// closes the package file
	void ClosePackfile();
};
}