#pragma once
#include "NetLibraryResourcesComponent.h"
#include "PacketHandler.h"
#include "ResourceManager.h"
#include "ResourcePacket.h"

extern NetLibrary* g_netLibrary;
extern tbb::concurrent_queue<std::function<void()>> executeNextGameFrame;

extern std::mutex g_resourceStartRequestMutex;
extern std::set<std::string> g_resourceStartRequestSet;

namespace fx
{
class ResourceStartPacketHandler : public net::PacketHandler<net::packet::ServerResourceStart, HashRageString("msgResStart")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerResourceStart& serverResourceStart)
		{
			std::string resourceName(serverResourceStart.resourceName.GetValue());

			fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
			resourceManager->MakeCurrent();

			auto resource = resourceManager->GetResource(resourceName);

			auto netLibraryResourcesComponent = g_netLibrary->GetComponent<NetLibraryResourcesComponent>();

			std::lock_guard _(g_resourceStartRequestMutex);
			if (g_resourceStartRequestSet.find(resourceName) == g_resourceStartRequestSet.end())
			{
				g_resourceStartRequestSet.insert(resourceName);
				netLibraryResourcesComponent->AddResourceToUpdateQueue(resourceName);

				executeNextGameFrame.push([netLibraryResourcesComponent]()
				{
					netLibraryResourcesComponent->UpdateOneResource();
				});
			}
		});
	}
};
}
