#pragma once
#include "ICoreGameInit.h"
#include "netGameEvent.h"
#include "PacketHandler.h"
#include "NetGameEventPacket.h"

namespace fx
{
class NetGameEventV2PacketHandler : public net::PacketHandler<net::packet::ServerNetGameEventV2, HashRageString("msgNetGameEventV2")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerNetGameEventV2& serverNetGameEventV2)
		{
			ICoreGameInit* icgi = Instance<ICoreGameInit>::Get();
			if (!icgi->OneSyncEnabled)
			{
				return;
			}

			rage::HandleNetGameEventV2(serverNetGameEventV2);
		});
	}
};
}
