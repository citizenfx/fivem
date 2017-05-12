#include <StdInc.h>
#include <LocalDevice.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

namespace vfs
{
Device::THandle LocalDevice::Open(const std::string& fileName, bool readOnly)
{
	int fd = open(fileName.c_str(), (readOnly) ? O_RDONLY : O_RDWR);

	if (fd < 0)
	{
		return INVALID_DEVICE_HANDLE;
	}

	return static_cast<THandle>(fd);
}

Device::THandle LocalDevice::OpenBulk(const std::string& fileName, uint64_t* ptr)
{
	*ptr = 0;
	return Open(fileName, true);
}

Device::THandle LocalDevice::Create(const std::string& filename)
{
	int fd = creat(filename.c_str(), 0755);

	if (fd < 0)
	{
		return INVALID_DEVICE_HANDLE;
	}

	return static_cast<THandle>(fd);
}

size_t LocalDevice::Read(THandle handle, void* outBuffer, size_t size)
{
	assert(handle != Device::InvalidHandle);

	ssize_t bytesRead = read(static_cast<int>(handle), outBuffer, size);

	return (bytesRead < 0) ? -1 : static_cast<size_t>(bytesRead);
}

size_t LocalDevice::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	assert(handle != Device::InvalidHandle);

	ssize_t bytesRead = pread(static_cast<int>(handle), outBuffer, size, static_cast<off_t>(ptr));

	return (bytesRead < 0) ? -1 : static_cast<size_t>(bytesRead);
}

size_t LocalDevice::Write(THandle handle, const void* buffer, size_t size)
{
	assert(handle != Device::InvalidHandle);

	ssize_t bytesWritten = write(static_cast<int>(handle), buffer, size);

	return (bytesWritten < 0) ? -1 : static_cast<size_t>(bytesWritten);
}

size_t LocalDevice::WriteBulk(THandle handle, uint64_t ptr, const void* buffer, size_t size)
{
	assert(!"Not implemented!");

	return -1;
}

size_t LocalDevice::Seek(THandle handle, intptr_t offset, int seekType)
{
	assert(handle != Device::InvalidHandle);

	return static_cast<size_t>(lseek(static_cast<int>(handle), static_cast<off_t>(offset), seekType));
}

bool LocalDevice::Close(THandle handle)
{
	return (close(handle) == 0);
}

bool LocalDevice::CloseBulk(THandle handle)
{
	return (close(handle) == 0);
}

bool LocalDevice::RemoveFile(const std::string& filename)
{
	return (unlink(filename.c_str()) == 0);
}

bool LocalDevice::RenameFile(const std::string& from, const std::string& to)
{
	return (rename(from.c_str(), to.c_str()) != 0);
}

bool LocalDevice::CreateDirectory(const std::string& name)
{
	return (mkdir(name.c_str(), 0755) == 0);
}

bool LocalDevice::RemoveDirectory(const std::string& name)
{
	return (rmdir(name.c_str()) == 0);
}

size_t LocalDevice::GetLength(THandle handle)
{
	struct stat buf;
	fstat(static_cast<int>(handle), &buf);

	return buf.st_size;
}

Device::THandle LocalDevice::FindFirst(const std::string& folder, FindData* findData)
{
	DIR* dir = opendir(folder.c_str());

	if (dir)
	{
		dirent* entry = readdir(dir);

		if (entry)
		{
			findData->name = entry->d_name;
			findData->attributes = 0;
			findData->length = 0; // TODO: implement

			return reinterpret_cast<Device::THandle>(dir);
		}
	}

	return INVALID_DEVICE_HANDLE;
}

bool LocalDevice::FindNext(THandle handle, FindData* findData)
{
	dirent* entry = readdir(reinterpret_cast<DIR*>(handle));

	if (entry)
	{
		findData->name = entry->d_name;
		findData->attributes = 0;
		findData->length = 0; // TODO: implement

		return true;
	}

	return false;
}

void LocalDevice::FindClose(THandle handle)
{
	closedir(reinterpret_cast<DIR*>(handle));
}
}

#ifdef EMSCRIPTEN
extern "C" int __cxa_thread_atexit(void(*)(char*), char*, char*)
{
	return 0;
}
#endif
