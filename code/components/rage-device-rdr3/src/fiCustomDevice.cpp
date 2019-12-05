/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <fiCustomDevice.h>

#include <Hooking.h>

#define PURECALL() FatalError("pure fiDevice call (" __FUNCTION__ ")"); return 0

namespace rage
{
static uintptr_t g_vTable_fiDevice;

fiCustomDevice::fiCustomDevice()
{
	*(uintptr_t*)(&m_parentDeviceData[0]) = g_vTable_fiDevice;

	m_parentDeviceRef = reinterpret_cast<rage::fiDevice*>(m_parentDeviceData);
}

fiCustomDevice::~fiCustomDevice()
{

}

uint64_t WRAPPER fiCustomDevice::OpenFlags(const char* fileName, bool readOnly, uint32_t shareFlags, uint32_t openFlags)
{
	return m_parentDeviceRef->OpenFlags(fileName, readOnly, shareFlags, openFlags);
}

uint64_t fiCustomDevice::OpenHash(uint32_t* hashValue)
{
	return m_parentDeviceRef->OpenHash(hashValue);
}

uint64_t fiCustomDevice::OpenHashExt(uint32_t* hashValue, int extension)
{
	return m_parentDeviceRef->OpenHashExt(hashValue, extension);
}

uint64_t fiCustomDevice::CreateFlags(const char* fileName, uint32_t shareFlags, uint32_t openFlags)
{
	return m_parentDeviceRef->CreateFlags(fileName, shareFlags, openFlags);
}

int fiCustomDevice::ReadFile(const char* path, void* buffer, int size)
{
	return m_parentDeviceRef->ReadFile(path, buffer, size);
}

uint32_t fiCustomDevice::ReadBulkOverlapped(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	return m_parentDeviceRef->ReadBulkOverlapped(handle, ptr, buffer, toRead);
}

int fiCustomDevice::CloseBulkOverlapped(uint64_t handle)
{
	return m_parentDeviceRef->CloseBulkOverlapped(handle);
}

int fiCustomDevice::Flush(int a)
{
	return m_parentDeviceRef->Flush(a);
}

void fiCustomDevice::RenameWithProgress(void* a)
{
	return m_parentDeviceRef->RenameWithProgress(a);
}

uint64_t fiCustomDevice::GetAvailableDiskSpace(const char* a)
{
	return m_parentDeviceRef->GetAvailableDiskSpace(a);
}

void fiCustomDevice::Sanitize()
{

}

int64_t fiCustomDevice::GetFileSize(const atArray<const char*>& files, atArray<uint64_t>& sizes)
{
	return m_parentDeviceRef->GetFileSize(files, sizes);
}

int64_t fiCustomDevice::GetFileTime(const atArray<const char*>& files, atArray<uint64_t>& times)
{
	return m_parentDeviceRef->GetFileTime(files, times);
}

uint64_t WRAPPER fiCustomDevice::OpenBulk(const char* fileName, uint64_t* ptr)
{
	return m_parentDeviceRef->OpenBulk(fileName, ptr);
}

uint64_t WRAPPER fiCustomDevice::CreateLocal(const char* fileName)
{
	return m_parentDeviceRef->CreateLocal(fileName);
}

uint32_t WRAPPER fiCustomDevice::ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	return m_parentDeviceRef->ReadBulk(handle, ptr, buffer, toRead);
}

uint32_t WRAPPER fiCustomDevice::WriteBulk(uint64_t a1, int a2, int a3, int a4, int a5)
{
	return m_parentDeviceRef->WriteBulk(a1, a2, a3, a4, a5);
}

int32_t WRAPPER fiCustomDevice::CloseBulk(uint64_t handle)
{
	return m_parentDeviceRef->CloseBulk(handle);
}

int WRAPPER fiCustomDevice::GetFileLength(uint64_t handle)
{
	return m_parentDeviceRef->GetFileLength(handle);
}

bool WRAPPER fiCustomDevice::RemoveFile(const char* file)
{
	return false;
}

int WRAPPER fiCustomDevice::RenameFile(const char* from, const char* to)
{
	return false;
}

int WRAPPER fiCustomDevice::CreateDirectory(const char* dir)
{
	return false;
}

int WRAPPER fiCustomDevice::RemoveDirectory(const char * dir)
{
	return false;
}

uint64_t WRAPPER fiCustomDevice::GetFileLengthUInt64(uint64_t file)
{
	return m_parentDeviceRef->GetFileLengthUInt64(file);
}

uint64_t WRAPPER fiCustomDevice::GetFileLengthLong(const char* fileName)
{
	return m_parentDeviceRef->GetFileLengthLong(fileName);
}

uint64_t fiCustomDevice::FindFileBegin(const char* path, const char* pattern, fiFindData* findData)
{
	return m_parentDeviceRef->FindFileBegin(path, pattern, findData);
}

uint64_t WRAPPER fiCustomDevice::FindFirst(const char* path, fiFindData* findData)
{
	return m_parentDeviceRef->FindFirst(path, findData);
}

bool WRAPPER fiCustomDevice::FindNext(uint64_t handle, fiFindData* findData)
{
	return m_parentDeviceRef->FindNext(handle, findData);
}

int WRAPPER fiCustomDevice::FindClose(uint64_t handle)
{
	return m_parentDeviceRef->FindClose(handle);
}

bool WRAPPER fiCustomDevice::Truncate(uint64_t handle)
{
	return false;
}

uint32_t WRAPPER fiCustomDevice::GetFileAttributes(const char* path)
{
	return m_parentDeviceRef->GetFileAttributes(path);
}

bool WRAPPER fiCustomDevice::SetFileAttributes(const char* file, uint32_t FileAttributes)
{
	return m_parentDeviceRef->SetFileAttributes(file, FileAttributes);
}

void* WRAPPER fiCustomDevice::m_xy(void* a, int b, void* c)
{
	return m_parentDeviceRef->m_xy(a, b, c);
}

bool WRAPPER fiCustomDevice::IsCollection()
{
	return m_parentDeviceRef->IsCollection();
}

fiDevice* WRAPPER fiCustomDevice::GetCollection()
{
	return m_parentDeviceRef->GetCollection();
}

fiDevice* fiCustomDevice::GetLowLevelDevice()
{
	return m_parentDeviceRef->GetLowLevelDevice();
}

bool fiCustomDevice::IsMemoryMappedDevice()
{
	return false;
}

int fiCustomDevice::GetResourceVersion(const atArray<const char*>& files, atArray<ResourceFlags>& flags)
{
	return m_parentDeviceRef->GetResourceVersion(files, flags);
}

int fiCustomDevice::GetEncryptionKey()
{
	return m_parentDeviceRef->GetEncryptionKey();
}

int fiCustomDevice::IsValidHandle(uint64_t a)
{
	return m_parentDeviceRef->IsValidHandle(a);
}

int64_t fiCustomDevice::GetBulkOffset(const char* a)
{
	return m_parentDeviceRef->GetBulkOffset(a);
}

int fiCustomDevice::GetPhysicalSortKey(void* a)
{
	return m_parentDeviceRef->GetPhysicalSortKey(a);
}

int fiCustomDevice::GetRpfVersion()
{
	return m_parentDeviceRef->GetRpfVersion();
}

bool fiCustomDevice::IsCloud()
{
	return false;
}

bool fiCustomDevice::IsZip()
{
	return false;
}

bool fiCustomDevice::SupportsOverlappedIO()
{
	return false;
}

uint32_t fiCustomDevice::GetClampedBufferSize(uint64_t, int)
{
	return -1;
}

uint64_t fiCustomDevice::GetFinalOffset(uint64_t)
{
	return 0;
}

bool fiCustomDevice::IsOverlappedRequestFinished(uint64_t, int)
{
	return false;
}

int WRAPPER fiCustomDevice::GetCollectionId()
{
	return m_parentDeviceRef->GetCollectionId();
}

uint64_t fiCustomDevice::OpenBulkWrap(const char* fileName, uint64_t* ptr, void*)
{
	return m_parentDeviceRef->OpenBulk(fileName, ptr);
}

bool fiCustomDevice::ReadFull(uint64_t handle, void* buffer, uint32_t length)
{
	return m_parentDeviceRef->ReadFull(handle, buffer, length);
}

bool fiCustomDevice::WriteFull(uint64_t handle, void* buffer, uint32_t length)
{
	return m_parentDeviceRef->WriteFull(handle, buffer, length);
}

int fiCustomDevice::GetResourceVersion(const char* fileName, ResourceFlags* version)
{
	return m_parentDeviceRef->GetResourceVersion(fileName, version);
}
}

static HookFunction hookFunction([] ()
{
	char* location = hook::pattern("48 89 7B 68 0F B7 43 08 48 89").count(1).get(0).get<char>(15);

	rage::g_vTable_fiDevice = (uintptr_t)(location + *(int32_t*)location + 4);
});
