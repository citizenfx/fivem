/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ResourceManagerImpl.h>

#include <network/uri.hpp>

namespace fx
{
ResourceManagerImpl::ResourceManagerImpl()
{
	OnInitializeInstance(this);
}

fwRefContainer<ResourceMounter> ResourceManagerImpl::GetMounterForUri(const std::string& uri)
{
	// parse the URI
	fwRefContainer<ResourceMounter> mounter;

	std::error_code ec;
	network::uri parsed = network::make_uri(uri, ec);

	if (!static_cast<bool>(ec))
	{
		std::unique_lock<std::recursive_mutex> lock(m_mountersMutex);

		for (auto& mounterEntry : m_mounters)
		{
			if (mounterEntry->HandlesScheme(parsed.scheme()->to_string()))
			{
				mounter = mounterEntry;
				break;
			}
		}
	}

	return mounter;
}

concurrency::task<fwRefContainer<Resource>> ResourceManagerImpl::AddResource(const std::string& uri)
{
	// find a valid mounter for this scheme
	auto mounter = GetMounterForUri(uri);

	// and forward to the mounter, if any.
	if (mounter.GetRef())
	{
		concurrency::task_completion_event<fwRefContainer<Resource>> completionEvent;

		// set a completion event, as well
		mounter->LoadResource(uri).then([=] (fwRefContainer<Resource> resource)
		{
			completionEvent.set(resource);
		});

		return concurrency::task<fwRefContainer<Resource>>(completionEvent);
	}

	return concurrency::task_from_result<fwRefContainer<Resource>>(nullptr);
}

void ResourceManagerImpl::AddResourceInternal(fwRefContainer<Resource> resource)
{
	{
		std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

		m_resources[resource->GetName()] = fwRefContainer<ResourceImpl>(resource);
	}
}

fwRefContainer<Resource> ResourceManagerImpl::GetResource(const std::string& identifier)
{
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	auto it = m_resources.find(identifier);

	return (it == m_resources.end()) ? nullptr : it->second;
}

void ResourceManagerImpl::ForAllResources(const std::function<void(fwRefContainer<Resource>)>& function)
{
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	for (auto& resource : m_resources)
	{
		function(resource.second);
	}
}

void ResourceManagerImpl::ResetResources()
{
	ForAllResources([] (fwRefContainer<Resource> resource)
	{
		fwRefContainer<ResourceImpl> impl = resource;

		impl->Stop();
		impl->Destroy();
	});

	m_resources.clear();
}

void ResourceManagerImpl::RemoveResource(fwRefContainer<Resource> resource)
{
	// lock the mutex (to provide a common root for a lock hierarchy)
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	fwRefContainer<ResourceImpl> impl = resource;
	impl->Stop();
	impl->Destroy();

	m_resources.erase(impl->GetName());
}

void ResourceManagerImpl::AddMounter(fwRefContainer<ResourceMounter> mounter)
{
	std::unique_lock<std::recursive_mutex> lock(m_mountersMutex);
	m_mounters.push_back(mounter);
}

fwRefContainer<Resource> ResourceManagerImpl::CreateResource(const std::string& resourceName)
{
	fwRefContainer<ResourceImpl> resource = new ResourceImpl(resourceName, this);

	AddResourceInternal(resource);

	return resource;
}

void ResourceManagerImpl::Tick()
{
	// execute resource tick functions
	ForAllResources([] (fwRefContainer<Resource> resource)
	{
		resource->Tick();
	});

	// execute tick events
	OnTick();
}

ResourceManager* CreateResourceManager()
{
	return new ResourceManagerImpl();
}

fwEvent<ResourceManager*> ResourceManager::OnInitializeInstance;
}
