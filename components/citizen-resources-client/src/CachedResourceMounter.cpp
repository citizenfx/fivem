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

#include <HttpClient.h>
#include <VFSRagePackfile.h>

#include <IteratorView.h>

#include <network/uri.hpp>

#include <ppl.h>

#include <Error.h>

using fx::CachedResourceMounter;

void MountResourceCacheDevice(std::shared_ptr<ResourceCache> cache);

static auto g_limitedScheduler = concurrency::Scheduler::Create(
	concurrency::SchedulerPolicy(2, 
		concurrency::MinConcurrency, 1, 
		concurrency::MaxConcurrency, 4));

static struct : public concurrency::scheduler_interface
{
	virtual void schedule(concurrency::TaskProc_t proc, void* arg) override
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
	m_resourceCache = std::make_shared<ResourceCache>(cachePath);

	MountResourceCacheDevice(m_resourceCache);
}

bool CachedResourceMounter::HandlesScheme(const std::string& scheme)
{
	return (scheme == "global");
}

fwRefContainer<fx::Resource> CachedResourceMounter::InitializeLoad(const std::string& uri, network::uri* parsedUri)
{
	// parse the input URI
	std::error_code ec;
	auto uriParsed = network::make_uri(uri, ec);

	if (parsedUri)
	{
		*parsedUri = uriParsed;
	}

	if (!ec)
	{
		// get the host name
		auto hostRef = uriParsed.host();

		if (hostRef)
		{
			// convert to a regular string
			std::string host = hostRef->to_string();

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
					entryList->AddEntry(ResourceCacheEntryList::Entry{ entry.first, entry.second.basename, entry.second.remoteUrl, entry.second.referenceHash, entry.second.size });
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

concurrency::task<fwRefContainer<fx::Resource>> CachedResourceMounter::LoadResource(const std::string& uri)
{
	fwRefContainer<fx::Resource> resource = InitializeLoad(uri, nullptr);

	if (resource.GetRef())
	{
		auto entryList = resource->GetComponent<ResourceCacheEntryList>();

		// verify if we even had an entry called 'resource.rpf'
		if (entryList->GetEntry("resource.rpf"))
		{
			/*if ((entryList->GetEntry("resource.rpf")->referenceHash == "D5541B91516FE7C823B99E261BD8C64D04B24F83" && resource->GetName().find("_new") != std::string::npos))
			{
				FatalError("This server has been blacklisted for violating the FiveM Terms of Service. If you manage this server and feel this is not justified, please contact your Technical Account Manager.");
			}*/

			// follow up by mounting resource.rpf (using the legacy mounter) from the resource on a background thread
			return concurrency::create_task([=]()
			{
				// copy the pointer in case we need to nullptr it
				fwRefContainer<fx::Resource> localResource = resource;

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
			}, concurrency::task_options(g_schedulerWrap));
		}
	}

	return concurrency::task<fwRefContainer<fx::Resource>>();
}

void CachedResourceMounter::AddResourceEntry(const std::string& resourceName, const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl, size_t size)
{
	m_resourceEntries.insert({ resourceName, ResourceFileEntry{basename, referenceHash, remoteUrl, size} });
}

void CachedResourceMounter::RemoveResourceEntries(const std::string& resourceName)
{
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
}