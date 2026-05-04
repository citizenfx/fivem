#include <StdInc.h>

#include <VFSManager.h>
#include <RelativeDevice.h>

#include <VFSLinkExtension.h>

namespace vfs
{
RelativeDevice::RelativeDevice(const std::string& otherPrefix)
    : RelativeDevice(GetDevice(otherPrefix), otherPrefix)
{
}

RelativeDevice::RelativeDevice(const fwRefContainer<Device>& otherDevice, const std::string& otherPrefix)
    : m_otherDevice(otherDevice), m_otherPrefix(otherPrefix)
{
}

Device::THandle RelativeDevice::Open(const std::string& fileName, bool readOnly, bool append)
{
	auto path = TranslatePath(fileName);
	if (path.empty()) return InvalidHandle;
	return m_otherDevice->Open(path, readOnly, append);
}

Device::THandle RelativeDevice::OpenBulk(const std::string& fileName, uint64_t* ptr)
{
	auto path = TranslatePath(fileName);
	if (path.empty()) return InvalidHandle;
	return m_otherDevice->OpenBulk(path, ptr);
}

Device::THandle RelativeDevice::Create(const std::string& filename, bool createIfExists, bool append)
{
	auto path = TranslatePath(filename);
	if (path.empty()) return InvalidHandle;
	return m_otherDevice->Create(path, createIfExists, append);
}

size_t RelativeDevice::Read(THandle handle, void* outBuffer, size_t size)
{
	return m_otherDevice->Read(handle, outBuffer, size);
}

size_t RelativeDevice::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	return m_otherDevice->ReadBulk(handle, ptr, outBuffer, size);
}

size_t RelativeDevice::Write(THandle handle, const void* buffer, size_t size)
{
	return m_otherDevice->Write(handle, buffer, size);
}

size_t RelativeDevice::WriteBulk(THandle handle, uint64_t ptr, const void* buffer, size_t size)
{
	return m_otherDevice->WriteBulk(handle, ptr, buffer, size);
}

size_t RelativeDevice::Seek(THandle handle, intptr_t offset, int seekType)
{
	return m_otherDevice->Seek(handle, offset, seekType);
}

bool RelativeDevice::Close(THandle handle)
{
	return m_otherDevice->Close(handle);
}

bool RelativeDevice::CloseBulk(THandle handle)
{
	return m_otherDevice->CloseBulk(handle);
}

bool RelativeDevice::RemoveFile(const std::string& filename)
{
	auto path = TranslatePath(filename);
	if (path.empty()) return false;
	return m_otherDevice->RemoveFile(path);
}

bool RelativeDevice::RenameFile(const std::string& from, const std::string& to)
{
	auto pathFrom = TranslatePath(from);
	auto pathTo = TranslatePath(to);
	if (pathFrom.empty() || pathTo.empty()) return false;
	return m_otherDevice->RenameFile(pathFrom, pathTo);
}

bool RelativeDevice::CreateDirectory(const std::string& name)
{
	auto path = TranslatePath(name);
	if (path.empty()) return false;
	return m_otherDevice->CreateDirectory(path);
}

bool RelativeDevice::RemoveDirectory(const std::string& name)
{
	auto path = TranslatePath(name);
	if (path.empty()) return false;
	return m_otherDevice->RemoveDirectory(path);
}

std::time_t RelativeDevice::GetModifiedTime(const std::string& fileName)
{
	auto path = TranslatePath(fileName);
	if (path.empty()) return 0;
	return m_otherDevice->GetModifiedTime(path);
}

size_t RelativeDevice::GetLength(THandle handle)
{
	return m_otherDevice->GetLength(handle);
}

size_t RelativeDevice::GetLength(const std::string& fileName)
{
	auto path = TranslatePath(fileName);
	if (path.empty()) return -1;
	return m_otherDevice->GetLength(path);
}

uint32_t RelativeDevice::GetAttributes(const std::string& fileName)
{
	auto path = TranslatePath(fileName);
	if (path.empty()) return -1;
	return m_otherDevice->GetAttributes(path);
}

Device::THandle RelativeDevice::FindFirst(const std::string& folder, FindData* findData)
{
	auto path = TranslatePath(folder);
	if (path.empty()) return InvalidHandle;
	return m_otherDevice->FindFirst(path, findData);
}

bool RelativeDevice::FindNext(THandle handle, FindData* findData)
{
	return m_otherDevice->FindNext(handle, findData);
}

void RelativeDevice::FindClose(THandle handle)
{
	return m_otherDevice->FindClose(handle);
}

bool RelativeDevice::ExtensionCtl(int controlIdx, void* controlData, size_t controlSize)
{
	if (controlIdx == VFS_MAKE_HARDLINK)
	{
		auto data = (vfs::MakeHardLinkExtension*)controlData;
		data->newPath = TranslatePath(data->newPath);
		if (data->newPath.empty()) return false;

		return m_otherDevice->ExtensionCtl(controlIdx, data, controlSize);
	}

	return false;
}

void RelativeDevice::SetPathPrefix(const std::string& pathPrefix)
{
	m_pathPrefix = pathPrefix;
}

bool RelativeDevice::Flush(THandle handle)
{
	return m_otherDevice->Flush(handle);
}

bool RelativeDevice::Truncate(THandle handle, uint64_t length)
{
	return m_otherDevice->Truncate(handle, length);
}
}
