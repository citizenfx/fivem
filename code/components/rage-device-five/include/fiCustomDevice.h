/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <fiDevice.h>

#ifdef COMPILING_RAGE_DEVICE_FIVE
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

	virtual uint64_t OpenBulk(const char* fileName, uint64_t* ptr);

	virtual uint64_t OpenBulkWrap(const char* fileName, uint64_t* ptr, void*);

	virtual uint64_t CreateLocal(const char* fileName);

	virtual uint32_t ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead);

	virtual uint32_t WriteBulk(uint64_t, int, int, int, int);

	virtual int32_t CloseBulk(uint64_t handle);

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

	virtual uint64_t FindFirst(const char* path, fiFindData* findData);
	virtual bool FindNext(uint64_t handle, fiFindData* findData);
	virtual int FindClose(uint64_t handle);

	virtual rage::fiDevice* GetUnkDevice();

	virtual void* m_xy(void*, int, void*);

	virtual bool Truncate(uint64_t handle);

	virtual uint32_t GetFileAttributes(const char* path);

	virtual bool m_xz();

	virtual bool SetFileAttributes(const char* file, uint32_t FileAttributes);

	// read even if read() returns less than length
	virtual bool ReadFull(uint64_t handle, void* buffer, uint32_t length);

	virtual bool WriteFull(uint64_t handle, void* buffer, uint32_t length);

	virtual int32_t GetResourceVersion(const char* fileName, ResourceFlags* version);

	virtual int32_t m_yy();

	virtual int32_t m_yz(void*);

	virtual int32_t m_zx(void*); // return 0x40000000

	virtual bool IsBulkDevice();

	virtual fiDevice* m_zz(); // return this

	virtual bool m_ax();

	virtual int32_t GetCollectionId();
};
}