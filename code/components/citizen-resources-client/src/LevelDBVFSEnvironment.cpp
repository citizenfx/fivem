/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#undef DeleteFile
#include <leveldb/env.h>

#include <VFSManager.h>

#include <mutex>

namespace leveldb
{
	class VFSEnvironment : public EnvWrapper
	{
	public:
		VFSEnvironment(Env* baseEnvironment)
			: EnvWrapper(baseEnvironment)
		{

		}

	public:
		virtual Status NewSequentialFile(const std::string& f, SequentialFile** r) override;

		virtual Status NewRandomAccessFile(const std::string& fname,
										   RandomAccessFile** result) override;

		virtual Status NewWritableFile(const std::string& fname,
									   WritableFile** result) override;

		virtual bool FileExists(const std::string& fname) override;

		virtual Status GetChildren(const std::string& dir,
								   std::vector<std::string>* result) override;

		virtual Status DeleteFile(const std::string& fname) override;

		virtual Status CreateDir(const std::string& dirname) override;

		virtual Status DeleteDir(const std::string& dirname) override;

		virtual Status GetFileSize(const std::string& fname, uint64_t* file_size) override;

		virtual Status RenameFile(const std::string& src,
								  const std::string& target) override;

		virtual Status LockFile(const std::string& fname, FileLock** lock) override;

		virtual Status UnlockFile(FileLock* lock) override;
	};

	class VFSSequentialFile : public SequentialFile
	{
	private:
		fwRefContainer<vfs::Device> m_device;

		vfs::Device::THandle m_handle;

	public:
		VFSSequentialFile(fwRefContainer<vfs::Device> device, vfs::Device::THandle handle);

		virtual ~VFSSequentialFile() override;

		virtual Status Read(size_t n, Slice* result, char* scratch) override;

		virtual Status Skip(uint64_t n) override;
	};

	VFSSequentialFile::VFSSequentialFile(fwRefContainer<vfs::Device> device, vfs::Device::THandle handle)
		: m_device(device), m_handle(handle)
	{

	}

	VFSSequentialFile::~VFSSequentialFile()
	{
		m_device->Close(m_handle);
	}

	Status VFSSequentialFile::Read(size_t n, Slice* result, char* scratch)
	{
		size_t read = m_device->Read(m_handle, scratch, n);

		if (read == -1)
		{
			return Status::IOError("m_device->Read failed");
		}

		*result = Slice(scratch, read);

		return Status();
	}

	Status VFSSequentialFile::Skip(uint64_t n)
	{
		m_device->Seek(m_handle, n, SEEK_CUR);

		return Status();
	}

	class VFSRandomAccessFile : public RandomAccessFile
	{
	private:
		fwRefContainer<vfs::Device> m_device;

		vfs::Device::THandle m_handle;

		uint64_t m_basePtr;

	public:
		VFSRandomAccessFile(fwRefContainer<vfs::Device> device, vfs::Device::THandle handle, uint64_t basePtr);

		virtual ~VFSRandomAccessFile() override;

		virtual Status Read(uint64_t offset, size_t n, Slice* result,
							char* scratch) const override; // (const? really?)

	private:
		Status ActualRead(uint64_t offset, size_t n, Slice* result, char* scratch);
	};

	VFSRandomAccessFile::VFSRandomAccessFile(fwRefContainer<vfs::Device> device, vfs::Device::THandle handle, uint64_t basePtr)
		: m_device(device), m_handle(handle), m_basePtr(basePtr)
	{

	}

	VFSRandomAccessFile::~VFSRandomAccessFile()
	{
		m_device->CloseBulk(m_handle);
	}

	Status VFSRandomAccessFile::Read(uint64_t offset, size_t n, Slice* result, char* scratch) const
	{
		// screw const
		return const_cast<VFSRandomAccessFile*>(this)->ActualRead(offset, n, result, scratch);
	}

	Status VFSRandomAccessFile::ActualRead(uint64_t offset, size_t n, Slice* result, char* scratch)
	{
		size_t read = m_device->ReadBulk(m_handle, m_basePtr + offset, scratch, n);

		if (read == -1)
		{
			return Status::IOError("m_device->ReadBulk failed");
		}

		*result = Slice(scratch, read);

		return Status();
	}

	class VFSWritableFile : public WritableFile
	{
	private:
		fwRefContainer<vfs::Device> m_device;

		vfs::Device::THandle m_handle;

	public:
		VFSWritableFile(fwRefContainer<vfs::Device> device, vfs::Device::THandle handle);

		virtual ~VFSWritableFile() override;

	public:
		virtual Status Append(const Slice& data) override;
		virtual Status Close() override;
		virtual Status Flush() override;
		virtual Status Sync() override;
	};

	VFSWritableFile::VFSWritableFile(fwRefContainer<vfs::Device> device, vfs::Device::THandle handle)
		: m_device(device), m_handle(handle)
	{

	}

	VFSWritableFile::~VFSWritableFile()
	{
		Close();
	}

	Status VFSWritableFile::Append(const Slice& data)
	{
		assert(m_handle != -1);

		size_t written = m_device->Write(m_handle, data.data(), data.size());

		return (written == data.size()) ? Status() : Status::IOError(va("m_device->Write with size %d only reports %d written", data.size(), written));
	}

	Status VFSWritableFile::Close()
	{
		if (m_handle != -1)
		{
			m_device->Close(m_handle);

			m_handle = -1;
		}

		return Status();
	}

	Status VFSWritableFile::Flush()
	{
		// we don't support flushing
		return Status();
	}

	Status VFSWritableFile::Sync()
	{
		// assume this is a valid NT handle (rage::fiDeviceLocal handles will typically be NT handles,
		// and rage::fiDeviceRelative will just wrap around these)
		vfs::FlushBuffersExtension flushBuffersCtl;
		flushBuffersCtl.handle = m_handle;

		m_device->ExtensionCtl(VFS_FLUSH_BUFFERS, &flushBuffersCtl, sizeof(flushBuffersCtl));

		return Status();
	}

	// vfsenvironment impl
	Status VFSEnvironment::NewSequentialFile(const std::string& f, SequentialFile** r)
	{
		// get a device handle
		fwRefContainer<vfs::Device> device = vfs::GetDevice(f);

		if (!device.GetRef())
		{
			*r = nullptr;

			return Status::IOError("vfs::GetDevice returned NULL.");
		}

		// open the file
		vfs::Device::THandle handle = device->Open(f, true);

		if (handle == vfs::Device::InvalidHandle)
		{
			*r = nullptr;

			return Status::IOError("vfs::Device::Open returned an invalid handle.");
		}

		*r = new VFSSequentialFile(device, handle);

		return Status();
	}

	Status VFSEnvironment::NewRandomAccessFile(const std::string& f, RandomAccessFile** r)
	{
		// get a device handle
		fwRefContainer<vfs::Device> device = vfs::GetDevice(f);

		if (!device.GetRef())
		{
			*r = nullptr;

			return Status::IOError("vfs::GetDevice returned NULL.");
		}

		// open the file
		uint64_t ptr = 0;

		vfs::Device::THandle handle = device->OpenBulk(f, &ptr);

		if (handle == vfs::Device::InvalidHandle)
		{
			*r = nullptr;

			return Status::IOError("vfs::Device::OpenBulk returned an invalid handle.");
		}

		*r = new VFSRandomAccessFile(device, handle, ptr);

		return Status();
	}

	Status VFSEnvironment::NewWritableFile(const std::string& fname, WritableFile** result)
	{
		fwRefContainer<vfs::Device> device = vfs::GetDevice(fname);

		if (!device.GetRef())
		{
			*result = nullptr;

			return Status::IOError("vfs::GetDevice returned NULL.");
		}

		vfs::Device::THandle handle = device->Create(fname);

		if (handle == vfs::Device::InvalidHandle)
		{
			*result = nullptr;

			return Status::IOError("vfs::Device::Create returned an invalid handle.");
		}

		*result = new VFSWritableFile(device, handle);

		return Status();
	}

	bool VFSEnvironment::FileExists(const std::string& fname)
	{
		fwRefContainer<vfs::Device> device = vfs::GetDevice(fname);

		if (!device.GetRef())
		{
			return false;
		}

		vfs::Device::THandle handle = device->Open(fname, true);

		if (handle != vfs::Device::InvalidHandle)
		{
			device->Close(handle);

			return true;
		}

		return false;
	}

	Status VFSEnvironment::GetChildren(const std::string& dir, std::vector<std::string>* result)
	{
		std::vector<std::string> files;
		fwRefContainer<vfs::Device> device = vfs::GetDevice(dir);

		if (!device.GetRef())
		{
			return Status::IOError("NULL device, blah blah");
		}

		vfs::FindData findData;
		vfs::Device::THandle handle = device->FindFirst(dir, &findData);

		if (handle != vfs::Device::InvalidHandle)
		{
			do 
			{
				files.push_back(findData.name);
			} while (device->FindNext(handle, &findData));

			device->FindClose(handle);
		}

		*result = files;

		return Status();
	}

	Status VFSEnvironment::DeleteFile(const std::string& fname)
	{
		fwRefContainer<vfs::Device> device = vfs::GetDevice(fname);

		if (!device.GetRef())
		{
			return Status::IOError("NULL device, blah blah");
		}

		return (device->RemoveFile(fname)) ? Status() : Status::IOError("Removing failed.");
	}

	Status VFSEnvironment::CreateDir(const std::string& dirname)
	{
		fwRefContainer<vfs::Device> device = vfs::GetDevice(dirname);

		if (!device.GetRef())
		{
			return Status::IOError("NULL device, blah blah");
		}

		return (device->CreateDirectory(dirname)) ? Status() : Status::IOError("Creating failed.");
	}

	Status VFSEnvironment::DeleteDir(const std::string& fname)
	{
		fwRefContainer<vfs::Device> device = vfs::GetDevice(fname);

		if (!device.GetRef())
		{
			return Status::IOError("NULL device, blah blah");
		}

		return (device->RemoveDirectory(fname)) ? Status() : Status::IOError("Removing failed.");
	}

	Status VFSEnvironment::RenameFile(const std::string& src, const std::string& target)
	{
		fwRefContainer<vfs::Device> device = vfs::GetDevice(src);

		if (!device.GetRef())
		{
			return Status::IOError("NULL device, blah blah");
		}

		return (device->RenameFile(src, target)) ? Status() : Status::IOError("Renaming failed.");
	}

	Status VFSEnvironment::GetFileSize(const std::string& fname, uint64_t* file_size)
	{
		fwRefContainer<vfs::Device> device = vfs::GetDevice(fname);

		if (!device.GetRef())
		{
			return Status::IOError("NULL device, blah blah");
		}

		*file_size = device->GetLength(fname);

		return Status();
	}

	class VFSFileLock : public FileLock
	{

	};

	Status VFSEnvironment::LockFile(const std::string& fname, FileLock** lock)
	{
		// no locking support for now
		*lock = new VFSFileLock();

		return Status();
	}

	Status VFSEnvironment::UnlockFile(FileLock* lock)
	{
		delete lock;

		return Status();
	}
}

leveldb::Env* GetVFSEnvironment()
{
	static std::once_flag initFlag;
	static leveldb::Env* vfsEnvironment;

	std::call_once(initFlag, [] ()
	{
		vfsEnvironment = new leveldb::VFSEnvironment(leveldb::Env::Default());
	});

	return vfsEnvironment;
}