#include "StdInc.h"
#include "fiDevice.h"

#include <Error.h>

#define PURECALL() FatalError("pure fiDevice call (" __FUNCTION__ ")"); return 0

namespace rage
{
fiDeviceImplemented::fiDeviceImplemented()
{
}

fiDeviceImplemented::~fiDeviceImplemented()
{

}

uint64_t fiDeviceImplemented::OpenFlags(const char* fileName, bool readOnly, uint32_t shareFlags, uint32_t openFlags)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::OpenHash(uint32_t* hashValue)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::OpenHashExt(uint32_t* hashValue, int extension)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::Open(const char* fileName, bool readOnly)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::OpenBulk(const char* fileName, uint64_t* ptr)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::OpenBulkWrap(const char* fileName, uint64_t* ptr, void*)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::CreateLocal(const char* fileName)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::CreateFlags(const char* fileName, uint32_t shareFlags, uint32_t openFlags)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::Create(const char* fileName)
{
	PURECALL();
}

uint32_t fiDeviceImplemented::Read(uint64_t handle, void* buffer, uint32_t toRead)
{
	PURECALL();
}

int fiDeviceImplemented::ReadFile(const char* path, void* buffer, int size)
{
	PURECALL();
}

uint32_t fiDeviceImplemented::ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t fiDeviceImplemented::ReadBulkOverlapped(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t fiDeviceImplemented::WriteBulk(uint64_t, int, int, int, int)
{
	PURECALL();
}

uint32_t fiDeviceImplemented::Write(uint64_t, void*, int)
{
	PURECALL();
}

uint32_t fiDeviceImplemented::Seek(uint64_t handle, int32_t distance, uint32_t method)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::SeekLong(uint64_t handle, int64_t distance, uint32_t method)
{
	PURECALL();
}

int32_t fiDeviceImplemented::Close(uint64_t handle)
{
	PURECALL();
}

int32_t fiDeviceImplemented::CloseBulk(uint64_t handle)
{
	PURECALL();
}

int32_t fiDeviceImplemented::CloseBulkOverlapped(uint64_t handle)
{
	PURECALL();
}

int fiDeviceImplemented::GetFileLength(uint64_t handle)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::GetFileLengthUInt64(uint64_t handle)
{
	PURECALL();
}

int fiDeviceImplemented::Flush(int)
{
	PURECALL();
}

bool fiDeviceImplemented::RemoveFile(const char* file)
{
	PURECALL();
}

int fiDeviceImplemented::RenameFile(const char* from, const char* to)
{
	PURECALL();
}

void fiDeviceImplemented::RenameWithProgress(void*)
{

}

uint64_t fiDeviceImplemented::GetAvailableDiskSpace(const char*)
{
	PURECALL();
}

int fiDeviceImplemented::CreateDirectory(const char* dir)
{
	PURECALL();
}

int fiDeviceImplemented::RemoveDirectory(const char* dir)
{
	PURECALL();
}

void fiDeviceImplemented::Sanitize()
{

}

int64_t fiDeviceImplemented::GetFileSize(const atArray<const char*>& files, atArray<uint64_t>& sizes)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::GetFileLengthLong(const char* fileName)
{
	PURECALL();
}

int64_t fiDeviceImplemented::GetFileTime(const atArray<const char*>& files, atArray<uint64_t>& times)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::GetFileTime(const char* file)
{
	PURECALL();
}

bool fiDeviceImplemented::SetFileTime(const char* file, FILETIME fileTime)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::FindFileBegin(const char* path, const char* pattern, fiFindData* findData)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::FindFirst(const char* path, fiFindData* findData)
{
	PURECALL();
}

bool fiDeviceImplemented::FindNext(uint64_t handle, fiFindData* findData)
{
	PURECALL();
}

int fiDeviceImplemented::FindClose(uint64_t handle)
{
	PURECALL();
}

rage::fiDevice* fiDeviceImplemented::GetLowLevelDevice()
{
	PURECALL();
}

void* fiDeviceImplemented::m_xy(void*, int, void*)
{
	PURECALL();
}

bool fiDeviceImplemented::Truncate(uint64_t handle)
{
	PURECALL();
}

uint32_t fiDeviceImplemented::GetFileAttributes(const char* path)
{
	PURECALL();
}

bool fiDeviceImplemented::SetFileAttributes(const char* file, uint32_t FileAttributes)
{
	PURECALL();
}

int fiDeviceImplemented::m_yx()
{
	PURECALL();
}

bool fiDeviceImplemented::IsMemoryMappedDevice()
{
	PURECALL();
}

bool fiDeviceImplemented::ReadFull(uint64_t handle, void* buffer, uint32_t length)
{
	PURECALL();
}

bool fiDeviceImplemented::WriteFull(uint64_t handle, void* buffer, uint32_t length)
{
	PURECALL();
}

int32_t fiDeviceImplemented::GetResourceVersion(const atArray<const char*>& files, atArray<ResourceFlags>& flags)
{
	PURECALL();
}

int32_t fiDeviceImplemented::GetResourceVersion(const char* fileName, ResourceFlags* flags)
{
	PURECALL();
}

int32_t fiDeviceImplemented::GetEncryptionKey()
{
	PURECALL();
}

int32_t fiDeviceImplemented::IsValidHandle(uint64_t)
{
	PURECALL();
}

int64_t fiDeviceImplemented::GetBulkOffset(const char*)
{
	PURECALL();
}

int32_t fiDeviceImplemented::GetPhysicalSortKey(void*)
{
	PURECALL();
}

bool fiDeviceImplemented::IsCollection()
{
	PURECALL();
}

int fiDeviceImplemented::GetRpfVersion()
{
	PURECALL();
}

rage::fiDevice* fiDeviceImplemented::GetCollection()
{
	PURECALL();
}

bool fiDeviceImplemented::IsCloud()
{
	PURECALL();
}

bool fiDeviceImplemented::IsZip()
{
	PURECALL();
}

int32_t fiDeviceImplemented::GetCollectionId()
{
	PURECALL();
}

const char* fiDeviceImplemented::GetName()
{
	PURECALL();
}

bool fiDeviceImplemented::SupportsOverlappedIO()
{
	PURECALL();
}

uint32_t fiDeviceImplemented::GetClampedBufferSize(uint64_t, int)
{
	PURECALL();
}

uint64_t fiDeviceImplemented::GetFinalOffset(uint64_t)
{
	PURECALL();
}

bool fiDeviceImplemented::IsOverlappedRequestFinished(uint64_t, int)
{
	PURECALL();
}
}
