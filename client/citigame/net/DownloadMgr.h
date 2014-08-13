#pragma once

#include "NetLibrary.h"
#include "ResourceCache.h"
#include "fiDevice.h"
#include "ResourceManager.h"
#include <memory>

class ResourceData;

class DownloadManager
{
private:
	NetAddress m_gameServer;

	HttpClient* m_httpClient;

	std::string m_serverLoadScreen;

	// queue for runtime-updated resources
	std::queue<std::string> m_updateQueue;

	std::vector<ResourceData> m_requiredResources;

	std::queue<ResourceDownload> m_downloadList;

	std::vector<std::pair<std::string, rage::fiPackfile*>> m_packFiles;

	std::unordered_set<std::string> m_removedPackFiles;

	std::shared_ptr<ResourceDownload> m_currentDownload;

	std::vector<StreamingResource> m_streamingFiles;

	std::list<std::shared_ptr<Resource>> m_loadedResources;

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

public:
	bool Process();

	bool DoingQueuedUpdate();

	void QueueResourceUpdate(std::string resourceName);

	void ReleaseLastServer();

	void AddStreamingFile(ResourceData data, std::string& filename, std::string& hash, uint32_t rscFlags, uint32_t rscVersion, uint32_t size);

	void SetServer(NetAddress& address);

public:
	inline std::vector<StreamingResource>& GetStreamingFiles() { return m_streamingFiles; }

	inline std::list<std::shared_ptr<Resource>>& GetLoadedResources() { return m_loadedResources; }

	inline void ClearLoadedResources() { m_loadedResources.clear(); }
};

extern DownloadManager TheDownloads;