#pragma once

#include "ResourceManager.h"
#include "EventCore.h"

namespace fx
{
class TestResourceManager : public virtual fx::ResourceManager
{
public:
	pplx::task<fwRefContainer<fx::Resource>> AddResource(const std::string& uri) override
	{
		return {};
	}

	pplx::task<tl::expected<fwRefContainer<fx::Resource>, fx::ResourceManagerError>> AddResourceWithError(
	const std::string& uri) override
	{
		return {};
	}

	fwRefContainer<fx::ResourceMounter> GetMounterForUri(const std::string& uri) override
	{
		return {};
	}

	fwRefContainer<fx::Resource> GetResource(const std::string& identifier, bool withProvides) override
	{
		return {};
	}

	void ForAllResources(const std::function<void(const fwRefContainer<fx::Resource>&)>& function) override
	{
	}

	void ResetResources() override
	{
	}

	void AddMounter(fwRefContainer<fx::ResourceMounter> mounter) override
	{
	}

	void RemoveResource(fwRefContainer<fx::Resource> resource) override
	{
	}

	fwRefContainer<fx::Resource> CreateResource(const std::string& resourceName,
	const fwRefContainer<fx::ResourceMounter>& mounter) override
	{
		return {};
	}

	void Tick() override
	{
		// on tick is required for the event queue inside ResourceEventManagerComponent. The ResourceEventManagerComponent on tick is called when onTick from the resource manager is called.
		OnTick();
	}

	void MakeCurrent() override
	{
	}

private:
	std::string CallReferenceInternal(const std::string& functionReference, const std::string& argsSerialized) override
	{
		return {};
	}
};

class ResourceManagerInstance
{
public:
	static fx::TestResourceManager* Create();
};
}
