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

int WRAPPER fiCustomDevice::m_40(int a1)
{
	return m_parentDeviceRef->m_40(a1);
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

void WRAPPER fiCustomDevice::m_xx()
{
	return m_parentDeviceRef->m_xx();
}

void* WRAPPER fiCustomDevice::m_xy(void* a, int b, void* c)
{
	return m_parentDeviceRef->m_xy(a, b, c);
}

bool WRAPPER fiCustomDevice::m_xz()
{
	return m_parentDeviceRef->m_xz();
}

int WRAPPER fiCustomDevice::m_yy()
{
	return m_parentDeviceRef->m_yy();
}

int WRAPPER fiCustomDevice::m_yz(void* a)
{
	return m_parentDeviceRef->m_yz(a);
}

int WRAPPER fiCustomDevice::m_zx(void* a)
{
	return m_parentDeviceRef->m_zx(a);
}

bool WRAPPER fiCustomDevice::IsBulkDevice()
{
	return m_parentDeviceRef->IsBulkDevice();
}

fiDevice* WRAPPER fiCustomDevice::m_zz()
{
	return m_parentDeviceRef->m_zz();
}

bool WRAPPER fiCustomDevice::m_ax()
{
	return m_parentDeviceRef->m_ax();
}

int WRAPPER fiCustomDevice::GetCollectionId()
{
	return m_parentDeviceRef->GetCollectionId();
}

uint64_t fiCustomDevice::OpenBulkWrap(const char* fileName, uint64_t* ptr, void*)
{
	return m_parentDeviceRef->OpenBulk(fileName, ptr);
}

fiDevice* fiCustomDevice::GetUnkDevice()
{
	return m_parentDeviceRef->GetUnkDevice();
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
	char* location = hook::pattern("48 21 35 ? ? ? ? 48 8B 74 24 38 48 8D 05").count(1).get(0).get<char>(15);

	rage::g_vTable_fiDevice = (uintptr_t)(location + *(int32_t*)location + 4);
});