#pragma once
#include "CloneManager.h"
#include "PackedClonesPacket.h"
#include "PacketHandler.h"

extern sync::CloneManager* TheClones;

namespace fx
{
class PackedClonesPacketHandler : public net::PacketHandler<net::packet::ServerPackedClones, HashRageString("msgPackedClones")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerPackedClones& serverPackedClones)
		{
			TheClones->HandleCloneSync(reinterpret_cast<const char*>(serverPackedClones.data.GetValue().data()), serverPackedClones.data.GetValue().size());
		});
	}
};
}
