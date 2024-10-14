#pragma once
#include "PacketHandler.h"
#include "Frame.h"
#include "ICoreGameInit.h"

namespace fx
{
class FramePacketHandler : public net::PacketHandler<net::packet::ServerFrame, HashRageString("msgFrame")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](const net::packet::ServerFrame& serverFrame)
		{
			ICoreGameInit* icgi = Instance<ICoreGameInit>::Get();
			const uint8_t strictLockdown = serverFrame.lockdownMode;
			const uint8_t syncStyle = serverFrame.syncStyle;

			static uint8_t lastStrictLockdown;

			if (strictLockdown != lastStrictLockdown)
			{
				if (!strictLockdown)
				{
					icgi->ClearVariable("strict_entity_lockdown");
				}
				else
				{
					icgi->SetVariable("strict_entity_lockdown");
				}

				lastStrictLockdown = strictLockdown;
			}

			icgi->SyncIsARQ = syncStyle == 1;
		});
	}
};
}
