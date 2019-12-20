/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <CachedResourceMounter.h>
#include <ResourceCache.h>
#include <ResourceCacheDevice.h>

#include <ResourceManager.h>

#include <StreamingEvents.h>

#include <HttpClient.h>
#include <VFSRagePackfile.h>

#include <IteratorView.h>

#include <skyr/url.hpp>

#include <ppl.h>

#include <Error.h>

using fx::CachedResourceMounter;

std::unordered_multimap<std::string, std::pair<std::string, std::string>> g_referenceHashList;
static std::mutex g_referenceHashListMutex;

static std::map<std::string, std::function<void(int, int)>> g_statusCallbacks;
static std::mutex g_statusCallbacksMutex;

void MountResourceCacheDevice(std::shared_ptr<ResourceCache> cache);

static auto g_limitedScheduler = concurrency::Scheduler::Create(
	concurrency::SchedulerPolicy(2, 
		concurrency::MinConcurrency, 1, 
		concurrency::MaxConcurrency, 4));

static struct : public pplx::scheduler_interface
{
	virtual void schedule(pplx::TaskProc_t proc, void* arg) override
	{
		g_limitedScheduler->ScheduleTask(proc, arg);
	}
} g_schedulerWrap;

CachedResourceMounter::CachedResourceMounter(fx::ResourceManager* manager)
	: m_manager(manager)
{

}

CachedResourceMounter::CachedResourceMounter(fx::ResourceManager* manager, const std::string& cachePath)
	: m_manager(manager)
{
	m_resourceCache = std::make_shared<ResourceCache>(cachePath, ToNarrow(MakeRelativeCitPath(L"cache/")));

	MountResourceCacheDevice(m_resourceCache);
}

bool CachedResourceMounter::HandlesScheme(const std::string& scheme)
{
	return (scheme == "global");
}

fwRefContainer<fx::Resource> CachedResourceMounter::InitializeLoad(const std::string& uri, skyr::url* parsedUri)
{
	// parse the input URI
	auto uriParsed = skyr::make_url(uri);

	if (uriParsed)
	{
		if (parsedUri)
		{
			*parsedUri = *uriParsed;
		}

		// get the host name
		auto host = uriParsed->host();

		if (!host.empty())
		{
			// find a resource entry in the entry list
			if (m_resourceEntries.find(host) != m_resourceEntries.end())
			{
				// if there is one, start by creating a resource with a list component
				fwRefContainer<fx::Resource> resource = m_manager->CreateResource(host);

				fwRefContainer<ResourceCacheEntryList> entryList = new ResourceCacheEntryList();
				resource->SetComponent(entryList);

				// and add the entries from the list to the resource
				for (auto& entry : GetIteratorView(m_resourceEntries.equal_range(host)))
				{
					entryList->AddEntry(ResourceCacheEntryList::Entry{ entry.first, entry.second.basename, entry.second.remoteUrl, entry.second.referenceHash, entry.second.size, entry.second.extData });
				}

				return resource;
			}
		}
	}

	return nullptr;
}

fwRefContainer<vfs::Device> CachedResourceMounter::OpenResourcePackfile(const fwRefContainer<fx::Resource>& resource)
{
	fwRefContainer<vfs::RagePackfile> packfile = new vfs::RagePackfile();

	if (packfile->OpenArchive(fmt::sprintf("cache:/%s/resource.rpf", resource->GetName())))
	{
		return packfile;
	}

	return nullptr;
}

void CachedResourceMounter::AddStatusCallback(const std::string& resourceName, const std::function<void(int, int)>& callback)
{
	auto path = FormatPath(resourceName, "resource.rpf");

	std::unique_lock<std::mutex> lock(g_statusCallbacksMutex);
	g_statusCallbacks[path] = callback;
}

pplx::task<fwRefContainer<fx::Resource>> CachedResourceMounter::LoadResource(const std::string& uri)
{
	fwRefContainer<fx::Resource> resource = InitializeLoad(uri, nullptr);

	if (resource.GetRef())
	{
		auto entryList = resource->GetComponent<ResourceCacheEntryList>();

		// verify if we even had an entry called 'resource.rpf'
		if (entryList->GetEntry("resource.rpf"))
		{
			// follow up by mounting resource.rpf (using the legacy mounter) from the resource on a background thread
			return pplx::create_task([=]()
			{
				m_manager->MakeCurrent();

				// copy the pointer in case we need to nullptr it
				fwRefContainer<fx::Resource> localResource = resource;

				if (!m_manager->GetResource(resource->GetName()).GetRef())
				{
					trace("Our resource - %s - disappeared right from under our nose?\n", resource->GetName());

					localResource = nullptr;

					return localResource;
				}

				// open the packfile
				fwRefContainer<vfs::Device> packfile = OpenResourcePackfile(resource);
				
				if (packfile.GetRef())
				{
					// and mount it
					std::string resourceRoot = "resources:/" + resource->GetName() + "/";
					vfs::Mount(packfile, resourceRoot);

					// if that went well, we should be able to _open_ the resource now
					if (!localResource->LoadFrom(resourceRoot))
					{
						GlobalError("Couldn't load resource %s from %s.", resource->GetName(), resourceRoot);

						localResource = nullptr;
					}
					else
					{
						localResource->OnRemove.Connect([=]()
						{
							vfs::Unmount(resourceRoot);
						});
					}
				}
				else
				{
					GlobalError("Couldn't load resource packfile for %s.", resource->GetName());

					localResource = nullptr;
				}

				return localResource;
			}, pplx::task_options(g_schedulerWrap));
		}
	}

	return pplx::task_from_result(fwRefContainer<fx::Resource>());
}

void CachedResourceMounter::AddResourceEntry(const std::string& resourceName, const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl, size_t size, const std::map<std::string, std::string>& extData)
{
	{
		std::unique_lock<std::mutex> g_referenceHashListMutex;
		g_referenceHashList.insert({ referenceHash, {resourceName, basename} });
	}

	m_resourceEntries.insert({ resourceName, ResourceFileEntry{basename, referenceHash, remoteUrl, size, extData} });
}

void CachedResourceMounter::RemoveResourceEntries(const std::string& resourceName)
{
	{
		std::unique_lock<std::mutex> g_referenceHashListMutex;

		auto bits = m_resourceEntries.equal_range(resourceName);

		for (const auto& entry : fx::GetIteratorView(bits))
		{
			g_referenceHashList.erase(entry.second.referenceHash);
		}
	}

	m_resourceEntries.erase(resourceName);
}

std::string CachedResourceMounter::FormatPath(const std::string& resourceName, const std::string& basename)
{
	return fmt::sprintf("cache:/%s/%s", resourceName, basename);
}

namespace fx
{
	fwRefContainer<CachedResourceMounter> GetCachedResourceMounter(ResourceManager* manager, const std::string& cachePath)
	{
		static std::map<std::string, fwRefContainer<CachedResourceMounter>> mounters;

		auto it = mounters.find(cachePath);

		if (it == mounters.end())
		{
			it = mounters.insert({ cachePath, new CachedResourceMounter(manager, cachePath) }).first;
		}

		return it->second;
	}

	fwEvent<const StreamingEntryData&> OnAddStreamingResource;

	fwEvent<> OnLockStreaming;
	fwEvent<> OnUnlockStreaming;
}

static InitFunction initFunction([]()
{
	fx::OnCacheDownloadStatus.Connect([](const std::string& fileName, size_t done, size_t total)
	{
		std::unique_lock<std::mutex> lock(g_statusCallbacksMutex);
		auto it = g_statusCallbacks.find(fileName);

		if (it != g_statusCallbacks.end())
		{
			lock.unlock();

			it->second(done, total);

			if (done >= total)
			{
				lock.lock();
				g_statusCallbacks.erase(fileName);
			}
		}
	});
});
