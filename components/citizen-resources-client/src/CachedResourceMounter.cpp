/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <CachedResourceMounter.h>
#include <ResourceCache.h>

#include <HttpClient.h>

void MountResourceCacheDevice(std::shared_ptr<ResourceCache> cache);

class CachedResourceMounter : public fx::ResourceMounter
{
private:
	std::shared_ptr<ResourceCache> m_resourceCache;

public:
	CachedResourceMounter(const std::string& cachePath);

	virtual bool HandlesScheme(const std::string& scheme) override;

	virtual concurrency::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override;
};

CachedResourceMounter::CachedResourceMounter(const std::string& cachePath)
{
	m_resourceCache = std::make_shared<ResourceCache>(cachePath);

	MountResourceCacheDevice(m_resourceCache);
}

bool CachedResourceMounter::HandlesScheme(const std::string& scheme)
{
	return false;
}

concurrency::task<fwRefContainer<fx::Resource>> CachedResourceMounter::LoadResource(const std::string& uri)
{
	return concurrency::task<fwRefContainer<fx::Resource>>();
}

namespace fx
{
	fwRefContainer<ResourceMounter> GetCachedResourceMounter(const std::string& cachePath)
	{
		return new CachedResourceMounter(cachePath);
	}
}