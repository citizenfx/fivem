#pragma once
#include "NetLibrary.h"
#include "Resource.h"

class COMPONENT_EXPORT(CITIZEN_LEGACY_NET_RESOURCES) NetLibraryResourcesComponent : public fwRefCountable, public fx::IAttached<NetLibrary>
{
public:
	virtual void AttachToObject(NetLibrary* netLibrary) override;

	void UpdateOneResource();

	void AddResourceToUpdateQueue(const std::string& resourceName)
	{
		m_resourceUpdateQueue.push(resourceName);
	}

	bool RequestResourceFileSet(fx::Resource* resource, const std::string& setName);
	bool ReleaseResourceFileSet(fx::Resource* resource, const std::string& setName);

private:
	void UpdateResources(const std::string& updateList, const std::function<void()>& doneCb);

private:
	std::queue<std::string> m_resourceUpdateQueue;

	NetLibrary* m_netLibrary;
};

DECLARE_INSTANCE_TYPE(NetLibraryResourcesComponent);
