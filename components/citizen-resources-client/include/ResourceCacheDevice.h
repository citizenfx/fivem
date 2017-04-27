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

struct IgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#endif
	ResourceCacheEntryList : public fwRefCountable, public fx::IAttached<fx::Resource>
{
public:
	struct Entry
	{
		std::string resourceName;
		std::string basename;
		std::string remoteUrl;
		std::string referenceHash;
		size_t size;

		inline Entry()
		{

		}

		inline Entry(const std::string& resourceName, const std::string& basename, const std::string& remoteUrl, const std::string& referenceHash, size_t size)
			: resourceName(resourceName), basename(basename), remoteUrl(remoteUrl), referenceHash(referenceHash), size(size)
		{

		}
	};

private:
	fx::Resource* m_parentResource;

	std::map<std::string, Entry, IgnoreCaseLess> m_entries;

public:
	virtual void AttachToObject(fx::Resource* resource) override;

	inline const std::map<std::string, Entry, IgnoreCaseLess>& GetEntries()
	{
		return m_entries;
	}

	inline boost::optional<Entry> GetEntry(const std::string& baseName)
	{
		auto it = m_entries.find(baseName);

		if (it == m_entries.end())
		{
			return boost::optional<Entry>();
		}

		return boost::optional<Entry>(it->second);
	}

	inline void AddEntry(const Entry& entry)
	{
		m_entries[entry.basename] = entry;
		m_entries[entry.basename].resourceName = m_parentResource->GetName();
	}
};

DECLARE_INSTANCE_TYPE(ResourceCacheEntryList);

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	ResourceCacheDevice : public vfs::Device
{
protected:
	struct HandleData
	{
		enum
		{
			StatusEmpty,
			StatusNotFetched,
			StatusFetching,
			StatusFetched,
			StatusError
		} status;

		fwRefContainer<vfs::Device> parentDevice;

		vfs::Device::THandle parentHandle;

		uint64_t bulkPtr;

		ResourceCacheEntryList::Entry entry;

		bool bulkHandle;

		std::map<std::string, std::string> metaData;

		std::mutex lockMutex;
		std::condition_variable lockVar;

		inline HandleData()
			: status(StatusEmpty), parentHandle(vfs::Device::InvalidHandle)
		{

		}
	};

protected:
	bool m_blocking;

	std::shared_ptr<ResourceCache> m_cache;

	HttpClient* m_httpClient;

	HandleData m_handles[512];

	std::mutex m_handleLock;

	std::string m_pathPrefix;

	std::string m_cachePath;

public:
	ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking);

	ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking, const std::string& cachePath);

protected:
	boost::optional<ResourceCacheEntryList::Entry> GetEntryForFileName(const std::string& fileName);

	HandleData* AllocateHandle(THandle* idx);

	THandle OpenInternal(const std::string& fileName, uint64_t* bulkPtr);

	bool EnsureFetched(HandleData* handleData);

	virtual void AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, HandleData* handleData);

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
};