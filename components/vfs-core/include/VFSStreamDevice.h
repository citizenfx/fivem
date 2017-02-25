/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#pragma once

#include <VFSDevice.h>

#ifdef COMPILING_VFS_CORE
#define VFS_CORE_EXPORT DLL_EXPORT
#else
#define VFS_CORE_EXPORT DLL_IMPORT
#endif

#include <mutex>

namespace vfs
{
struct SeekableStream {};
struct WritableStream {};
struct BulkWritableStream {};

namespace detail
{
	template<typename StreamType>
	struct HandleData
	{
		std::shared_ptr<StreamType> stream;
		bool valid;
	};

	template<typename StreamType, typename BulkStreamType>
	struct HandleDataWithBulk
	{
		std::shared_ptr<StreamType> stream;
		std::shared_ptr<BulkStreamType> bulkStream;
		bool valid;
	};
}

template<class StreamType, class HandleDataType = detail::HandleData<StreamType>>
class StreamDevice : public Device
{
public:
	virtual THandle Open(const std::string& fileName, bool readOnly) override
	{
		auto lock = AcquireMutex();

		THandle handle;
		auto handleData = AllocateHandle(&handle);

		if (handleData)
		{
			auto ptr = OpenStream(fileName, readOnly);

			if (ptr)
			{
				handleData->stream = ptr;
				handleData->valid = true;

				return handle;
			}
		}

		return InvalidHandle;
	}

	virtual THandle Create(const std::string& filename) override
	{
		auto lock = AcquireMutex();

		THandle handle;
		auto handleData = AllocateHandle(&handle);

		if (handleData)
		{
			auto ptr = CreateStream(filename);

			if (ptr)
			{
				handleData->stream = ptr;
				handleData->valid = true;

				return handle;
			}
		}

		return InvalidHandle;
	}

	virtual size_t Read(THandle handle, void* outBuffer, size_t size) override
	{
		auto data = GetHandle(handle);

		if (data && data->stream)
		{
			return data->stream->Read(outBuffer, size);
		}

		return -1;
	}

private:
	template<bool value>
	struct WriteImpl
	{
		static size_t Write(StreamType* stream, const void* buffer, size_t size)
		{
			return -1;
		}
	};

	template<>
	struct WriteImpl<true>
	{
		static size_t Write(StreamType* stream, const void* buffer, size_t size)
		{
			return stream->Write(buffer, size);
		}
	};

public:
	virtual size_t Write(THandle handle, const void* buffer, size_t size) override
	{
		auto data = GetHandle(handle);

		if (data && data->stream)
		{
			return WriteImpl<std::is_base_of_v<WritableStream, StreamType>>::Write(data->stream.get(), buffer, size);
		}

		return -1;
	}

private:
	template<bool value>
	struct SeekImpl
	{
		static size_t Seek(StreamType* stream, intptr_t offset, int seekType)
		{
			return -1;
		}
	};

	template<>
	struct SeekImpl<true>
	{
		static size_t Seek(StreamType* stream, intptr_t offset, int seekType)
		{
			return stream->Seek(offset, seekType);
		}
	};

public:
	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override
	{
		auto data = GetHandle(handle);

		if (data && data->stream)
		{
			return SeekImpl<std::is_base_of_v<SeekableStream, StreamType>>::Seek(data->stream.get(), offset, seekType);
		}

		return -1;
	}

	virtual bool Close(THandle handle) override
	{
		auto data = GetHandle(handle);

		if (data && data->stream)
		{
			data->stream.reset();
			data->valid = false;
			return true;
		}

		return false;
	}

	virtual THandle FindFirst(const std::string& folder, FindData* findData) override
	{
		return InvalidHandle;
	}

	virtual bool FindNext(THandle handle, FindData* findData) override
	{
		return false;
	}

	virtual void FindClose(THandle handle) override
	{

	}

	virtual std::shared_ptr<StreamType> OpenStream(const std::string& fileName, bool readOnly) = 0;

	virtual std::shared_ptr<StreamType> CreateStream(const std::string& fileName) = 0;

protected:
	HandleDataType* AllocateHandle(THandle* handle)
	{
		for (int i = 0; i < m_handles.size(); i++)
		{
			if (!m_handles[i].valid)
			{
				*handle = i;

				return &m_handles[i];
			}
		}

		HandleDataType hd;
		hd.valid = false;

		*handle = m_handles.size();

		m_handles.push_back(hd);

		return &m_handles.back();
	}

	HandleDataType* GetHandle(THandle inHandle)
	{
		if (inHandle >= 0 && inHandle < m_handles.size())
		{
			if (m_handles[inHandle].valid)
			{
				return &m_handles[inHandle];
			}
		}

		return nullptr;
	}

protected:
	auto AcquireMutex()
	{
		return std::move(std::unique_lock<std::mutex>(m_mutex));
	}

private:
	std::vector<HandleDataType> m_handles;

	std::mutex m_mutex;
};

template<class StreamType, class BulkType>
class BulkStreamDevice : public StreamDevice<StreamType, detail::HandleDataWithBulk<StreamType, BulkType>>
{
public:
	virtual THandle OpenBulk(const std::string& fileName, uint64_t* bulkPtr) override
	{
		auto lock = AcquireMutex();

		THandle handle;
		auto handleData = AllocateHandle(&handle);

		if (handleData)
		{
			auto ptr = OpenBulkStream(fileName, bulkPtr);

			if (ptr)
			{
				handleData->bulkStream = ptr;
				handleData->valid = true;

				return handle;
			}
		}

		return InvalidHandle;
	}

	virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size) override
	{
		auto data = GetHandle(handle);

		if (data && data->bulkStream)
		{
			return data->bulkStream->ReadBulk(ptr, outBuffer, size);
		}

		return -1;
	}

private:
	template<bool value>
	struct WriteBulkImpl
	{
		static size_t Write(BulkType* stream, uint64_t ptr, const void* buffer, size_t size)
		{
			return -1;
		}
	};

	template<>
	struct WriteBulkImpl<true>
	{
		static size_t Write(BulkType* stream, uint64_t ptr, const void* buffer, size_t size)
		{
			return stream->WriteBulk(ptr, buffer, size);
		}
	};

public:
	virtual size_t WriteBulk(THandle handle, uint64_t ptr, const void* buffer, size_t size) override
	{
		auto data = GetHandle(handle);

		if (data && data->bulkStream)
		{
			return WriteBulkImpl<std::is_base_of_v<BulkWritableStream, BulkType>>::Write(data->bulkStream.get(), ptr, buffer, size);
		}

		return -1;
	}
	
	virtual bool CloseBulk(THandle handle) override
	{
		auto data = GetHandle(handle);

		if (data)
		{
			data->bulkStream.reset();
			data->valid = false;
			return true;
		}

		return false;
	}

public:
	virtual std::shared_ptr<BulkType> OpenBulkStream(const std::string& fileName, uint64_t* ptr) = 0;
};
}