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

}

concurrency::task<fwRefContainer<Resource>> ResourceManagerImpl::AddResource(const std::string& uri)
{
	// parse the URI
	std::error_code ec;
	network::uri parsed = network::make_uri(uri, ec);
	
	if (!static_cast<bool>(ec))
	{
		// find a valid mounter for this scheme
		fwRefContainer<ResourceMounter> mounter;

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
	}

	return concurrency::task_from_result<fwRefContainer<Resource>>(nullptr);
}

void ResourceManagerImpl::AddResourceInternal(fwRefContainer<Resource> resource)
{
	{
		std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

		m_resources.insert({ resource->GetName(), resource });
	}
}

fwRefContainer<Resource> ResourceManagerImpl::GetResource(const std::string& identifier)
{
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	return m_resources[identifier];
}

void ResourceManagerImpl::ForAllResources(const std::function<void(fwRefContainer<Resource>)>& function)
{
	// copy resources locally
	decltype(m_resources) resources;

	{
		std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

		resources = m_resources;
	}

	for (auto& resource : resources)
	{
		function(resource.second);
	}
}

void ResourceManagerImpl::ResetResources()
{
	ForAllResources([] (fwRefContainer<Resource> resource)
	{
		resource->Stop();
	});

	m_resources.clear();
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

ResourceManager* CreateResourceManager()
{
	return new ResourceManagerImpl();
}
}
