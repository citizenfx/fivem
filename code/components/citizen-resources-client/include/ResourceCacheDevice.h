/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "ResourceCache.h"

#include <condition_variable>

#include <Resource.h>
#include <VFSManager.h>
#include <HttpClient.h>

#include <ExtDownloader.h>

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	ResourceCacheDevice : public vfs::Device
{
public:
	struct FileData
	{
		HANDLE eventHandle;

		std::mutex fetchLock;

		std::map<std::string, std::string> metaData;

		volatile enum Status
		{
			StatusEmpty,
			StatusNotFetched,
			StatusFetching,
			StatusFetched,
			StatusError
		} status;

		char rscHeader[4];

		inline FileData()
			: status(StatusEmpty)
		{
			eventHandle = CreateEvent(nullptr, TRUE, FALSE, nullptr);

			rscHeader[0] = rscHeader[1] = rscHeader[2] = rscHeader[3] = 0;
		}

		inline ~FileData()
		{
			CloseHandle(eventHandle);
		}
	};

protected:
	struct HandleData
	{
		fwRefContainer<vfs::Device> parentDevice;

		vfs::Device::THandle parentHandle;

		uint64_t bulkPtr;

		ResourceCacheEntryList::Entry entry;

		bool bulkHandle;
		bool allocated;

		size_t downloadProgress;
		size_t downloadSize;

		HttpRequestPtr getRequest;
		std::shared_ptr<FileData> fileData;

		std::string extHandle;

		std::string localPath;
		std::string fileName;

		inline HandleData()
			: parentHandle(vfs::Device::InvalidHandle), downloadProgress(0), downloadSize(0), allocated(false)
		{

		}
	};

protected:
	bool m_blocking;

	std::shared_ptr<ResourceCache> m_cache;

	HttpClient* m_httpClient;

	std::shared_ptr<ExtDownloader> m_extDownloader;

	HandleData m_handles[512];

	std::mutex m_handleLock;

	std::string m_pathPrefix;

	std::string m_cachePath;

	std::string m_physCachePath;

public:
	ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking);

	ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking, const std::string& cachePath, const std::string& physCachePath);

protected:
	std::optional<ResourceCacheEntryList::Entry> GetEntryForFileName(const std::string& fileName);

	HandleData* AllocateHandle(THandle* idx);

	THandle OpenInternal(const std::string& fileName, uint64_t* bulkPtr);

	void EnsureDeferredOpen(THandle handle, HandleData* handleData);

	bool EnsureFetched(HandleData* handleData);

	virtual void AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, const ResourceCacheEntryList::Entry& entryRef);

	virtual void MarkFetched(HandleData* handleData);

	inline THandle GetHandleForData(HandleData* data)
	{
		return data - m_handles;
	}

public:
	virtual THandle Open(const std::string& fileName, bool readOnly) override;

	virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr) override;

	virtual size_t Read(THandle handle, void* outBuffer, size_t size) override;

	virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size) override;

	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override;

	virtual bool Close(THandle handle) override;

	virtual bool CloseBulk(THandle handle) override;

	virtual THandle FindFirst(const std::string& folder, vfs::FindData* findData) override;

	virtual bool FindNext(THandle handle, vfs::FindData* findData) override;

	virtual void FindClose(THandle handle) override;

	virtual void SetPathPrefix(const std::string& pathPrefix) override;

	virtual size_t GetLength(THandle handle) override;

	virtual size_t GetLength(const std::string& fileName) override;

	virtual uint32_t GetAttributes(const std::string& filename) override;

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override;
};
