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

using fx::CachedResourceMounter;

void MountResourceCacheDevice(std::shared_ptr<ResourceCache> cache);

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

concurrency::task<fwRefContainer<fx::Resource>> CachedResourceMounter::LoadResource(const std::string& uri)
{
	// parse the input URI
	std::error_code ec;
	auto uriParsed = network::make_uri(uri, ec);

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
					entryList->AddEntry(ResourceCacheEntryList::Entry{ entry.first, entry.second.basename, entry.second.remoteUrl, entry.second.referenceHash });
				}

				// verify if we even had an entry called 'resource.rpf'
				if (entryList->GetEntry("resource.rpf"))
				{
					// follow up by mounting resource.rpf (using the legacy mounter) from the resource on a background thread
					return concurrency::create_task([=] ()
					{
						// copy the pointer in case we need to nullptr it
						fwRefContainer<fx::Resource> localResource = resource;

						// open the packfile
						fwRefContainer<vfs::RagePackfile> packfile = new vfs::RagePackfile();
						if (packfile->OpenArchive(va("cache:/%s/resource.rpf", host.c_str())))
						{
							// and mount it
							std::string resourceRoot = "resources:/" + host + "/";
							vfs::Mount(packfile, resourceRoot);

							// if that went well, we should be able to _open_ the resource now
							if (!localResource->LoadFrom(resourceRoot))
							{
								localResource = nullptr;
							}
						}
						else
						{
							localResource = nullptr;
						}

						return localResource;
					});
				}
			}
		}
	}

	return concurrency::task<fwRefContainer<fx::Resource>>();
}

void CachedResourceMounter::AddResourceEntry(const std::string& resourceName, const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl)
{
	m_resourceEntries.insert({ resourceName, ResourceFileEntry{basename, referenceHash, remoteUrl} });
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
}