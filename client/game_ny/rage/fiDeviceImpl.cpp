#include "StdInc.h"
#include "fiDevice.h"

#define PURECALL() __asm { jmp _purecall }

namespace rage
{
fiDeviceImplemented::fiDeviceImplemented()
{
}

fiDeviceImplemented::~fiDeviceImplemented()
{

}

uint32_t WRAPPER fiDeviceImplemented::Open(const char* fileName, bool) { PURECALL(); }

uint32_t WRAPPER fiDeviceImplemented::OpenBulk(const char* fileName, uint64_t* ptr)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::CreateLocal(const char* fileName)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::Create(const char*)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::Read(uint32_t handle, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::ReadBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::WriteBulk(int, int, int, int, int)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::Write(int, void*, int)
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::Seek(uint32_t handle, int32_t distance, uint32_t method)
{
	PURECALL();
}

void WRAPPER fiDeviceImplemented::Close(uint32_t handle)
{
	PURECALL();
}

void WRAPPER fiDeviceImplemented::CloseBulk(uint32_t handle)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::GetFileLength(uint32_t handle)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::m_34()
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

LARGE_INTEGER WRAPPER fiDeviceImplemented::GetFileLengthLong(const char* file)
{
	PURECALL();
}

uint64_t WRAPPER fiDeviceImplemented::GetFileTime(const char* file)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::SetFileTime(const char* file, FILETIME fileTime)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::FindFirst(const char* path, fiFindData* findData)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::FindNext(int handle, fiFindData* findData)
{
	PURECALL();
}

int WRAPPER fiDeviceImplemented::FindClose(int handle)
{
	PURECALL();
}

bool WRAPPER fiDeviceImplemented::Truncate(uint32_t handle)
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

int WRAPPER fiDeviceImplemented::m_6C()
{
	PURECALL();
}

uint32_t WRAPPER fiDeviceImplemented::GetParentHandle()
{
	PURECALL();
}
}