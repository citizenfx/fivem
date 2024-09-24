#pragma once
#include "ICoreGameInit.h"
#include "netGameEvent.h"
#include "PacketHandler.h"
#include "NetGameEventPacket.h"

namespace fx
{
class NetGameEventPacketHandler : public net::PacketHandler<net::packet::ServerNetGameEvent, HashRageString("msgNetGameEvent")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerNetGameEvent& serverNetGameEvent)
		{
			ICoreGameInit* icgi = Instance<ICoreGameInit>::Get();
			if (!icgi->OneSyncEnabled)
			{
				return;
			}

			rage::HandleNetGameEvent(reinterpret_cast<const char*>(serverNetGameEvent.data.GetValue().data()), serverNetGameEvent.data.GetValue().size());
		});
	}
};
}
