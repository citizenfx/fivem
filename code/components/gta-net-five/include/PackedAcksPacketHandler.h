#pragma once
#include "CloneManager.h"
#include "PackedAcksPacket.h"
#include "PacketHandler.h"

extern sync::CloneManager* TheClones;

namespace fx
{
class PackedAcksPacketHandler : public net::PacketHandler<net::packet::ServerPackedAcks, HashRageString("msgPackedAcks")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerPackedAcks& serverPackedAcks)
		{
			TheClones->HandleCloneAcks(reinterpret_cast<const char*>(serverPackedAcks.data.GetValue().data()), serverPackedAcks.data.GetValue().size());
		});
	}
};
}
