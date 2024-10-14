#pragma once
#include "PacketHandler.h"
#include "ResourceManager.h"
#include "ResourcePacket.h"

namespace fx
{
class ResourceStopPacketHandler : public net::PacketHandler<net::packet::ServerResourceStop, HashRageString("msgResStop")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerResourceStop& serverResourceStop)
		{
			std::string resourceName(serverResourceStop.resourceName.GetValue());

			fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
			resourceManager->MakeCurrent();

			auto resource = resourceManager->GetResource(resourceName);

			if (resource.GetRef() == nullptr)
			{
				trace("Server requested resource %s to be stopped, but we don't know that resource\n", resourceName);
				return;
			}

#if 0
				if (resource->GetState() != ResourceStateRunning)
				{
					trace("Server requested resource %s to be stopped, but it's not running\n", resourceName.c_str());
					return;
				}
#endif

			resource->Stop();
		});
	}
};
}
