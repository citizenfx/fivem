/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fiCustomDevice.h"
#include <Hooking.h>

#define PURECALL() __asm { jmp _purecall }

namespace rage
{
fiCustomDevice::fiCustomDevice()
{
	static auto loc = *(uintptr_t*)(hook::pattern("56 8B F1 C7 06 ? ? ? ? E8 32 00 00 00 F6 44").count(1).get(0).get<char>(21));
	*(uintptr_t*)(&m_parentDeviceData[0]) = loc;

	m_parentDeviceRef = reinterpret_cast<rage::fiDevice*>(m_parentDeviceData);
}

fiCustomDevice::~fiCustomDevice()
{

}

uint32_t fiCustomDevice::Open(const char* fileName, bool readOnly)
{
	return m_parentDeviceRef->Open(fileName, readOnly);
}

uint32_t fiCustomDevice::OpenBulk(const char* fileName, uint64_t* ptr)
{
	return m_parentDeviceRef->OpenBulk(fileName, ptr);
}

uint32_t fiCustomDevice::CreateLocal(const char* fileName)
{
	return m_parentDeviceRef->CreateLocal(fileName);
}

uint32_t fiCustomDevice::Create(const char* fileName)
{
	return m_parentDeviceRef->Create(fileName);
}

uint32_t fiCustomDevice::Read(uint32_t handle, void* buffer, uint32_t toRead)
{
	return m_parentDeviceRef->Read(handle, buffer, toRead);
}

uint32_t fiCustomDevice::ReadBulk(uint32_t handle, uint64_t ptr, void* buffer, uint32_t toRead)
{
	return m_parentDeviceRef->ReadBulk(handle, ptr, buffer, toRead);
}

uint32_t fiCustomDevice::WriteBulk(int a1, int a2, int a3, int a4, int a5)
{
	return m_parentDeviceRef->WriteBulk(a1, a2, a3, a4, a5);
}

uint32_t fiCustomDevice::Write(uint32_t a1, void* a2, int a3)
{
	return m_parentDeviceRef->Write(a1, a2, a3);
}

uint32_t fiCustomDevice::Seek(uint32_t handle, int32_t distance, uint32_t method)
{
	return m_parentDeviceRef->Seek(handle, distance, method);
}

uint64_t fiCustomDevice::SeekLong(uint32_t handle, int64_t distance, uint32_t method)
{
	return m_parentDeviceRef->SeekLong(handle, distance, method);
}

int32_t fiCustomDevice::Close(uint32_t handle)
{
	return m_parentDeviceRef->Close(handle);
}

int32_t fiCustomDevice::CloseBulk(uint32_t handle)
{
	return m_parentDeviceRef->CloseBulk(handle);
}

int fiCustomDevice::GetFileLength(uint32_t handle)
{
	return m_parentDeviceRef->GetFileLength(handle);
}

int fiCustomDevice::m_34()
{
	return m_parentDeviceRef->m_34();
}

bool fiCustomDevice::RemoveFile(const char* file)
{
	return false;
}

int fiCustomDevice::RenameFile(const char* from, const char* to)
{
	return false;
}

int fiCustomDevice::CreateDirectory(const char* dir)
{
	return false;
}

int fiCustomDevice::RemoveDirectory(const char * dir)
{
	return false;
}

uint64_t fiCustomDevice::GetFileLengthLong(const char* file)
{
	return m_parentDeviceRef->GetFileLengthLong(file);
}

uint64_t fiCustomDevice::GetFileTime(const char* file)
{
	return m_parentDeviceRef->GetFileTime(file);
}

bool fiCustomDevice::SetFileTime(const char* file, FILETIME fileTime)
{
	return m_parentDeviceRef->SetFileTime(file, fileTime);
}

size_t fiCustomDevice::FindFirst(const char* path, fiFindData* findData)
{
	return m_parentDeviceRef->FindFirst(path, findData);
}

bool fiCustomDevice::FindNext(size_t handle, fiFindData* findData)
{
	return m_parentDeviceRef->FindNext(handle, findData);
}

int fiCustomDevice::FindClose(size_t handle)
{
	return m_parentDeviceRef->FindClose(handle);
}

bool fiCustomDevice::Truncate(uint32_t handle)
{
	return false;
}

uint32_t fiCustomDevice::GetFileAttributes(const char* path)
{
	return m_parentDeviceRef->GetFileAttributes(path);
}

bool fiCustomDevice::SetFileAttributes(const char* file, uint32_t FileAttributes)
{
	return m_parentDeviceRef->SetFileAttributes(file, FileAttributes);
}

int fiCustomDevice::m_6C()
{
	return m_parentDeviceRef->m_6C();
}

uint32_t fiCustomDevice::GetParentHandle()
{
	return m_parentDeviceRef->GetParentHandle();
}
}
