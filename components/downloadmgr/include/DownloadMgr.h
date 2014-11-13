#pragma once

#include "NetLibrary.h"
#include "ResourceCache.h"
#include "fiDevice.h"
#include "ResourceManager.h"
#include <memory>

class ResourceData;

///
/// Metadata for a configuration request.
///
struct ConfigurationRequestData
{
	// The name of the server the file originates from.
	fwString serverName;
};

class 
#ifdef COMPILING_DOWNLOADMGR
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	DownloadManager
{
private:
	NetAddress m_gameServer;

	HttpClient* m_httpClient;

	std::string m_serverLoadScreen;

	// queue for runtime-updated resources
	std::queue<fwString> m_updateQueue;

	fwVector<ResourceData> m_requiredResources;

	std::queue<ResourceDownload> m_downloadList;

	std::vector<std::pair<fwString, rage::fiPackfile*>> m_packFiles;

	std::unordered_set<std::string> m_removedPackFiles;

	std::shared_ptr<ResourceDownload> m_currentDownload;

	fwVector<StreamingResource> m_streamingFiles;

	std::list<fwRefContainer<Resource>> m_loadedResources;

	// Mutex for parsing/adding download data.
	std::mutex m_parseMutex;

	// The amount of pending configuration requests.
	uint32_t m_numPendingConfigRequests;

	enum DownloadState
	{
		DS_IDLE,
		DS_FETCHING_CONFIG,
		DS_CONFIG_FETCHED,
		DS_DOWNLOADING,
		DS_DOWNLOADED_SINGLE,
		DS_DONE
	} m_downloadState;

	bool m_isUpdate;

private:
	void ProcessQueuedUpdates();

	void ParseConfiguration(const ConfigurationRequestData& request, bool result, const char* connDataStr, size_t connDataLength);

	void InitiateChildRequest(fwString url);

public:
	bool Process();

	bool DoingQueuedUpdate();

	void QueueResourceUpdate(fwString resourceName);

	void ReleaseLastServer();

	void AddStreamingFile(ResourceData data, fwString& filename, fwString& hash, uint32_t rscFlags, uint32_t rscVersion, uint32_t size);

	void SetServer(NetAddress& address);

public:
	inline fwVector<StreamingResource>& GetStreamingFiles() { return m_streamingFiles; }

	inline std::list<fwRefContainer<Resource>>& GetLoadedResources() { return m_loadedResources; }

	inline void ClearLoadedResources() { m_loadedResources.clear(); }
};

extern
#ifdef COMPILING_DOWNLOADMGR
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	DownloadManager TheDownloads;