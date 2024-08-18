/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ResourceManagerImpl.h>

#include <ResourceMetaDataComponent.h>

#include <skyr/url.hpp>

static fx::ResourceManager* g_globalManager;
static thread_local fx::ResourceManager* g_currentManager;

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

	auto parsed = skyr::make_url(uri);

	if (parsed)
	{
		std::unique_lock<std::recursive_mutex> lock(m_mountersMutex);

		for (auto& mounterEntry : m_mounters)
		{
			if (mounterEntry->HandlesScheme(parsed->protocol().substr(0, parsed->protocol().length() - 1)))
			{
				mounter = mounterEntry;
				break;
			}
		}
	}
	else
	{
		trace("%s: %s\n", __func__, skyr::make_error_code(parsed.error()).message());
	}

	return mounter;
}

pplx::task<fwRefContainer<Resource>> ResourceManagerImpl::AddResource(const std::string& uri)
{
	return AddResourceWithError(uri)
		.then([](tl::expected<fwRefContainer<Resource>, ResourceManagerError> result)
	{
		return result.value_or(nullptr);
	});
}

pplx::task<tl::expected<fwRefContainer<Resource>, ResourceManagerError>> ResourceManagerImpl::AddResourceWithError(const std::string& uri)
{
	// find a valid mounter for this scheme
	auto mounter = GetMounterForUri(uri);

	// and forward to the mounter, if any.
	if (mounter.GetRef())
	{
		pplx::task_completion_event<tl::expected<fwRefContainer<Resource>, ResourceManagerError>> completionEvent;

		// set a completion event, as well
		mounter->LoadResourceWithError(uri).then([=] (tl::expected<fwRefContainer<Resource>, ResourceManagerError> resourceRef)
		{
			if (resourceRef)
			{
				std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);
				auto resource = resourceRef.value();

				// handle provides
				auto md = resource->GetComponent<ResourceMetaDataComponent>();

				for (const auto& entry : md->GetEntries("provide"))
				{
					m_resourceProvides.emplace(entry.second, resource);
				}
			}

			completionEvent.set(resourceRef);
		});

		return pplx::task<tl::expected<fwRefContainer<Resource>, ResourceManagerError>>(completionEvent);
	}

	return pplx::task_from_result<tl::expected<fwRefContainer<Resource>, ResourceManagerError>>(tl::make_unexpected(ResourceManagerError{ "No mounter for resource URI." }));
}

void ResourceManagerImpl::AddResourceInternal(fwRefContainer<Resource> resource)
{
	{
		std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

		m_resources[resource->GetName()] = fwRefContainer<ResourceImpl>(resource);
	}
}

fwRefContainer<Resource> ResourceManagerImpl::GetResource(const std::string& identifier, bool withProvides /* = true */)
{
	fwRefContainer<Resource> resource;
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	auto it = m_resources.find(identifier);
	resource = (it == m_resources.end()) ? nullptr : it->second;

	// if non-existent, or stopped
	if (withProvides && (!resource.GetRef() || resource->GetState() == ResourceState::Stopped))
	{
		auto provides = m_resourceProvides.equal_range(identifier);

		for (auto it = provides.first; it != provides.second; it++)
		{
			// if the provides is started (and the identifier is stopped), or if the identifier does not exist
			if (it->second->GetState() == ResourceState::Started || !resource.GetRef())
			{
				resource = it->second;
				break;
			}
		}
	}

	return resource;
}

void ResourceManagerImpl::ForAllResources(const std::function<void(const fwRefContainer<Resource>&)>& function)
{
	static std::vector<fwRefContainer<Resource>> currentResources;

	// collect resources so the mutex doesn't have to last for the callback duration
	// leading to potential deadlocks
	size_t resCount = 0;

	{
		std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

		if (currentResources.size() < m_resources.size())
		{
			currentResources.resize(m_resources.size());
		}

		for (const auto& resource : m_resources)
		{
			// avoid having to do a ref count increment/decrement (lock add/sub) at the cost
			// of a compare-branch (which might be predicted away)
			if (currentResources[resCount].GetRef() != resource.second.GetRef())
			{
				currentResources[resCount] = resource.second;
			}

			++resCount;
		}
	}

	auto lastManager = g_currentManager;
	g_currentManager = this;

	for (size_t idx = 0; idx < resCount; idx++)
	{
		function(currentResources[idx]);
	}

	g_currentManager = lastManager;
}

void ResourceManagerImpl::ResetResources()
{
	// lock for the entire ResetResources call
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	auto lastManager = g_currentManager;
	g_currentManager = this;

	ForAllResources([] (fwRefContainer<Resource> resource)
	{
		fwRefContainer<ResourceImpl> impl = resource;

		impl->Stop();
		impl->Destroy();
	});

	m_resourceProvides.clear();
	m_resources.clear();

	OnAfterReset();

	g_currentManager = lastManager;
}

void ResourceManagerImpl::RemoveResource(fwRefContainer<Resource> resource)
{
	auto lastManager = g_currentManager;
	g_currentManager = this;

	// lock the mutex (to provide a common root for a lock hierarchy)
	std::unique_lock<std::recursive_mutex> lock(m_resourcesMutex);

	fwRefContainer<ResourceImpl> impl = resource;
	impl->Stop();
	impl->Destroy();

	m_resources.erase(impl->GetName());

	g_currentManager = lastManager;
}

void ResourceManagerImpl::AddMounter(fwRefContainer<ResourceMounter> mounter)
{
	std::unique_lock<std::recursive_mutex> lock(m_mountersMutex);
	m_mounters.push_back(mounter);
}

fwRefContainer<Resource> ResourceManagerImpl::CreateResource(const std::string& resourceName, const fwRefContainer<ResourceMounter>& mounter)
{
	fwRefContainer<ResourceImpl> resource = new ResourceImpl(resourceName, this);

	if (mounter.GetRef())
	{
		resource->SetComponent(mounter);
	}

	AddResourceInternal(resource);

	return resource;
}

void ResourceManagerImpl::Tick()
{
	auto lastManager = g_currentManager;
	g_currentManager = this;

	// execute tick events
	OnTick();

	g_currentManager = lastManager;
}

ResourceManager* ResourceManager::GetCurrent(bool allowFallback /* = true */)
{
	if (!g_currentManager)
	{
		if (!allowFallback)
		{
			throw std::runtime_error("No current resource manager.");
		}

		return g_globalManager;
	}

	return g_currentManager;
}

void ResourceManagerImpl::MakeCurrent()
{
	g_currentManager = this;
	g_globalManager = this;
}

static std::function<std::string(const std::string&, const std::string&)> g_callRefCallback;

std::string ResourceManagerImpl::CallReferenceInternal(const std::string& functionReference, const std::string& argsSerialized)
{
	if (g_callRefCallback)
	{
		MakeCurrent();
		return g_callRefCallback(functionReference, argsSerialized);
	}

	return std::string();
}

void ResourceManager::SetCallRefCallback(const std::function<std::string(const std::string &, const std::string &)>& refCallback)
{
	g_callRefCallback = refCallback;
}

ResourceManager* CreateResourceManager()
{
	return new ResourceManagerImpl();
}

fwEvent<ResourceManager*> ResourceManager::OnInitializeInstance;
}
