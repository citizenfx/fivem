#pragma once
#include "NetLibrary.h"
#include "Resource.h"

class NetLibraryResourcesComponent : public fwRefCountable, public fx::IAttached<NetLibrary>
{
public:
	virtual void AttachToObject(NetLibrary* netLibrary) override;

	void UpdateOneResource();

	void AddResourceToUpdateQueue(const std::string& resourceName)
	{
		m_resourceUpdateQueue.push(resourceName);
	}

private:
	void UpdateResources(const std::string& updateList, const std::function<void()>& doneCb);

	bool RequestResourceFileSet(fx::Resource* resource, const std::string& setName);

private:
	std::queue<std::string> m_resourceUpdateQueue;

	NetLibrary* m_netLibrary;
};

DECLARE_INSTANCE_TYPE(NetLibraryResourcesComponent);
