#include "StdInc.h"
#include "ResourceScripting.h"
#include "ResourceManager.h"

//#include "NativeInvoke.h"
//#include "ScriptManager.h"
#include <queue>
#include <map>

LUA_FUNCTION(TriggerEvent)
{
	STACK_BASE;

	// get event name
	const char* eventName = luaL_checkstring(L, 1);

	// serialize arguments
	int nargs = lua_gettop(L);
	luaS_serializeArgs(L, 2, nargs - 1);

	size_t len;
	const char* jsonString = lua_tolstring(L, -1, &len);

	// call into resources
	TheResources.TriggerEvent(std::string(eventName), std::string(jsonString, len));

	// remove serialized string
	lua_pop(L, 1);

	STACK_CHECK;

	return 0;
}

#if 0
struct QueuedEvent
{
	std::string eventName;
	std::string dataString;
	uint32_t lastSendTime;
	int uniqId;
	NPID tId;

	QueuedEvent()
	{
		lastSendTime = 0;
		tId = 0;
		uniqId = 0;
	}
};

struct QueuedAck
{
	uint32_t ack;
	uint32_t seq;
};

static CRITICAL_SECTION g_eventSendCS;

struct EventCxnState
{
	std::set<uint32_t> ackedPackets;
	std::vector<QueuedAck> ackQueue;
	std::map<uint32_t, QueuedEvent> eventSendQueue;
	int reliableSeq;
	int inSeq;
	int outSeq;

	EventCxnState()
	{
		reliableSeq = 0;
		inSeq = 0;
		outSeq = 0;
	}
};

static struct
{
	std::hash_map<NPID, EventCxnState> connections;
} g_evs;

LUA_FUNCTION(TriggerRemoteEvent)
{
	STACK_BASE;

	// get event name
	const char* eventName = luaL_checkstring(L, 1);

	// get target client ids
	bool targetClients[32] = { 0 };

	int targetClient = luaL_checkinteger(L, 2);

	// serialize arguments
	int nargs = lua_gettop(L);
	luaS_serializeArgs(L, 3, nargs - 2);

	const char* jsonString = lua_tostring(L, -1);

	lua_pop(L, 1);

	// make event object
	QueuedEvent ev;
	ev.dataString = std::string(jsonString);
	ev.eventName = std::string(eventName);

	NPID selfNPID;
	NP_GetNPID(&selfNPID);

	if (targetClient == -1)
	{
		for (int i = 0; i < 32; i++)
		{
			if (NativeInvoke::Invoke<0x4E237943, int>(i))
			{
				auto info = GetPlayerInfo(i);
				auto npID = *(NPID*)(info->address.abOnline);

				if (npID == selfNPID)
				{
					TheResources.QueueEvent(ev.eventName, ev.dataString, selfNPID);
				}
				else
				{
					ev.uniqId = g_evs.connections[npID].reliableSeq++;

					g_evs.connections[npID].eventSendQueue[ev.uniqId] = ev;
				}
			}
		}
	}
	else
	{
		auto info = GetPlayerInfo(targetClient);
		auto npID = *(NPID*)(info->address.abOnline);
		
		if (npID == selfNPID)
		{
			TheResources.QueueEvent(ev.eventName, ev.dataString, selfNPID);
		}
		else
		{
			ev.uniqId = g_evs.connections[npID].reliableSeq++;

			g_evs.connections[npID].eventSendQueue[ev.uniqId] = ev;
		}
	}

	STACK_CHECK;

	// done
	return 0;
}

void ResetBackpack(NPID remoteID)
{
	auto& conn = g_evs.connections[remoteID];

	conn.ackedPackets.clear();
	conn.ackQueue.clear();
	conn.eventSendQueue.clear();
	conn.reliableSeq = 0;
	conn.inSeq = 0;
	conn.outSeq = 0;
}

void NetEventsBackpackOnRecv(NPID remoteID, char* backpack, uint32_t packLen)
{
	// get acknowledged packets
	auto& conn = g_evs.connections[remoteID];

	uint32_t inSeq = *(uint32_t*)(backpack);
	backpack += 4;
	packLen -= 4;

	uint32_t outSeq = *(uint32_t*)(backpack);
	backpack += 4;
	packLen -= 4;

	for (int i = conn.ackQueue.size() - 1; i >= 0; i--)
	{
		if (conn.ackQueue[i].seq <= outSeq)
		{
			conn.ackQueue.erase(conn.ackQueue.begin() + i);
		}
	}

	conn.inSeq = inSeq;

	uint32_t numAcks = *(uint32_t*)(backpack);
	backpack += 4;
	packLen -= 4;
	
	// remove acks from the list
	for (uint32_t i = 0; i < numAcks; i++)
	{
		uint32_t ackId = *(uint32_t*)(backpack);
		backpack += 4;
		packLen -= 4;

		conn.eventSendQueue.erase(ackId);
	}

	// read real events
	while (packLen > 0)
	{
		uint32_t evId = *(uint32_t*)(backpack);

		QueuedAck ack;
		ack.ack = evId;
		ack.seq = INT_MAX;

		conn.ackQueue.push_back(ack);

		backpack += 4;
		packLen -= 4;

		static char nameBuffer[65536];
		static char dataBuffer[65536];

		char* outP = nameBuffer;

		while (*backpack)
		{
			*outP = *backpack;

			outP++;
			packLen--;
			backpack++;
		}

		backpack++;
		packLen--;
		outP[0] = '\0';

		outP = dataBuffer;

		while (*backpack)
		{
			*outP = *backpack;

			outP++;
			packLen--;
			backpack++;
		}

		backpack++;
		packLen--;
		outP[0] = '\0';

		// if we are not already acked
		if (conn.ackedPackets.find(evId) == conn.ackedPackets.end())
		{
			conn.ackedPackets.insert(evId);

			TheResources.QueueEvent(std::string(nameBuffer), std::string(dataBuffer), remoteID);
		}
	}
}

int NetEventsBackpackOnSend(NPID remoteID, char* appendHere)
{
	int addedLength = 0;
	auto& conn = g_evs.connections[remoteID];

	// list acknowledged packets
	*(uint32_t*)(appendHere) = conn.outSeq;
	addedLength += 4;
	appendHere += 4;

	conn.outSeq++;

	*(uint32_t*)(appendHere) = conn.inSeq; // last packet received
	addedLength += 4;
	appendHere += 4;

	*(uint32_t*)(appendHere) = conn.ackQueue.size();
	addedLength += 4;
	appendHere += 4;

	for (auto& p : conn.ackQueue)
	{
		p.seq = conn.outSeq - 1;

		*(uint32_t*)(appendHere) = p.ack;
		addedLength += 4;
		appendHere += 4;
	}

	for (auto& evQ : conn.eventSendQueue)
	{
		auto& ev = evQ.second;

		if (timeGetTime() < (ev.lastSendTime + 100))
		{
			continue;
		}

		int len = ev.eventName.length();

		ev.lastSendTime = timeGetTime();
		
		*(uint32_t*)(appendHere) = ev.uniqId;
		addedLength += 4;
		appendHere += 4;

		memcpy(appendHere, ev.eventName.c_str(), len + 1);
		addedLength += len + 1;
		appendHere += len + 1;

		len = ev.dataString.length();

		memcpy(appendHere, ev.dataString.c_str(), len + 1);
		addedLength += len + 1;
		appendHere += len + 1;
	}

	return addedLength;
}

InitFunction netEventInit([] ()
{
	InitializeCriticalSection(&g_eventSendCS);
});
#endif