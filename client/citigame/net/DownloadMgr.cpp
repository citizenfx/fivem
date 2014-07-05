#include "StdInc.h"
#include "ResourceManager.h"
#include "DownloadMgr.h"
#include <SHA1.h>
#include <yaml-cpp/yaml.h>
#include "../ui/CefOverlay.h"

bool DownloadManager::Process()
{
	switch (m_downloadState)
	{
		case DS_IDLE:
		{
			m_downloadState = DS_FETCHING_CONFIG;

			m_httpClient = new HttpClient();

			std::wstring hostname = m_gameServer.GetWAddress();

			std::map<std::string, std::string> postMap;
			postMap["method"] = "getConfiguration";

			bool isUpdate = false;

			if (!m_updateQueue.empty())
			{
				std::string resourceString;

				while (!m_updateQueue.empty())
				{
					resourceString += m_updateQueue.front();

					m_updateQueue.pop();

					if (!m_updateQueue.empty())
					{
						resourceString += ";";
					}
				}

				postMap["resources"] = resourceString;

				isUpdate = true;
			}

			m_httpClient->DoPostRequest(hostname, m_gameServer.GetPort(), L"/client", postMap, [&] (bool result, std::string connData)
			{
				if (!result)
				{
					// TODO: make this a non-fatal error leading back to UI
					GlobalError("Obtaining configuration from server failed.");

					return;
				}

				if (!isUpdate)
				{
					m_requiredResources.clear();
				}

				std::string serverHost = m_gameServer.GetAddress();
				serverHost += va(":%d", m_gameServer.GetPort());

				// parse the received YAML file
				try
				{
					auto node = YAML::Load(connData);

					for (auto& resource : node["resources"])
					{
						std::string baseUrl = node["fileServer"].as<std::string>();

						if (resource["fileServer"].IsDefined())
						{
							baseUrl = resource["fileServer"].as<std::string>();
						}

						ResourceData resData(resource["name"].as<std::string>(), va(baseUrl.c_str(), serverHost.c_str()));
						
						for (auto& file : resource["files"])
						{
							std::string filename = file.first.as<std::string>();
							std::string hash = file.second.as<std::string>();

							resData.AddFile(filename, hash);
						}

						if (isUpdate)
						{
							for (auto it = m_requiredResources.begin(); it != m_requiredResources.end(); it++)
							{
								if (it->GetName() == resData.GetName())
								{
									m_requiredResources.erase(it);
									break;
								}
							}
						}

						m_isUpdate = isUpdate;

						m_requiredResources.push_back(resData);
					}
				}
				catch (YAML::Exception& e)
				{
					GlobalError("YAML parsing error in server configuration (%s)", e.msg);

					return;
				}

				m_downloadState = DS_CONFIG_FETCHED;
			});

			break;
		}

		case DS_CONFIG_FETCHED:
		{
			// check cache existence (TODO: and integrity?)
			auto resourceCache = TheResources.GetCache();

			auto downloadList = resourceCache->GetDownloadsFromList(m_requiredResources);

			resourceCache->ClearMark();
			resourceCache->MarkList(m_requiredResources);

			m_downloadList = std::queue<ResourceDownload>();

			for (auto& download : downloadList)
			{
				m_downloadList.push(download);
			}
			
			if (m_downloadList.empty())
			{
				m_downloadState = DS_DOWNLOADED_SINGLE;
			}
			else
			{
				m_downloadState = DS_DOWNLOADING;
			}

			break;
		}

		case DS_DOWNLOADING:
		{
			if (!m_currentDownload.get())
			{
				m_currentDownload = std::make_shared<ResourceDownload>(m_downloadList.front());
				m_downloadList.pop();

				std::wstring hostname, path;
				uint16_t port;

				m_httpClient->CrackUrl(m_currentDownload->sourceUrl, hostname, path, port);

				m_httpClient->DoFileGetRequest(hostname, port, path, TheResources.GetCache()->GetCacheDevice(), m_currentDownload->targetFilename, [&] (bool result, std::string connData)
				{
					m_downloadState = DS_DOWNLOADED_SINGLE;
				});
			}

			break;
		}

		case DS_DOWNLOADED_SINGLE:
		{
			if (m_currentDownload.get())
			{
				TheResources.GetCache()->AddFile(m_currentDownload->targetFilename, m_currentDownload->filename, m_currentDownload->resname);

				m_currentDownload = nullptr;
			}

			if (!m_downloadList.empty())
			{
				m_downloadState = DS_DOWNLOADING;
			}
			else
			{
				if (!m_isUpdate)
				{
					TheResources.Reset();
				}
				else
				{
					// unload any resources we already know that are currently unprocessed
					for (auto& resource : m_requiredResources)
					{
						// this is one we just got from the configuration redownload
						if (!resource.IsProcessed())
						{
							auto resourceData = TheResources.GetResource(resource.GetName());

							if (!resourceData.get())
							{
								continue;
							}

							// sanity check: is the resource not running?
							if (resourceData->GetState() == ResourceStateRunning)
							{
								FatalError("Tried to unload a running resource in DownloadMgr. (%s)", resource.GetName().c_str());
							}

							// remove all packfiles related to this old resource
							auto packfiles = resourceData->GetPackFiles();

							for (auto& packfile : packfiles)
							{
								// FIXME: implementation detail from same class
								fiDevice::Unmount(va("resources:/%s/", resourceData->GetName().c_str()));

								packfile->closeArchive();

								// remove from the to-close list (!)
								for (auto it = m_packFiles.begin(); it != m_packFiles.end(); it++)
								{
									if (it->second == packfile)
									{
										m_packFiles.erase(it);
										break;
									}
								}
							}

							// and delete the resource (hope nobody kept a reference to that sucker, ha!)
							TheResources.DeleteResource(resourceData);
						}
					}
				}

				//std::string resourcePath = "citizen:/resources/";
				//TheResources.ScanResources(fiDevice::GetDevice("citizen:/setup2.xml", true), resourcePath);

				std::list<std::shared_ptr<Resource>> loadedResources;

				// mount any RPF files that we include
				for (auto& resource : m_requiredResources)
				{
					if (m_isUpdate && resource.IsProcessed())
					{
						continue;
					}

					std::vector<rage::fiPackfile*> packFiles;

					for (auto& file : resource.GetFiles())
					{
						if (file.filename.find(".rpf") != std::string::npos)
						{
							// get the path of the RPF
							std::string markedFile = TheResources.GetCache()->GetMarkedFilenameFor(resource.GetName(), file.filename);

							rage::fiPackfile* packFile = new rage::fiPackfile();
							packFile->openArchive(markedFile.c_str(), true, false, 0);
							packFile->mount(va("resources:/%s/", resource.GetName().c_str()));

							packFiles.push_back(packFile);
							m_packFiles.push_back(std::make_pair(va("resources:/%s/", resource.GetName().c_str()), packFile));
						}
					}

					// load the resource
					auto resourceLoad = TheResources.AddResource(resource.GetName(), va("resources:/%s/", resource.GetName().c_str()));

					if (resourceLoad.get())
					{
						resourceLoad->AddPackFiles(packFiles);

						loadedResources.push_back(resourceLoad);
					}

					resource.SetProcessed();
				}

				// and start all of them!
				for (auto& resource : loadedResources)
				{
					if (resource->GetState() == ResourceStateStopped)
					{
						resource->Start();
					}
				}

				m_downloadState = DS_DONE;
			}

			break;
		}

		case DS_DONE:
			m_downloadState = DS_IDLE;

			if (m_isUpdate && !m_updateQueue.empty())
			{
				ProcessQueuedUpdates();
			}

			m_isUpdate = false;

			g_netLibrary->DownloadsComplete();

			while (!g_netLibrary->ProcessPreGameTick())
			{
				HANDLE hThread = GetCurrentThread();

				MsgWaitForMultipleObjects(1, &hThread, TRUE, 15, 0);
			}
			
			return true;
	}

	return false;
}

bool DownloadManager::DoingQueuedUpdate()
{
	return m_isUpdate && m_downloadState != DS_IDLE;
}

void DownloadManager::QueueResourceUpdate(std::string resourceName)
{
	m_updateQueue.push(resourceName);

	if (m_downloadState == DS_IDLE)
	{
		ProcessQueuedUpdates();
	}
}

void DownloadManager::ProcessQueuedUpdates()
{
	m_downloadState = DS_IDLE;

	Process();
}

void DownloadManager::ReleaseLastServer()
{
	for (auto& packfile : m_packFiles)
	{
		fiDevice::Unmount(packfile.first.c_str());

		packfile.second->closeArchive();
	}

	//nui::ReloadNUI();
	for (auto& resource : m_requiredResources)
	{
		nui::DestroyFrame(resource.GetName());
	}
}

void DownloadManager::SetServer(NetAddress& address)
{
	m_gameServer = address;
}

DownloadManager TheDownloads;