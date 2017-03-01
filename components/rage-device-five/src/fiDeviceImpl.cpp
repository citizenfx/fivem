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

uint64_t WRAPPER fiDeviceImplemented::Open(const char* fileName, bool) { PURECALL(); }

uint64_t WRAPPER fiDeviceImplemented::OpenBulk(const char* fileName, uint64_t* ptr)
{
	PURECALL();
}

uint64_t WRAPPER fiDeviceImplemented::CreateLocal(const char* fileName)
{
	PURECALL();
}

uint64_t WRAPPER fiDeviceImplemented::Create(const char*)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::Read(uint64_t handle, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::ReadBulk(uint64_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::WriteBulk(uint64_t, int, int, int, int)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::Write(uint64_t, void*, int)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::Seek(uint64_t handle, int32_t distance, uint32_t method)
{
	PURECALL();
}

int32_t WRAPPER fiDeviceImplemented::Close(uint64_t handle)
{
	PURECALL();
}

int32_t WRAPPER fiDeviceImplemented::CloseBulk(uint64_t handle)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::GetFileLength(uint64_t handle)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_40(int)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::RemoveFile(const char* file)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::RenameFile(const char* from, const char* to)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::CreateDirectory(const char* dir)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::RemoveDirectory(const char * dir)
{
	PURECALL();
}

uint64_t WRAPPER fiDeviceImplemented::GetFileLengthUInt64(uint64_t file)
{
	PURECALL();
}

uint64_t WRAPPER fiDeviceImplemented::GetFileLengthLong(const char* fileName)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::GetFileTime(const char* file)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::SetFileTime(const char* file, FILETIME fileTime)
{
	PURECALL();
}

uint64_t WRAPPER fiDeviceImplemented::FindFirst(const char* path, fiFindData* findData)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::FindNext(uint64_t handle, fiFindData* findData)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::FindClose(uint64_t handle)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::Truncate(uint64_t handle)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::GetFileAttributes(const char* path)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::SetFileAttributes(const char* file, uint32_t FileAttributes)
{
	PURECALL();
}

void WRAPPER fiDeviceImplemented::m_xx()
{
	return;
}

void* WRAPPER fiDeviceImplemented::m_xy(void*, int, void*)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::m_xz()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_yx()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_yy()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_yz(void*)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_zx(void*)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::IsBulkDevice()
{
	PURECALL();
}

fiDevice* WRAPPER fiDeviceImplemented::m_zz()
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::m_ax()
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::GetCollectionId()
{
	PURECALL();
}

const char* WRAPPER fiDeviceImplemented::GetName()
{
	PURECALL();
}
}