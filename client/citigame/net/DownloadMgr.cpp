#include "StdInc.h"
#include "ResourceManager.h"
#include "DownloadMgr.h"
#include <SHA1.h>
#include <yaml-cpp/yaml.h>

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

			m_httpClient->DoPostRequest(hostname, m_gameServer.GetPort(), L"/client", postMap, [&] (bool result, std::string connData)
			{
				if (!result)
				{
					// TODO: make this a non-fatal error leading back to UI
					GlobalError("Obtaining configuration from server failed.");

					return;
				}

				m_requiredResources.clear();

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

				m_httpClient->DoFileGetRequest(hostname, port, path, m_currentDownload->targetFilename, [&] (bool result, std::string connData)
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
				TheResources.Reset();

				//std::string resourcePath = "citizen:/resources/";
				//TheResources.ScanResources(fiDevice::GetDevice("citizen:/setup2.xml", true), resourcePath);

				std::list<std::shared_ptr<Resource>> loadedResources;

				// mount any RPF files that we include
				for (auto& resource : m_requiredResources)
				{
					for (auto& file : resource.GetFiles())
					{
						if (file.filename.find(".rpf") != std::string::npos)
						{
							// get the path of the RPF
							std::string markedFile = TheResources.GetCache()->GetMarkedFilenameFor(resource.GetName(), file.filename);

							rage::fiPackfile* packFile = new rage::fiPackfile();
							packFile->openArchive(markedFile.c_str(), true, false, 0);
							packFile->mount(va("resources:/%s/", resource.GetName().c_str()));
						}
					}

					// load the resource
					auto resourceLoad = TheResources.AddResource(resource.GetName(), va("resources:/%s/", resource.GetName().c_str()));

					if (resourceLoad.get())
					{
						loadedResources.push_back(resourceLoad);
					}
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

			g_netLibrary->DownloadsComplete();

			while (!g_netLibrary->ProcessPreGameTick())
			{
				Sleep(1);
			}
			
			return true;
	}

	return false;
}

void DownloadManager::SetServer(NetAddress& address)
{
	m_gameServer = address;
}

DownloadManager TheDownloads;