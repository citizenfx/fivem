#pragma once

#include <unordered_set>
#include <mutex>
#include "fiDevice.h"

struct ResourceDownload
{
	fwString sourceUrl;
	fwString targetFilename;
	fwString filename;
	fwString resname;
};

class ResourceData;

struct ResourceFile
{
	fwString filename;

	fwString hash;
};

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#endif
	ResourceData
{
private:
	fwString m_name;

	fwVector<ResourceFile> m_files;

	fwString m_baseUrl;

	bool m_processed;

public:
	inline ResourceData() {}

	ResourceData(fwString name, fwString baseUrl);

	void AddFile(fwString filename, fwString hash);

	inline fwString GetBaseURL() const { return m_baseUrl; }

	inline const fwVector<ResourceFile>& GetFiles() const { return m_files; }

	inline fwString GetName() const { return m_name; }

	inline bool IsProcessed() const { return m_processed;  }

	inline void SetProcessed() { m_processed = true; }
};

struct StreamingResource : ResourceFile
{
	ResourceData resData;
	uint32_t rscFlags;
	uint32_t rscVersion;
	uint32_t size;
};

struct CacheEntry
{
	fwString resource;
	fwString filename;
	fwString hash;

	bool operator==(const CacheEntry& right) const;
	bool operator!=(const CacheEntry& right) const;
};

namespace std
{
	template<>
	struct hash < CacheEntry >
	{
		size_t operator()(const CacheEntry& entry) const
		{
			return std::hash<fwString>()(entry.filename) ^ (std::hash<fwString>()(entry.resource) << 1) ^ (std::hash<fwString>()(entry.hash) << 2);
		}
	};
}

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#endif
	ResourceCache
{
private:
	fwVector<CacheEntry> m_cache;

	std::unordered_set<CacheEntry> m_cacheSet;

	std::unordered_map<fwString, CacheEntry> m_markList;

	rage::fiDevice* m_cacheDevice;

	std::mutex m_dataLock;

private:
	void AddEntry(fwString fileName, fwString resourceName, fwString hash);

	bool ParseFileName(const char* inString, fwString& fileNameOut, fwString& resourceNameOut, fwString& hashOut);

public:
	ResourceDownload GetResourceDownload(const ResourceData& resource, const ResourceFile& file);

	void Initialize();

	void LoadCache(rage::fiDevice* device);

	void AddFile(fwString& sourcePath, fwString& filename, fwString& resource);

	void ClearMark();

	void MarkList(fwVector<ResourceData>& resourceList);

	void MarkStreamingList(fwVector<StreamingResource>& streamList);

	inline rage::fiDevice* GetCacheDevice() { return m_cacheDevice; }

	fwString GetMarkedFilenameFor(fwString resource, fwString filename);

	fwVector<ResourceDownload> GetDownloadsFromList(fwVector<ResourceData>& resourceList);
};