/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

// method names based on research by listener, though they seem to match actual
// names somewhat (i.e. 'bulk' isn't some made-up shit)

#pragma once

#include "sysAllocator.h"

#ifdef COMPILING_RAGE_DEVICE_NY
#define DEVICE_EXPORT COMPONENT_EXPORT
#define DEVICE_IMPORT
#else
#define DEVICE_IMPORT __declspec(dllimport)
#define DEVICE_EXPORT
#endif

namespace rage
{
struct fiFindData
{
	char fileName[512];
	uint64_t fileSize;
	FILETIME lastWriteTime;
	DWORD fileAttributes;
};

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

	virtual uint32_t createLocal(const char* fileName) = 0;

	virtual uint32_t create(const char* fileName);

	virtual uint32_t read(uint32_t handle, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t readBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead) = 0;

	virtual uint32_t writeBulk(int, int, int, int, int) = 0;

	virtual uint32_t write(int, void*, int) = 0;

	virtual uint32_t seek(uint32_t handle, int32_t distance, uint32_t method) = 0;

	virtual void close(uint32_t handle) = 0;

	virtual void closeBulk(uint32_t handle) = 0;

	virtual int fileLength(uint32_t handle) = 0;

	virtual int m_34() = 0;
	virtual bool rm(const char* file) = 0;
	virtual int rename(const char* from, const char* to) = 0;
	virtual int mkdir(const char* dir) = 0;

	virtual int rmdir(const char * dir) = 0;
	virtual LARGE_INTEGER fileLengthLong(const char* file) = 0;
	virtual uint32_t fileTime(const char* file) = 0;
	virtual bool setFileTime(const char* file, FILETIME fileTime) = 0;

	virtual int findFirst(const char* path, fiFindData* findData) = 0;
	virtual bool findNext(int handle, fiFindData* findData) = 0;
	virtual int findClose(int handle) = 0;
	virtual bool truncate(uint32_t handle) = 0;

	virtual uint32_t getFileAttributes(const char* path) = 0;
	virtual bool setFileAttributes(const char* file, uint32_t FileAttributes) = 0;
	virtual int m_6C() = 0;
	virtual uint32_t getParentHandle() = 0;
};

class DEVICE_EXPORT __declspec(novtable) fiDeviceImplemented : public fiDevice
{
protected:
	fiDeviceImplemented();

public:
	virtual ~fiDeviceImplemented();

	virtual uint32_t open(const char* fileName, bool);

	virtual uint32_t openBulk(const char* fileName, uint64_t* ptr);

	virtual uint32_t createLocal(const char* fileName);

	virtual uint32_t create(const char*);

	virtual uint32_t read(uint32_t handle, void* buffer, uint32_t toRead);

	virtual uint32_t readBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead);

	virtual uint32_t writeBulk(int, int, int, int, int);

	virtual uint32_t write(int, void*, int);

	virtual uint32_t seek(uint32_t handle, int32_t distance, uint32_t method);

	virtual void close(uint32_t handle);

	virtual void closeBulk(uint32_t handle);

	virtual int fileLength(uint32_t handle);

	virtual int m_34();
	virtual bool rm(const char* file);
	virtual int rename(const char* from, const char* to);
	virtual int mkdir(const char* dir);

	virtual int rmdir(const char * dir);
	virtual LARGE_INTEGER fileLengthLong(const char* file);
	virtual uint32_t fileTime(const char* file);
	virtual bool setFileTime(const char* file, FILETIME fileTime);

	virtual int findFirst(const char* path, fiFindData* findData);
	virtual bool findNext(int handle, fiFindData* findData);
	virtual int findClose(int handle);
	virtual bool truncate(uint32_t handle);

	virtual uint32_t getFileAttributes(const char* path);
	virtual bool setFileAttributes(const char* file, uint32_t FileAttributes);
	virtual int m_6C();
	virtual uint32_t getParentHandle();
};

class DEVICE_EXPORT __declspec(novtable) fiDeviceRelative : public fiDeviceImplemented
{
private:
	char m_pad[524];
public:
	fiDeviceRelative();

	// any RAGE path can be used; including root-relative paths
	void setPath(const char* relativeTo, bool allowRoot);

	// mounts the device in the device stack
	void mount(const char* mountPoint);
};

class DEVICE_EXPORT __declspec(novtable) fiPackfile : public fiDeviceImplemented
{
private:
	char m_pad[1144];
public:
	fiPackfile();

	// any RAGE path can be used; including root-relative paths
	void openArchive(const char* archive, bool bTrue, bool bFalse, int type);

	// mounts the device in the device stack
	void mount(const char* mountPoint);

	// closes the package file
	void closeArchive();
};
}