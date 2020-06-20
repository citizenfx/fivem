#include <StdInc.h>
#include <LocalDevice.h>

#include <VFSLinkExtension.h>
#include <VFSManager.h>

#include <windows.h>

#include <versionhelpers.h>

namespace vfs
{
Device::THandle LocalDevice::Open(const std::string& fileName, bool readOnly)
{
	std::wstring wideName = ToWide(fileName);

	HANDLE hFile = CreateFileW(wideName.c_str(),
	    (readOnly) ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
	    FILE_SHARE_READ | FILE_SHARE_WRITE,
	    nullptr,
	    OPEN_EXISTING,
	    FILE_ATTRIBUTE_NORMAL,
	    nullptr);

	return reinterpret_cast<THandle>(hFile);
}

Device::THandle LocalDevice::OpenBulk(const std::string& fileName, uint64_t* ptr)
{
	std::wstring wideName = ToWide(fileName);
	*ptr                  = 0;

	HANDLE hFile = CreateFileW(wideName.c_str(),
	    GENERIC_READ,
	    FILE_SHARE_READ,
	    nullptr,
	    OPEN_EXISTING,
	    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
	    nullptr);

	return reinterpret_cast<THandle>(hFile);
}

Device::THandle LocalDevice::Create(const std::string& filename)
{
	std::wstring wideName = ToWide(filename);

	HANDLE hFile = CreateFileW(wideName.c_str(),
	    GENERIC_READ | GENERIC_WRITE,
	    FILE_SHARE_READ,
	    nullptr,
	    CREATE_ALWAYS,
	    FILE_ATTRIBUTE_NORMAL,
	    nullptr);

	return reinterpret_cast<THandle>(hFile);
}

size_t LocalDevice::Read(THandle handle, void* outBuffer, size_t size)
{
	assert(handle != Device::InvalidHandle);

	DWORD bytesRead;
	BOOL result = ReadFile(reinterpret_cast<HANDLE>(handle), outBuffer, static_cast<DWORD>(size), &bytesRead, nullptr);

	return (!result) ? -1 : bytesRead;
}

size_t LocalDevice::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	assert(handle != Device::InvalidHandle);

	OVERLAPPED overlapped = {};
	overlapped.Offset     = (ptr & 0xFFFFFFFF);
	overlapped.OffsetHigh = ptr >> 32;

	BOOL result    = ReadFile(reinterpret_cast<HANDLE>(handle), outBuffer, static_cast<DWORD>(size), nullptr, &overlapped);
	bool ioPending = false;

	if (!result)
	{
		ioPending = (GetLastError() == ERROR_IO_PENDING);

		if (!ioPending)
		{
			return -1;
		}
	}

	// wait for the IO to complete in a polling fashion
	// this should be safe enough as nobody expects IO to be time-critical
	if (ioPending)
	{
		while (!HasOverlappedIoCompleted(&overlapped))
		{
			Sleep(5);
		}
	}

	// get error result/bytes transferred the 'safe' way (one could also use Internal/InternalHigh, but despite
	// being standardized those are scarcely documented)
	DWORD bytesRead;
	if (!GetOverlappedResult(reinterpret_cast<HANDLE>(handle), &overlapped, &bytesRead, FALSE))
	{
		return -1;
	}

	return bytesRead;
}

size_t LocalDevice::Write(THandle handle, const void* buffer, size_t size)
{
	assert(handle != Device::InvalidHandle);

	DWORD bytesWritten;
	BOOL result = WriteFile(reinterpret_cast<HANDLE>(handle), buffer, static_cast<DWORD>(size), &bytesWritten, nullptr);

	return (result) ? bytesWritten : -1;
}

size_t LocalDevice::WriteBulk(THandle handle, uint64_t ptr, const void* buffer, size_t size)
{
	assert(!"Not implemented!");

	return -1;
}

size_t LocalDevice::Seek(THandle handle, intptr_t offset, int seekType)
{
	assert(handle != Device::InvalidHandle);

	DWORD moveMethod;

	if (seekType == SEEK_SET)
	{
		moveMethod = FILE_BEGIN;
	}
	else if (seekType == SEEK_CUR)
	{
		moveMethod = FILE_CURRENT;
	}
	else if (seekType == SEEK_END)
	{
		moveMethod = FILE_END;
	}

	return SetFilePointer(reinterpret_cast<HANDLE>(handle), static_cast<LONG>(offset), nullptr, moveMethod);
}

bool LocalDevice::Close(THandle handle)
{
	return CloseHandle(reinterpret_cast<HANDLE>(handle)) != FALSE;
}

bool LocalDevice::CloseBulk(THandle handle)
{
	return Close(handle);
}

bool LocalDevice::RemoveFile(const std::string& filename)
{
	std::wstring wideName = ToWide(filename);
	return DeleteFile(wideName.c_str()) != FALSE;
}

bool LocalDevice::RenameFile(const std::string& from, const std::string& to)
{
	std::wstring fromName = ToWide(from);
	std::wstring toName   = ToWide(to);

	return MoveFile(fromName.c_str(), toName.c_str()) != FALSE;
}

bool LocalDevice::CreateDirectory(const std::string& name)
{
	std::wstring wideName = ToWide(name);

	return ::CreateDirectoryW(wideName.c_str(), nullptr) != FALSE;
}

bool LocalDevice::RemoveDirectory(const std::string& name)
{
	std::wstring wideName = ToWide(name);

	return ::RemoveDirectoryW(wideName.c_str()) != FALSE;
}

std::time_t LocalDevice::GetModifiedTime(const std::string& fileName)
{
	THandle handle = Open(fileName, true);
	
	if (handle != InvalidHandle)
	{
		FILETIME lastWriteTime;
		GetFileTime(reinterpret_cast<HANDLE>(handle), nullptr, nullptr, &lastWriteTime);

		Close(handle);

		ULARGE_INTEGER li;
		li.HighPart = lastWriteTime.dwHighDateTime;
		li.LowPart = lastWriteTime.dwLowDateTime;

		return li.QuadPart / 10000000ULL - 11644473600ULL;
	}

	return 0;
}

size_t LocalDevice::GetLength(THandle handle)
{
	DWORD highPortion;
	DWORD lowPortion = GetFileSize(reinterpret_cast<HANDLE>(handle), &highPortion);

	return lowPortion | (static_cast<size_t>(highPortion) << 32);
}

Device::THandle LocalDevice::FindFirst(const std::string& folder, FindData* findData)
{
	std::wstring wideName = ToWide(folder + "/*");

	WIN32_FIND_DATA winFindData;
	HANDLE hFind = FindFirstFile(wideName.c_str(), &winFindData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		return InvalidHandle;
	}

	findData->attributes = winFindData.dwFileAttributes;
	findData->length     = winFindData.nFileSizeLow;
	findData->name       = ToNarrow(winFindData.cFileName);

	return reinterpret_cast<THandle>(hFind);
}

bool LocalDevice::FindNext(THandle handle, FindData* findData)
{
	WIN32_FIND_DATA winFindData;
	BOOL result = FindNextFile(reinterpret_cast<HANDLE>(handle), &winFindData);

	if (result)
	{
		findData->attributes = winFindData.dwFileAttributes;
		findData->length     = winFindData.nFileSizeLow;
		findData->name       = ToNarrow(winFindData.cFileName);
	}

	return result != FALSE;
}

void LocalDevice::FindClose(THandle handle)
{
	::FindClose(reinterpret_cast<HANDLE>(handle));
}

bool LocalDevice::ExtensionCtl(int controlIdx, void* controlData, size_t controlSize)
{
	if (controlIdx == VFS_GET_FILE_ID && controlSize == sizeof(GetFileIdExtension))
	{
		auto data = reinterpret_cast<GetFileIdExtension*>(controlData);

		if (IsWindows8OrGreater())
		{
			FILE_ID_INFO info;
			BOOL result = GetFileInformationByHandleEx(reinterpret_cast<HANDLE>(data->handle), FileIdInfo, &info, sizeof(info));

			if (result)
			{
				data->fileId = vfs::FileId{};
				memcpy(data->fileId.data(), &info.FileId.Identifier, std::min(sizeof(info.FileId.Identifier), data->fileId.size()));

				return true;
			}
		}
		else
		{
			BY_HANDLE_FILE_INFORMATION info;
			BOOL result = GetFileInformationByHandle(reinterpret_cast<HANDLE>(data->handle), &info);

			if (result)
			{
				ULARGE_INTEGER fileInt;
				fileInt.LowPart = info.nFileIndexLow;
				fileInt.HighPart = info.nFileIndexHigh;

				data->fileId = vfs::FileId{};
				memcpy(data->fileId.data(), &fileInt, std::min(sizeof(fileInt), data->fileId.size()));

				return true;
			}
		}
	}
	else if (controlIdx == VFS_MAKE_HARDLINK && controlSize == sizeof(MakeHardLinkExtension))
	{
		auto data = reinterpret_cast<MakeHardLinkExtension*>(controlData);

		auto deviceRef = vfs::GetDevice(data->existingPath);

		if (!deviceRef.GetRef())
		{
			return false;
		}

		auto handle = deviceRef->Open(data->existingPath, true);

		if (handle == INVALID_DEVICE_HANDLE)
		{
			return false;
		}

		wchar_t filePath[MAX_PATH] = { 0 };
		auto plen = GetFinalPathNameByHandleW(reinterpret_cast<HANDLE>(handle), filePath, std::size(filePath) - 1, FILE_NAME_NORMALIZED);

		deviceRef->Close(handle);

		if (plen == 0)
		{
			return false;
		}

		bool success = CreateHardLinkW(ToWide(data->newPath).c_str(), filePath, NULL) != FALSE;

		if (!success)
		{
			auto le = GetLastError();

			if (le == ERROR_ALREADY_EXISTS)
			{
				success = true;
			}
		}

		return success;
	}

	return false;
}
}
