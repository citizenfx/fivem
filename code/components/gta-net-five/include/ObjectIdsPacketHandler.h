#pragma once
#include "PacketHandler.h"
#include "ObjectIds.h"

extern bool g_requestedIds;
extern std::list<int> g_objectIds;
extern std::set<int> g_stolenObjectIds;

namespace fx
{
class ObjectIdsPacketHandler : public net::PacketHandler<net::packet::ServerObjectIds<false/*for client assume its largest possible packet, so none big mode*/>, HashRageString("msgObjectIds")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerObjectIds<false>& serverObjectIds)
		{
			serverObjectIds.ForEachId([](uint16_t objectId)
			{
				g_objectIds.push_back(objectId);
				g_stolenObjectIds.erase(objectId);
			});

			g_requestedIds = false;
		});
	}
};
}
