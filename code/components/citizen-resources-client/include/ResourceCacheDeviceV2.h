#pragma once

#include <tbb/concurrent_unordered_map.h>

#include <optional>

#include <ResourceCache.h>

#include <VFSManager.h>
#include <VFSStreamDevice.h>

namespace resources
{
class RcdFetcher;

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	RcdBaseStream : public vfs::LengthableStream
{
#ifdef RCD_BASE_INSERT
	RCD_BASE_INSERT
#endif

public:
	inline RcdBaseStream(RcdFetcher* fetcher, const std::string& fileName)
		: m_fetcher(fetcher), m_fileName(fileName), m_parentHandle(INVALID_DEVICE_HANDLE)
	{

	}

	virtual ~RcdBaseStream() = default;

public:
	size_t GetLength();

protected:
	virtual vfs::Device::THandle OpenFile(const std::string& localPath) = 0;

	virtual void CloseFile() = 0;

	virtual bool EnsureRead();

protected:
	RcdFetcher* m_fetcher;
	std::string m_fileName;

	fwRefContainer<vfs::Device> m_parentDevice;
	vfs::Device::THandle m_parentHandle;

	std::map<std::string, std::string> m_metaData;
};

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	RcdStream : public RcdBaseStream, public vfs::SeekableStream
{
public:
	inline RcdStream(RcdFetcher* fetcher, const std::string& fileName)
		: RcdBaseStream(fetcher, fileName)
	{

	}

	virtual ~RcdStream() override;

	virtual size_t Read(void* outBuffer, size_t size);

	virtual size_t Seek(intptr_t off, int at);

protected:
	virtual vfs::Device::THandle OpenFile(const std::string& localPath) override;

	virtual void CloseFile() override;
};

class 
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	RcdBulkStream : public RcdBaseStream
{
public:
	inline RcdBulkStream(RcdFetcher* fetcher, const std::string& fileName)
		: RcdBaseStream(fetcher, fileName)
	{

	}

	virtual ~RcdBulkStream() override;

	virtual size_t ReadBulk(uint64_t ptr, void* outBuffer, size_t size);

protected:
	virtual vfs::Device::THandle OpenFile(const std::string& localPath) override;

	virtual void CloseFile() override;

private:
	size_t m_parentPtr;
};

struct RcdFetchResult
{
	std::string localPath;
	std::map<std::string, std::string> metaData;
};

class RcdFetcher
{
public:
	virtual bool IsBlocking() = 0;

	virtual concurrency::task<RcdFetchResult> FetchEntry(const std::string& fileName) = 0;

	virtual std::optional<std::reference_wrapper<const ResourceCacheEntryList::Entry>> GetEntryForFileName(std::string_view fileName) = 0;

	virtual size_t GetLength(const std::string& fileName) = 0;

	virtual bool ExistsOnDisk(const std::string& fileName) = 0;
};

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	ResourceCacheDeviceV2 : public vfs::BulkStreamDevice<RcdStream, RcdBulkStream>, public RcdFetcher
{
public:
	ResourceCacheDeviceV2(const std::shared_ptr<ResourceCache>& cache, bool blocking);

	virtual std::shared_ptr<RcdStream> OpenStream(const std::string& fileName, bool readOnly) override;

	virtual std::shared_ptr<RcdStream> CreateStream(const std::string& fileName) override;

	virtual std::shared_ptr<RcdBulkStream> OpenBulkStream(const std::string& fileName, uint64_t* ptr) override;

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override;

	virtual size_t GetLength(const std::string& fileName) override;

	virtual concurrency::task<RcdFetchResult> FetchEntry(const std::string& fileName) override;

	virtual inline bool IsBlocking() override
	{
		return m_blocking;
	}

	virtual bool ExistsOnDisk(const std::string& fileName) override;

	void SetPathPrefix(const std::string& pathPrefix) override
	{
		m_pathPrefix = pathPrefix;
	}

protected:
	static std::mutex ms_lock;

public:
	std::optional<std::reference_wrapper<const ResourceCacheEntryList::Entry>> GetEntryForFileName(std::string_view fileName);

protected:
	virtual fwRefContainer<vfs::Stream> GetVerificationStream(const ResourceCacheEntryList::Entry& entry, const ResourceCache::Entry& cacheEntry);

	virtual void AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, const ResourceCacheEntryList::Entry& entry);

private:
	concurrency::task<RcdFetchResult> DoFetch(const ResourceCacheEntryList::Entry& entry);

protected:
	static tbb::concurrent_unordered_map<std::string, concurrency::task<RcdFetchResult>> ms_entries;

	std::shared_ptr<ResourceCache> m_cache;
	bool m_blocking;

	std::string m_pathPrefix;
};
}
