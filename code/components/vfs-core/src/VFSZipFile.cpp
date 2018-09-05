/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <VFSZipFile.h>

#include <VFSManager.h>

#include <mz.h>
#include <mz_strm.h>
#include <mz_zip.h>

int32_t mz_stream_vfs_open(void *stream, const char *path, int32_t mode);
int32_t mz_stream_vfs_is_open(void *stream);
int32_t mz_stream_vfs_read(void *stream, void *buf, int32_t size);
int32_t mz_stream_vfs_write(void *stream, const void *buf, int32_t size);
int64_t mz_stream_vfs_tell(void *stream);
int32_t mz_stream_vfs_seek(void *stream, int64_t offset, int32_t origin);
int32_t mz_stream_vfs_close(void *stream);
int32_t mz_stream_vfs_error(void *stream);

void*   mz_stream_vfs_create(void **stream);
void    mz_stream_vfs_delete(void **stream);

void*   mz_stream_vfs_get_interface(void);

// minizip stream for VFS
static mz_stream_vtbl mz_stream_vfs_vtbl = {
	mz_stream_vfs_open,
	mz_stream_vfs_is_open,
	mz_stream_vfs_read,
	mz_stream_vfs_write,
	mz_stream_vfs_tell,
	mz_stream_vfs_seek,
	mz_stream_vfs_close,
	mz_stream_vfs_error,
	mz_stream_vfs_create,
	mz_stream_vfs_delete,
	NULL,
	NULL
};

/***************************************************************************/

typedef struct mz_stream_vfs_s
{
	mz_stream       stream;
	uint64_t        offset;
	uint64_t		baseOffset;

	bool owned;
	fwRefContainer<vfs::Device> parentDevice;
	vfs::Device::THandle parentHandle;
} mz_stream_vfs;

int32_t mz_stream_vfs_open(void *stream, const char *path, int32_t mode)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;

	if (!path)
	{
		return MZ_STREAM_ERROR;
	}

	if ((mode & MZ_OPEN_MODE_READWRITE) != MZ_OPEN_MODE_READ)
	{
		return MZ_STREAM_ERROR;
	}

	vfs->parentDevice = vfs::GetDevice(path);

	if (!vfs->parentDevice.GetRef())
	{
		return MZ_STREAM_ERROR;
	}

	vfs->parentHandle = vfs->parentDevice->OpenBulk(path, &vfs->baseOffset);
	vfs->offset = 0;
	vfs->owned = true;

	if (vfs->parentHandle == INVALID_DEVICE_HANDLE)
	{
		return MZ_STREAM_ERROR;
	}

	return MZ_OK;
}

int32_t mz_stream_vfs_reuse(void *stream, fwRefContainer<vfs::Device> device, vfs::Device::THandle handle, uint64_t ptr)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;

	vfs->owned = false;
	vfs->parentDevice = device;
	vfs->parentHandle = handle;
	vfs->baseOffset = ptr;
	vfs->offset = 0;

	return MZ_OK;
}

int32_t mz_stream_vfs_is_open(void *stream)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;
	if (!vfs->parentDevice.GetRef() || vfs->parentHandle == INVALID_DEVICE_HANDLE)
	{
		return MZ_STREAM_ERROR;
	}

	return MZ_OK;
}

int32_t mz_stream_vfs_read(void *stream, void *buf, int32_t size)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;
	uint32_t read = 0;

	if (mz_stream_vfs_is_open(stream) != MZ_OK)
		return MZ_STREAM_ERROR;

	read = vfs->parentDevice->ReadBulk(vfs->parentHandle, vfs->baseOffset + vfs->offset, buf, size);
	vfs->offset += read;

	return read;
}

int32_t mz_stream_vfs_write(void *stream, const void *buf, int32_t size)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;
	uint32_t written = 0;

	if (mz_stream_vfs_is_open(stream) != MZ_OK)
		return MZ_STREAM_ERROR;

	return MZ_STREAM_ERROR;
}

int64_t mz_stream_vfs_tell(void *stream)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;

	if (mz_stream_vfs_is_open(stream) != MZ_OK)
		return MZ_STREAM_ERROR;

	return vfs->offset;
}

int32_t mz_stream_vfs_seek(void *stream, int64_t offset, int32_t origin)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;
	int move_method = 0xFFFFFFFF;


	if (mz_stream_vfs_is_open(stream) == MZ_STREAM_ERROR)
		return MZ_STREAM_ERROR;

	switch (origin)
	{
	case MZ_SEEK_CUR:
		vfs->offset += offset;
		break;
	case MZ_SEEK_END:
		vfs->offset = vfs->parentDevice->GetLength(vfs->parentHandle) - offset;
		break;
	case MZ_SEEK_SET:
		vfs->offset = offset;
		break;
	default:
		return MZ_STREAM_ERROR;
	}

	return MZ_OK;
}

int mz_stream_vfs_close(void *stream)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;

	if (vfs->owned)
	{
		vfs->parentDevice->CloseBulk(vfs->parentHandle);
		vfs->parentDevice = nullptr;
	}

	return MZ_OK;
}

int mz_stream_vfs_error(void *stream)
{
	mz_stream_vfs *vfs = (mz_stream_vfs *)stream;
	return 42;
}

void *mz_stream_vfs_create(void **stream)
{
	mz_stream_vfs *vfs = NULL;

	vfs = new mz_stream_vfs;
	if (vfs != NULL)
	{
		vfs->stream.vtbl = &mz_stream_vfs_vtbl;
	}
	if (stream != NULL)
		*stream = vfs;

	return vfs;
}

void mz_stream_vfs_delete(void **stream)
{
	mz_stream_vfs *vfs = NULL;
	if (stream == NULL)
		return;
	vfs = (mz_stream_vfs *)*stream;
	if (vfs != NULL)
	{
		delete vfs;
	}
	*stream = NULL;
}

void *mz_stream_vfs_get_interface(void)
{
	return (void *)&mz_stream_vfs_vtbl;
}

namespace vfs
{
	ZipFile::ZipFile()
		: m_parentHandle(InvalidHandle)
	{

	}

	ZipFile::~ZipFile()
	{
		if (m_parentHandle != InvalidHandle)
		{
			m_parentDevice->CloseBulk(m_parentHandle);

			m_parentHandle = InvalidHandle;
		}
	}

	bool ZipFile::OpenArchive(const std::string& archivePath)
	{
		// get the containing device, and early out if we don't have one
		fwRefContainer<Device> parentDevice = vfs::GetDevice(archivePath);

		if (!parentDevice.GetRef())
		{
			return false;
		}

		// attempt opening the archive from the specified path
		m_parentHandle = parentDevice->OpenBulk(archivePath, &m_parentPtr);

		// early out if invalid, again
		if (m_parentHandle == InvalidHandle)
		{
			return false;
		}

		m_parentDevice = parentDevice;

		// open a MZ stream
		void* inStream;
		mz_stream_vfs_create(&inStream);

		if (mz_stream_vfs_reuse(inStream, m_parentDevice, m_parentHandle, m_parentPtr) != MZ_OK)
		{
			mz_stream_vfs_delete(&inStream);
			return false;
		}

		void* inZip;
		mz_zip_create(&inZip);

		if (mz_zip_open(inZip, inStream, MZ_OPEN_MODE_READ) != MZ_OK)
		{
			mz_stream_vfs_delete(&inStream);
			mz_zip_delete(&inZip);

			return false;
		}

		int err = mz_zip_goto_first_entry(inZip);

		while (err == MZ_OK)
		{
			mz_zip_file* file;
			if (mz_zip_entry_get_info(inZip, &file) == MZ_OK)
			{
				Entry e;
				e.entryOffset = mz_zip_get_entry(inZip);
				e.fileSize = file->uncompressed_size;

				m_entries[file->filename] = e;
			}

			err = mz_zip_goto_next_entry(inZip);
		}

		mz_zip_delete(&inZip);
		mz_stream_vfs_delete(&inStream);

		// return a success value
		return true;
	}

	const ZipFile::Entry* ZipFile::FindEntry(const std::string& path)
	{
		// first, remove the path prefix
		std::string relativePath = path.substr(m_pathPrefix.length() + 1);

		// TODO: strip excessive slashes
		auto it = m_entries.find(relativePath);

		if (it != m_entries.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	ZipFile::HandleData* ZipFile::AllocateHandle(THandle* outHandle)
	{
		for (int i = 0; i < _countof(m_handles); i++)
		{
			if (!m_handles[i].valid)
			{
				*outHandle = i;

				return &m_handles[i];
			}
		}

		return nullptr;
	}

	ZipFile::HandleData* ZipFile::GetHandle(THandle inHandle)
	{
		if (inHandle >= 0 && inHandle < _countof(m_handles))
		{
			if (m_handles[inHandle].valid)
			{
				return &m_handles[inHandle];
			}
		}

		return nullptr;
	}

	ZipFile::THandle ZipFile::Open(const std::string& fileName, bool readOnly)
	{
		if (readOnly)
		{
			auto entry = FindEntry(fileName);

			if (entry)
			{
				THandle handle;
				auto handleData = AllocateHandle(&handle);

				if (handleData)
				{
					handleData->valid = true;
					handleData->entry = *entry;
					handleData->curOffset = 0;

					void* inStream;
					mz_stream_vfs_create(&inStream);

					if (mz_stream_vfs_reuse(inStream, m_parentDevice, m_parentHandle, m_parentPtr) != MZ_OK)
					{
						mz_stream_vfs_delete(&inStream);
						return InvalidHandle;
					}

					void* inZip;
					mz_zip_create(&inZip);

					if (mz_zip_open(inZip, inStream, MZ_OPEN_MODE_READ) != MZ_OK)
					{
						mz_stream_vfs_delete(&inStream);
						mz_zip_delete(&inZip);

						return InvalidHandle;
					}

					mz_zip_goto_entry(inZip, entry->entryOffset);

					if (mz_zip_entry_read_open(inZip, 0, nullptr) != MZ_OK)
					{
						mz_stream_vfs_delete(&inStream);
						mz_zip_delete(&inZip);

						return InvalidHandle;
					}

					handleData->mzStream = inStream;
					handleData->mzZip = inZip;

					return handle;
				}
			}
		}

		return InvalidHandle;
	}

	ZipFile::THandle ZipFile::OpenBulk(const std::string& fileName, uint64_t* ptr)
	{
		return InvalidHandle;
	}

	size_t ZipFile::Read(THandle handle, void* outBuffer, size_t size)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			size_t read = mz_zip_entry_read(handleData->mzZip, outBuffer, size);
			handleData->curOffset += read;

			return read;
		}

		return -1;
	}

	size_t ZipFile::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
	{
		return -1;
	}

	bool ZipFile::Close(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			mz_zip_close(handleData->mzZip);
			mz_zip_delete(&handleData->mzZip);

			mz_stream_close(handleData->mzStream);
			mz_stream_delete(&handleData->mzStream);

			handleData->valid = false;

			return true;
		}

		return false;
	}

	bool ZipFile::CloseBulk(THandle handle)
	{
		return true;
	}

	size_t ZipFile::Seek(THandle handle, intptr_t offset, int seekType)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			auto seekFromNow = [&]() -> uint64_t
			{
				uint8_t fakeBuf[4096];
				uint32_t res = 0;
				int64_t i;

				for (i = 0; i < intptr_t(offset - sizeof(fakeBuf)); i += sizeof(fakeBuf)) {
					res = mz_zip_entry_read(handleData->mzZip, fakeBuf, sizeof(fakeBuf));

					if (res < sizeof(fakeBuf))
					{
						return -1;
					}
				}

				res = i + mz_zip_entry_read(handleData->mzZip, fakeBuf, offset - i);

				handleData->curOffset += offset;

				return handleData->curOffset;
			};

			if (seekType == SEEK_CUR)
			{
				return seekFromNow();
			}
			else if (seekType == SEEK_SET || seekType == SEEK_END)
			{
				if (seekType == SEEK_END)
				{
					offset = handleData->entry.fileSize - offset;
				}

				mz_zip_goto_entry(handleData->mzZip, handleData->entry.entryOffset);
				mz_zip_entry_read_open(handleData->mzZip, 0, nullptr);

				handleData->curOffset = 0;

				if (offset <= 0)
				{
					return 0;
				}

				return seekFromNow();
			}
			else
			{
				return -1;
			}

			return handleData->curOffset;
		}

		return -1;
	}

	size_t ZipFile::GetLength(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			return handleData->entry.fileSize;
		}

		return -1;
	}

	size_t ZipFile::GetLength(const std::string& fileName)
	{
		auto entry = FindEntry(fileName);

		if (entry)
		{
			return entry->fileSize;
		}

		return -1;
	}

	ZipFile::THandle ZipFile::FindFirst(const std::string& folder, FindData* findData)
	{
		return InvalidHandle;
	}

	bool ZipFile::FindNext(THandle handle, FindData* findData)
	{
		return false;
	}

	void ZipFile::FindClose(THandle handle)
	{
		auto handleData = GetHandle(handle);

		if (handleData)
		{
			handleData->valid = false;
		}
	}

	void ZipFile::FillFindData(FindData* data, const Entry* entry)
	{
		
	}

	void ZipFile::SetPathPrefix(const std::string& pathPrefix)
	{
		m_pathPrefix = pathPrefix.substr(0, pathPrefix.find_last_not_of('/') + 1);
	}
}
