/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ResourceImpl.h>
#include <ResourceManager.h>

#include <mutex>

namespace fx
{
class ResourceManagerImpl : public ResourceManager
{
private:
	std::recursive_mutex m_resourcesMutex;

	std::unordered_map<std::string, fwRefContainer<ResourceImpl>> m_resources;

	std::recursive_mutex m_mountersMutex;

	std::vector<fwRefContainer<ResourceMounter>> m_mounters;

public:
	ResourceManagerImpl();

	void AddResourceInternal(fwRefContainer<Resource> resource);

	virtual fwRefContainer<ResourceMounter> GetMounterForUri(const std::string& uri) override;

	virtual concurrency::task<fwRefContainer<Resource>> AddResource(const std::string& uri) override;

	virtual fwRefContainer<Resource> GetResource(const std::string& identifier) override;

	virtual void ForAllResources(const std::function<void(fwRefContainer<Resource>)>& function) override;

	virtual void ResetResources() override;

	virtual void AddMounter(fwRefContainer<ResourceMounter> mounter) override;

	virtual void RemoveResource(fwRefContainer<Resource> resource) override;

	virtual fwRefContainer<Resource> CreateResource(const std::string& resourceName) override;

	virtual void Tick() override;
};
}