#pragma once

#include <tbb/concurrent_unordered_map.h>
#include <concurrent_queue.h>

#include <optional>

#include <ResourceCache.h>

#include <VFSManager.h>
#include <VFSStreamDevice.h>

struct HttpRequestHandle;
using HttpRequestPtr = std::shared_ptr<HttpRequestHandle>;

namespace resources
{
class RcdFetcher;

class RcdFetchFailedException : public std::runtime_error
{
public:
	RcdFetchFailedException(const std::string& reason)
		: std::runtime_error(fmt::sprintf("Failed to fetch: %s", reason))
	{
	}
};

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

public:
	virtual bool EnsureRead(const std::function<void(bool, const std::string&)>& cb = {});

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
	inline RcdBulkStream(RcdFetcher* fetcher, const std::string& fileName, size_t realSize = 0)
		: RcdBaseStream(fetcher, fileName), m_realSize(realSize)
	{

	}

	virtual ~RcdBulkStream() override;

	virtual size_t ReadBulk(uint64_t ptr, void* outBuffer, size_t size);

protected:
	virtual vfs::Device::THandle OpenFile(const std::string& localPath) override;

	virtual void CloseFile() override;

protected:
	size_t m_realSize;

private:
	uint64_t m_parentPtr = 0;
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

	virtual void UnfetchEntry(const std::string& fileName) = 0;

	virtual std::optional<std::reference_wrapper<const ResourceCacheEntryList::Entry>> GetEntryForFileName(std::string_view fileName) = 0;

	virtual size_t GetLength(const std::string& fileName) = 0;

	virtual bool ExistsOnDisk(const std::string& fileName) = 0;

	virtual void PropagateError(const std::string& error) = 0;
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

	virtual std::shared_ptr<RcdStream> CreateStream(const std::string& fileName, bool createIfExists = true) override;

	virtual std::shared_ptr<RcdBulkStream> OpenBulkStream(const std::string& fileName, uint64_t* ptr) override;

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override;

	virtual size_t GetLength(const std::string& fileName) override;

	virtual concurrency::task<RcdFetchResult> FetchEntry(const std::string& fileName) override;

	virtual void UnfetchEntry(const std::string& fileName) override;

	virtual inline bool IsBlocking() override
	{
		return m_blocking;
	}

	virtual bool ExistsOnDisk(const std::string& fileName) override;

	void SetPathPrefix(const std::string& pathPrefix) override
	{
		m_pathPrefix = pathPrefix;
	}

	void PropagateError(const std::string& error) override
	{
		m_lastError = error;
	}

protected:
	static std::mutex ms_lock;

public:
	std::optional<std::reference_wrapper<const ResourceCacheEntryList::Entry>> GetEntryForFileName(std::string_view fileName);

protected:
	virtual fwRefContainer<vfs::Stream> GetVerificationStream(const ResourceCacheEntryList::Entry& entry, const ResourceCache::Entry& cacheEntry);

	virtual void AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, const ResourceCacheEntryList::Entry& entry);

	virtual size_t GetRealSize(const ResourceCacheEntryList::Entry& entry);

private:
	concurrency::task<RcdFetchResult> DoFetch(const ResourceCacheEntryList::Entry& entry);

private:
	void StoreHttpRequest(const std::string& hash, const HttpRequestPtr& request);

	void RemoveHttpRequest(const std::string& hash);

	void SetRequestWeight(const std::string& hash, int newWeight);

private:
	std::shared_mutex m_pendingRequestWeightsMutex;
	std::unordered_map<std::string, int> m_pendingRequestWeights;

	std::shared_mutex m_requestMapMutex;
	std::unordered_map<std::string, HttpRequestPtr> m_requestMap;

protected:
	static tbb::concurrent_unordered_map<std::string, std::optional<concurrency::task<RcdFetchResult>>> ms_entries;

	std::shared_ptr<ResourceCache> m_cache;
	bool m_blocking;

	std::string m_pathPrefix;

	std::string m_lastError;

	concurrency::concurrent_queue<THandle> m_handleDeleteQueue;
};
}
