#pragma once

#include "NetLibrary.h"
#include "ResourceCache.h"
#include <memory>

class ResourceData;

class DownloadManager
{
private:
	NetAddress m_gameServer;

	HttpClient* m_httpClient;

	std::vector<ResourceData> m_requiredResources;

	std::queue<ResourceDownload> m_downloadList;

	std::shared_ptr<ResourceDownload> m_currentDownload;

	enum DownloadState
	{
		DS_IDLE,
		DS_FETCHING_CONFIG,
		DS_CONFIG_FETCHED,
		DS_DOWNLOADING,
		DS_DOWNLOADED_SINGLE,
		DS_DONE
	} m_downloadState;

public:
	bool Process();

	void SetServer(NetAddress& address);
};

extern DownloadManager TheDownloads;