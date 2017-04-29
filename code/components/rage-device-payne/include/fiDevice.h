#pragma once

#include "sysAllocator.h"

#ifdef COMPILING_RAGE_DEVICE_PAYNE
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

// since Payne, RAGE devices are thread-safe
class DEVICE_EXPORT __declspec(novtable) fiDevice : public sysUseAllocator
{
public:
	static fiDevice* GetDevice(const char* path, bool allowRoot);

	static void Unmount(const char* rootPath);

	static DEVICE_IMPORT fwEvent<> OnInitialMount;

public:
	virtual ~fiDevice() = 0;

	virtual uint32_t open(const char* fileName, bool readOnly) = 0;

	virtual uint32_t openBulk(const char* fileName, uint64_t* ptr) = 0;

	virtual uint32_t openBulkWrap(const char* fileName, uint64_t* ptr, void*) = 0;

	virtual uint32_t createLocal(const char* fileName) = 0;

	virtual uint32_t create(const char* fileName) = 0;

	virtual uint32_t read(uint32_t handle, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t readBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t writeBulk(int, int, int, int, int) = 0;

	virtual uint32_t write(int, void*, int) = 0;

	virtual uint32_t seek(uint32_t handle, int32_t distance, uint32_t method) = 0;

	virtual uint64_t seekLong(uint32_t handle, int64_t distance, uint32_t method) = 0;

	virtual void close(uint32_t handle) = 0;

	virtual void closeBulk(uint32_t handle) = 0;

	virtual int fileLength(uint32_t handle) = 0;

	virtual uint64_t fileLengthLong(uint32_t handle) = 0;

	// dummy!
	virtual int m_40(int) = 0;

	virtual bool rm(const char* file) = 0;
	virtual int rename(const char* from, const char* to) = 0;
	virtual int mkdir(const char* dir) = 0;

	virtual int rmdir(const char * dir) = 0;

	virtual uint64_t fileLengthByName(const char* fileName) = 0;

	virtual uint32_t fileTime(const char* file) = 0;
	virtual bool setFileTime(const char* file, FILETIME fileTime) = 0;

	virtual int findFirst(const char* path, fiFindData* findData) = 0;
	virtual bool findNext(int handle, fiFindData* findData) = 0;
	virtual int findClose(int handle) = 0;
	virtual bool truncate(uint32_t handle) = 0;

	virtual uint32_t getFileAttributes(const char* path) = 0;
	virtual bool setFileAttributes(const char* file, uint32_t FileAttributes) = 0;

	// read even if read() returns less than length
	virtual bool readFull(uint32_t handle, void* buffer, uint32_t length) = 0;

	virtual bool writeFull(uint32_t handle, void* buffer, uint32_t length) = 0;

	virtual bool getResourceVersion(const char* fileName, uint32_t* version) = 0;

	virtual uint64_t m_84(int a1) = 0;

	virtual uint32_t m_88(int a1) = 0;

	virtual bool m_8C() = 0;

	virtual int m_90() = 0;

	virtual const char* getName() = 0;

	virtual rage::fiDevice* getUnkDevice() = 0;

	virtual int m_9C(int, int, int) = 0;

	virtual void acquireLock() = 0;
	virtual void releaseLock() = 0;
};

class DEVICE_EXPORT __declspec(novtable) fiDeviceImplemented : public fiDevice
{
protected:
	fiDeviceImplemented();

public:
	virtual ~fiDeviceImplemented();

	virtual uint32_t open(const char* fileName, bool readOnly);

	virtual uint32_t openBulk(const char* fileName, uint64_t* ptr);

	virtual uint32_t openBulkWrap(const char* fileName, uint64_t* ptr, void*);

	virtual uint32_t createLocal(const char* fileName);

	virtual uint32_t create(const char* fileName);

	virtual uint32_t read(uint32_t handle, void* buffer, uint32_t toRead);

	virtual uint32_t readBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead);

	virtual uint32_t writeBulk(int, int, int, int, int);

	virtual uint32_t write(int, void*, int);

	virtual uint32_t seek(uint32_t handle, int32_t distance, uint32_t method);

	virtual uint64_t seekLong(uint32_t handle, int64_t distance, uint32_t method);

	virtual void close(uint32_t handle);

	virtual void closeBulk(uint32_t handle);
	
	virtual int fileLength(uint32_t handle);

	virtual uint64_t fileLengthLong(uint32_t handle);

	// dummy!
	virtual int m_40(int);

	virtual bool rm(const char* file);
	virtual int rename(const char* from, const char* to);
	virtual int mkdir(const char* dir);

	virtual int rmdir(const char * dir);

	virtual uint64_t fileLengthByName(const char* fileName);

	virtual uint32_t fileTime(const char* file);
	virtual bool setFileTime(const char* file, FILETIME fileTime);

	virtual int findFirst(const char* path, fiFindData* findData);
	virtual bool findNext(int handle, fiFindData* findData);
	virtual int findClose(int handle);
	virtual bool truncate(uint32_t handle);

	virtual uint32_t getFileAttributes(const char* path);
	virtual bool setFileAttributes(const char* file, uint32_t FileAttributes);

	// read even if read() returns less than length
	virtual bool readFull(uint32_t handle, void* buffer, uint32_t length);

	virtual bool writeFull(uint32_t handle, void* buffer, uint32_t length);

	virtual bool getResourceVersion(const char* fileName, uint32_t* version);

	virtual uint64_t m_84(int a1);

	virtual uint32_t m_88(int a1);

	virtual bool m_8C();

	virtual int m_90();

	virtual const char* getName();

	virtual rage::fiDevice* getUnkDevice();

	virtual int m_9C(int, int, int);

	virtual void acquireLock();
	virtual void releaseLock();
};

class DEVICE_EXPORT __declspec(novtable) fiDeviceRelative : public fiDeviceImplemented
{
private:
	char m_pad[272];
public:
	fiDeviceRelative();

	// any RAGE path can be used; including root-relative paths
	void setPath(const char* relativeTo, rage::fiDevice* baseDevice, bool allowRoot);

	// mounts the device in the device stack
	void mount(const char* mountPoint);
};

class DEVICE_EXPORT __declspec(novtable) fiPackfile : public fiDeviceImplemented
{
private:
	char m_pad[368];
public:
	fiPackfile();

	// any RAGE path can be used; including root-relative paths
	void openArchive(const char* archive, bool bTrue, bool bFalse, int type, int veryFalse);

	// mounts the device in the device stack
	void mount(const char* mountPoint);

	// closes the package file
	void closeArchive();
};
}