#include "StdInc.h"
#include "ResourceManager.h"

std::shared_ptr<Resource> ResourceManager::GetResource(std::string& name)
{
	auto it = m_resources.find(name);

	if (it == m_resources.end())
	{
		return nullptr;
	}

	return it->second;
}

void ResourceManager::Tick()
{
	for (auto& resource : m_resources)
	{
		resource.second->Tick();
	}

	//for (auto& qEvent : m_eventQueue)
	for (auto it = m_eventQueue.begin(); it < m_eventQueue.end(); it++) // as the queue's end may change during this iteration
	{
		auto& qEvent = *it;

		TriggerEvent(qEvent.eventName, qEvent.argsSerialized, qEvent.source);
	}

	m_eventQueue.clear();
}

void ResourceManager::TriggerEvent(std::string& eventName, std::string& argsSerialized, int source)
{
	for (auto& resource : m_resources)
	{
		resource.second->TriggerEvent(eventName, argsSerialized, source);
	}
}

void ResourceManager::QueueEvent(std::string& eventName, std::string& argsSerialized, uint64_t source)
{
	QueuedScriptEvent ev;
	ev.eventName = eventName;
	ev.argsSerialized = argsSerialized;
	ev.source = (int)source;

	m_eventQueue.push_back(ev);
}

std::shared_ptr<Resource> ResourceManager::AddResource(std::string name, std::string path)
{
	auto resource = std::make_shared<Resource>(name, path);

	if (resource->Parse())
	{
		m_resources[resource->GetName()] = resource;

		return resource;
	}

	return nullptr;
}

void ResourceManager::DeleteResource(std::shared_ptr<Resource> resource)
{
	m_resources.erase(resource->GetName());
}

void ResourceManager::ScanResources(fiDevice* device, std::string& path)
{
	rage::fiFindData findData;
	int handle = device->findFirst(path.c_str(), &findData);

	if (!handle || handle == -1)
	{
		return;
	}

	do 
	{
		// dotfiles we don't want
		if (findData.fileName[0] == '.')
		{
			continue;
		}

		std::string fullPath = path;

		if (path[path.length() - 1] != '/')
		{
			fullPath.append("/");
		}

		fullPath.append(findData.fileName);

		// is this a directory?
		if (!(device->getFileAttributes(fullPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY))
		{
			continue;
		}

		// as this is a directory, is this a resource subdirectory?
		if (findData.fileName[0] == '[')
		{
			char* endBracket = strrchr(findData.fileName, ']');

			if (!endBracket)
			{
				// invalid name
				trace("ignored %s - no end bracket\n", findData.fileName);
				continue;
			}

			// traverse the directory
			ScanResources(device, fullPath);
		}
		else
		{
			// probably a resource directory...
			AddResource(std::string(findData.fileName), fullPath);
		}
	} while (device->findNext(handle, &findData));

	device->findClose(handle);
}

void ResourceManager::ScanResources(std::vector<std::pair<fiDevice*, std::string>>& paths)
{
	for (auto& path : paths)
	{
		ScanResources(path.first, path.second);
	}
}

void ResourceManager::Reset()
{
	m_stateNumber++;

	m_resources.clear();
}

ResourceCache* ResourceManager::GetCache()
{
	if (!m_resourceCache)
	{
		m_resourceCache = new ResourceCache();
		m_resourceCache->Initialize();
	}

	return m_resourceCache;
}

ResourceManager TheResources;