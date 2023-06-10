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

#include <deque>
#include <mutex>

namespace vfs
{
struct SeekableStream {};
struct WritableStream {};
struct LengthableStream {};
struct BulkWritableStream {};

inline namespace detail
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

	template<bool value>
	struct GetLengthImpl
	{
		template<typename TStream>
		static size_t GetLength(TStream* stream)
		{
			return -1;
		}
	};

	template<>
	struct GetLengthImpl<true>
	{
		template<typename TStream>
		static size_t GetLength(TStream* stream)
		{
			return stream->GetLength();
		}
	};
}

template<class StreamType, class HandleDataType = detail::HandleData<StreamType>>
class StreamDevice : public Device
{
public:
	virtual THandle Open(const std::string& fileName, bool readOnly) override
	{
		auto ptr = OpenStream(fileName, readOnly);

		if (ptr)
		{
			THandle handle;
			auto handleData = AllocateHandle(&handle);

			if (handleData)
			{
				handleData->stream = ptr;

				return handle;
			}
		}

		return InvalidHandle;
	}

	virtual THandle Create(const std::string& filename) override
	{
		auto ptr = CreateStream(filename);

		if (ptr)
		{
			THandle handle;
			auto handleData = AllocateHandle(&handle);

			if (handleData)
			{
				handleData->stream = ptr;

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

	virtual size_t GetLength(THandle handle) override
	{
		auto data = GetHandle(handle);

		if (data && data->stream)
		{
			return GetLengthImpl<std::is_base_of_v<LengthableStream, StreamType>>::GetLength(data->stream.get());
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
		auto lock = AcquireMutex();

		for (int i = 0; i < m_handles.size(); i++)
		{
			if (!m_handles[i].valid)
			{
				*handle = i;
				m_handles[i].valid = true;

				return &m_handles[i];
			}
		}

		HandleDataType hd;
		hd.valid = true;

		*handle = m_handles.size();

		m_handles.push_back(hd);

		return &m_handles.back();
	}

	HandleDataType* GetHandle(THandle inHandle)
	{
		auto lock = AcquireMutex();

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
		return std::move(std::unique_lock(m_mutex));
	}

private:
	std::deque<HandleDataType> m_handles;

	std::mutex m_mutex;
};

template<class StreamType, class BulkType>
class BulkStreamDevice : public StreamDevice<StreamType, detail::HandleDataWithBulk<StreamType, BulkType>>
{
public:
	virtual Device::THandle OpenBulk(const std::string& fileName, uint64_t* bulkPtr) override
	{
		auto ptr = OpenBulkStream(fileName, bulkPtr);

		if (ptr)
		{
			THandle handle;
			auto handleData = AllocateHandle(&handle);

			if (handleData)
			{
				handleData->bulkStream = ptr;

				return handle;
			}
		}

		return InvalidHandle;
	}

	virtual size_t ReadBulk(Device::THandle handle, uint64_t ptr, void* outBuffer, size_t size) override
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
	virtual size_t WriteBulk(Device::THandle handle, uint64_t ptr, const void* buffer, size_t size) override
	{
		auto data = GetHandle(handle);

		if (data && data->bulkStream)
		{
			return WriteBulkImpl<std::is_base_of_v<BulkWritableStream, BulkType>>::Write(data->bulkStream.get(), ptr, buffer, size);
		}

		return -1;
	}

	virtual size_t GetLength(Device::THandle handle) override
	{
		auto data = GetHandle(handle);

		if (data && data->bulkStream)
		{
			return GetLengthImpl<std::is_base_of_v<LengthableStream, BulkType>>::GetLength(data->bulkStream.get());
		}

		return StreamDevice::GetLength(handle);
	}
	
	virtual bool CloseBulk(Device::THandle handle) override
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
