#include <StdInc.h>

#include "NetGameEventPacket.h"

#include "ByteReader.h"
#include "ByteWriter.h"
#include "CrossBuildRuntime.h"
#include "Error.h"
#include "GameInit.h"
#include "Hooking.h"
#include "ICoreGameInit.h"
#include "MinHook.h"
#include "NetBitVersion.h"
#include "netBuffer.h"
#include "NetGameEventPacket.h"
#include "NetGameEvent.h"
#include "NetLibrary.h"
#include "netPlayerManager.h"
#include "Pool.h"
#include "rlNetBuffer.h"

#include <chrono>
#include <deque>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "NetGameEventPacketHandler.h"
#include "NetGameEventV2PacketHandler.h"

using namespace std::chrono_literals;

// TODO: event expiration?
struct netGameEventState
{
	rage::netGameEvent* ev;
	std::chrono::milliseconds time;
	bool sent;

	netGameEventState()
		: ev(nullptr), time(0), sent(false)
	{

	}

	netGameEventState(rage::netGameEvent* ev, std::chrono::milliseconds time)
		: ev(ev), time(time), sent(false)
	{

	}
};

struct ReEventQueueItem
{
	uint32_t eventNameHash;
	uint16_t clientNetId;
	uint16_t eventId;
	bool isReply;
	std::vector<uint8_t> eventData;

	ReEventQueueItem(net::packet::ServerNetGameEventV2& serverNetGameEvent):
		eventNameHash(serverNetGameEvent.eventNameHash),
		clientNetId(serverNetGameEvent.clientNetId),
		eventId(serverNetGameEvent.eventId),
		isReply(serverNetGameEvent.isReply),
		eventData(serverNetGameEvent.data.GetValue().begin(), serverNetGameEvent.data.GetValue().end())
	{

	}
};

extern NetLibrary* g_netLibrary;

static ICoreGameInit* icgi;

static CNetGamePlayer* g_player31;

// event stuff
static void* g_netEventMgr;

static uint16_t g_eventHeader;
static bool g_lastEventGotRejected;

static atPoolBase** g_netGameEventPool;

static std::map<std::tuple<uint16_t, uint16_t>, netGameEventState> g_events;
static std::map<std::tuple<uint32_t, uint16_t>, netGameEventState> g_eventsV2;

static std::deque<net::Buffer> g_reEventQueue;
static std::deque<ReEventQueueItem> g_reEventQueueV2;

static std::unordered_set<uint16_t> g_eventBlacklist;
static std::unordered_set<uint32_t> g_eventBlacklistV2;

#ifdef IS_RDR3
static std::unordered_map<uint16_t, const char*> g_eventNames;
#endif

static void (*g_origAddEvent)(void*, rage::netGameEvent*);
static void (*g_origExecuteNetGameEvent)(void* eventMgr, rage::netGameEvent* ev, rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t evH, uint32_t, uint32_t);
static void (*g_origUnkEventMgr)(void*, void*);
static bool (*g_origMetricCASHIsGamerHandleValid)(void*);
static bool(*g_origSendGameEvent)(void*, void*);

#if defined(GTA_FIVE)
static void (*g_origSendAlterWantedLevelEvent1)(void*, void*, void*, void*);
static void (*g_origSendAlterWantedLevelEvent2)(void*, void*, void*, void*);
static uint32_t (*g_origGetFireApplicability)(void* event, void* pos);
static bool (*g_origCPlayerTauntEventDecide)(void*, CNetGamePlayer*, void*);

static hook::cdecl_stub<const char*(void*, uint16_t)> _netEventMgr_GetNameFromType([]()
{
	return hook::get_pattern("66 83 FA ? 73 12 0F B7 D2 48 8B 8C D1 ? ? ? ? 48 85 C9", -7);
});

static hook::cdecl_stub<void*(CNetGamePlayer*)> CNetGamePlayer_GetPlayerPed([]()
{
	return hook::get_pattern("48 8B 91 ? ? ? ? 33 C0 48 85 D2 74 07 48 8B 82");
});
#elif defined(IS_RDR3)
static uint32_t* (*g_origGetFireApplicability)(void* event, uint32_t*, void* pos);
static void* (*g_origRegisterNetGameEvent)(void*, uint16_t, void*, const char*);
#endif

static hook::thiscall_stub<bool(void* eventMgr, bool fatal)> rage__netEventMgr__CheckForSpaceInPool([]()
{
#ifdef GTA_FIVE
	return hook::get_pattern("41 C1 E0 02 41 C1 F8 02 41 2B C0 0F 85", -0x2A);
#else // IS_RDR3
	return hook::get_pattern("33 DB 85 C0 0F 85 ? ? ? ? 40 84 FF 0F 84", -0x30);
#endif
});

#ifdef GTA_FIVE
static void SendAlterWantedLevelEvent1Hook(void* a1, void* a2, void* a3, void* a4)
{
	if (!rage__netEventMgr__CheckForSpaceInPool(g_netEventMgr, false))
	{
		return;
	}

	g_origSendAlterWantedLevelEvent1(a1, a2, a3, a4);
}

static void SendAlterWantedLevelEvent2Hook(void* a1, void* a2, void* a3, void* a4)
{
	if (!rage__netEventMgr__CheckForSpaceInPool(g_netEventMgr, false))
	{
		return;
	}

	g_origSendAlterWantedLevelEvent2(a1, a2, a3, a4);
}
#endif

// #TODO: Once file is reorganized dump into netEventMgr as utility functions
static uint16_t netEventMgr_GetMaxEventType()
{
#ifdef GTA_FIVE
	return (xbr::IsGameBuildOrGreater<2060>() ? 0x5B : 0x5A);
#elif IS_RDR3
	return 0xA5;
#endif
}

static void netEventMgr_PopulateEventBlacklist()
{
	std::unordered_map<std::string_view, uint16_t> eventIdents;
	auto BlacklistEvent = [&](const char* name)
	{
		if (auto it = eventIdents.find(name); it != eventIdents.end())
		{
			g_eventBlacklist.emplace(it->second);
		}
	};

#ifdef GTA_FIVE
	auto eventMgr = *(char**)g_netEventMgr;
	for (uint16_t i = 0; i < netEventMgr_GetMaxEventType(); ++i)
	{
		if (auto name = _netEventMgr_GetNameFromType(eventMgr, i))
		{
			eventIdents.insert({ name, i });
		}
	}
#elif IS_RDR3
	for (auto [type, name] : g_eventNames)
	{
		eventIdents.insert({ name, type });
	}
#endif

	BlacklistEvent("GIVE_CONTROL_EVENT"); // don't give control using events!
	BlacklistEvent("BLOW_UP_VEHICLE_EVENT"); // used only during migration
	BlacklistEvent("KICK_VOTES_EVENT");
	BlacklistEvent("NETWORK_CRC_HASH_CHECK_EVENT");
	BlacklistEvent("NETWORK_CHECK_EXE_SIZE_EVENT");
	BlacklistEvent("NETWORK_CHECK_CODE_CRCS_EVENT");
	BlacklistEvent("NETWORK_CHECK_CATALOG_CRC");
}

static std::unordered_map<uint32_t, uint16_t> netEventMgr_GetEventHashIdents()
{
	std::unordered_map<uint32_t, uint16_t> eventIdents;

#ifdef GTA_FIVE
	auto eventMgr = *(char**)g_netEventMgr;
	for (uint16_t i = 0; i < netEventMgr_GetMaxEventType(); ++i)
	{
		if (auto name = _netEventMgr_GetNameFromType(eventMgr, i))
		{
			eventIdents.insert({ HashRageString(name), i });
		}
	}
#elif defined(IS_RDR3)
	for (auto [type, name] : g_eventNames)
	{
		eventIdents.insert({  HashRageString(name), type });
	}
#endif

	return eventIdents;
}

static void netEventMgr_PopulateEventBlacklistV2()
{
	auto BlacklistEvent = [&](const char* name)
	{
		g_eventBlacklistV2.emplace(HashRageString(name));
	};

	BlacklistEvent("GIVE_CONTROL_EVENT"); // don't give control using events!
	BlacklistEvent("BLOW_UP_VEHICLE_EVENT"); // used only during migration
	BlacklistEvent("KICK_VOTES_EVENT");
	BlacklistEvent("NETWORK_CRC_HASH_CHECK_EVENT");
	BlacklistEvent("NETWORK_CHECK_EXE_SIZE_EVENT");
	BlacklistEvent("NETWORK_CHECK_CODE_CRCS_EVENT");
	BlacklistEvent("NETWORK_CHECK_CATALOG_CRC");
}

/// Ignore processing events that do not apply to OneSyncEnabled.
///
/// If the event implements a Reply method, or is exposed via script command,
/// ensure blacklisting it will not negatively impact the game or network state.
static bool netEventMgr_IsBlacklistedEvent(uint16_t type)
{
	static std::once_flag generated;
	std::call_once(generated, netEventMgr_PopulateEventBlacklist);

	return g_eventBlacklist.find(type) != g_eventBlacklist.end();
}

/// Ignore processing events that do not apply to OneSyncEnabled.
///
/// If the event implements a Reply method, or is exposed via script command,
/// ensure blacklisting it will not negatively impact the game or network state.
static bool netEventMgr_IsBlacklistedEventV2(uint16_t type)
{
	static std::once_flag generated;
	std::call_once(generated, netEventMgr_PopulateEventBlacklistV2);

	return g_eventBlacklistV2.find(type) != g_eventBlacklistV2.end();
}

/// TEMPORARY: Event ID overriding process. Should be used for RedM only for now
static uint16_t netEventMgr_MapEventId(uint16_t type, bool isSend)
{
#if IS_RDR3
	if (xbr::IsGameBuildOrGreater<1491>())
	{
		if (isSend && type >= 51)
		{
			return type + 1;
		}
		else if (!isSend && type >= 52)
		{
			return type - 1;
		}
	}
#endif

	return type;
}

static std::vector<uint32_t> netEventMgr_GetIdentsToEventHash()
{
	std::vector<uint32_t> eventIdents;

#ifdef GTA_FIVE
	auto eventMgr = *(char**)g_netEventMgr;

	eventIdents.resize(netEventMgr_GetMaxEventType());

	for (uint16_t i = 0; i < netEventMgr_GetMaxEventType(); ++i)
	{
		if (auto name = _netEventMgr_GetNameFromType(eventMgr, i))
		{
			eventIdents[i] = HashRageString(name);
		}
	}
#elif IS_RDR3
	for (auto [type, name] : g_eventNames)
	{
		eventIdents.resize(type + 1);
		eventIdents[type] =  HashRageString(name);
	}
#endif

	return eventIdents;
}

static bool EventNeedsOriginalPlayer(rage::netGameEvent* ev)
{
#ifdef GTA_FIVE
	auto nameHash = HashString(ev->GetName());

	// synced scenes depend on this to target the correct remote player
	if (nameHash == HashString("REQUEST_NETWORK_SYNCED_SCENE_EVENT") ||
		nameHash == HashString("START_NETWORK_SYNCED_SCENE_EVENT") ||
		nameHash == HashString("STOP_NETWORK_SYNCED_SCENE_EVENT") ||
		nameHash == HashString("UPDATE_NETWORK_SYNCED_SCENE_EVENT"))
	{
		return true;
	}
#endif

	return false;
}

static void SendGameEventRaw(uint16_t eventId, rage::netGameEvent* ev)
{
	// TODO: use a real player for some things
	rage::EnsurePlayer31();

	// allocate a RAGE buffer
	uint8_t packetStub[1024];
	rage::datBitBuffer rlBuffer(packetStub, sizeof(packetStub));

	ev->Prepare(&rlBuffer, g_player31, nullptr);

#ifdef GTA_FIVE
	ev->PrepareExtraData(&rlBuffer, false, g_player31, nullptr);
#elif IS_RDR3
	ev->PrepareExtraData(&rlBuffer, g_player31, nullptr);
#endif

	net::Buffer outBuffer;

	// TODO: replace with bit array?
	std::set<uint16_t> targetPlayers;

	for (auto& player : g_players)
	{
		if (
			player
#ifdef GTA_FIVE
			&& player->nonPhysicalPlayerData()
#endif
		)
		{
			// temporary pointer check
#ifdef GTA_FIVE
			if ((uintptr_t)player->nonPhysicalPlayerData() > 256)
#endif
			{
				// make it 31 for a while (objectmgr dependencies mandate this)
				auto originalIndex = player->physicalPlayerIndex();

				if (!EventNeedsOriginalPlayer(ev))
				{
					player->physicalPlayerIndex() = (player != rage::GetLocalPlayer()) ? 31 : 0;
				}

				if (ev->IsInScope(player))
				{
					targetPlayers.insert(g_netIdsByPlayer[player]);
				}

				player->physicalPlayerIndex() = originalIndex;
			}
#ifdef GTA_FIVE
			else
			{
				AddCrashometry("player_corruption", "true");
			}
#endif
		}
	}

	std::vector<uint16_t> targetPlayersVector;
	targetPlayersVector.reserve(targetPlayers.size());
	for (const uint16_t playerId : targetPlayers)
	{
		targetPlayersVector.push_back(playerId);
	}

	if (icgi->IsNetVersionOrHigher(net::NetBitVersion::netVersion4))
	{
		static std::vector<uint32_t> eventIdentsToHash = netEventMgr_GetIdentsToEventHash();

		if (ev->eventType >= eventIdentsToHash.size())
		{
			// invalid event id
			return;
		}

		net::packet::ClientNetGameEventV2Packet clientNetGameEvent;
		clientNetGameEvent.event.targetPlayers.SetValue({ targetPlayersVector.data(), targetPlayersVector.size() });
		clientNetGameEvent.event.eventId = eventId;
		clientNetGameEvent.event.isReply = false;
		clientNetGameEvent.event.eventNameHash = eventIdentsToHash[ev->eventType];
		clientNetGameEvent.event.data.SetValue(net::Span{static_cast<uint8_t*>(rlBuffer.m_data), rlBuffer.GetDataLength()});

		if (!g_netLibrary->SendNetPacket(clientNetGameEvent))
		{
			trace("Serialization of the net game event packet failed. Event target count: %d, Data length: %d\n", targetPlayersVector.size(), rlBuffer.GetDataLength());
		}
	}
	else
	{
		net::packet::ClientNetGameEventPacket clientNetGameEvent;
		clientNetGameEvent.event.targetPlayers.SetValue({ targetPlayersVector.data(), targetPlayersVector.size() });
		clientNetGameEvent.event.eventId = eventId;
		clientNetGameEvent.event.isReply = 0;
		clientNetGameEvent.event.eventType = ev->eventType;
		clientNetGameEvent.event.data.SetValue({static_cast<uint8_t*>(rlBuffer.m_data), rlBuffer.GetDataLength() });

		if (!g_netLibrary->SendNetPacket(clientNetGameEvent))
		{
			trace("Serialization of the net game event packet failed. Event target count: %d, Data length: %d\n", targetPlayersVector.size(), rlBuffer.GetDataLength());
		}
	}
}

void rage::HandleNetGameEvent(const char* idata, size_t len)
{
	if (!icgi->HasVariable("networkInited"))
	{
		return;
	}

	// TODO: use a real player for some things that _are_ 32-safe
	rage::EnsurePlayer31();

	net::Buffer buf(reinterpret_cast<const uint8_t*>(idata), len);
	auto sourcePlayerId = buf.Read<uint16_t>();
	auto eventHeader = buf.Read<uint16_t>();
	auto isReply = buf.Read<uint8_t>();
	auto eventType = buf.Read<uint16_t>();
	auto length = buf.Read<uint16_t>();

	// TEMPORARY: mapping back on receiving event from server
	eventType = netEventMgr_MapEventId(eventType, false);

	auto player = g_playersByNetId[sourcePlayerId];

	if (!player)
	{
		player = g_player31;
	}

	std::vector<uint8_t> data(length);
	buf.Read(data.data(), data.size());

	rage::datBitBuffer rlBuffer(const_cast<uint8_t*>(data.data()), data.size());
	rlBuffer.m_f1C = 1;

	if (eventType > netEventMgr_GetMaxEventType())
	{
		return;
	}

	if (isReply)
	{
		auto evSetIt = g_events.find({ eventType, eventHeader });

		if (evSetIt != g_events.end())
		{
			auto ev = evSetIt->second.ev;

			if (ev)
			{
				ev->HandleReply(&rlBuffer, player);

#ifdef GTA_FIVE
				ev->HandleExtraData(&rlBuffer, true, player, rage::GetLocalPlayer());
#elif IS_RDR3
				ev->HandleExtraData(&rlBuffer, player, rage::GetLocalPlayer());
#endif

#if defined(GTA_FIVE) && 0
				auto em = reinterpret_cast<rage::netEventMgr*>(*(char**)g_netEventMgr);
				em->RemoveEvent(ev);
#endif

				delete ev;
				g_events.erase({ eventType, eventHeader });
			}
		}
	}
	else
	{
		using TEventHandlerFn = void(*)(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t, uint32_t, uint32_t);
		if (netEventMgr_IsBlacklistedEvent(eventType))
		{
			//trace("Rejecting Blacklisted Event: %d\n", eventType);
			return;
		}

		bool rejected = false;

		// for all intents and purposes, the player will be 31
		auto lastIndex = player->physicalPlayerIndex();
		player->physicalPlayerIndex() = 31;

		auto eventMgr = *(char**)g_netEventMgr;

		if (eventMgr)
		{
#ifdef GTA_FIVE
			auto eventHandlerList = (TEventHandlerFn*)(eventMgr + (xbr::IsGameBuildOrGreater<2372>() ? 0x3B3D0 : xbr::IsGameBuildOrGreater<2060>() ? 0x3ABD0 : 0x3AB80));
#elif IS_RDR3
			auto eventHandlerList = (TEventHandlerFn*)(eventMgr + 0x3BF10);
#else
			auto eventHandlerList = (TEventHandlerFn*)(nullptr);
#endif

			auto eh = eventHandlerList[eventType];

			if (eh && (uintptr_t)eh >= hook::get_adjusted(0x140000000) && (uintptr_t)eh < hook::get_adjusted(hook::exe_end()))
			{
				eh(&rlBuffer, player, rage::GetLocalPlayer(), eventHeader, 0, 0);
				rejected = g_lastEventGotRejected;
			}
		}

		player->physicalPlayerIndex() = lastIndex;

		if (rejected)
		{
			g_reEventQueue.push_back(buf);
		}
	}
}

void rage::HandleNetGameEventV2(net::packet::ServerNetGameEventV2& serverNetGameEventV2)
{
	if (!icgi->HasVariable("networkInited"))
	{
		return;
	}

	// TODO: use a real player for some things that _are_ 32-safe
	rage::EnsurePlayer31();

	const uint16_t sourcePlayerId = serverNetGameEventV2.clientNetId.GetValue();
	const uint16_t eventHeader =  serverNetGameEventV2.eventId.GetValue();
	const bool isReply = serverNetGameEventV2.isReply.GetValue();
	const uint32_t eventNameHash = serverNetGameEventV2.eventNameHash.GetValue();

	auto player = g_playersByNetId[sourcePlayerId];

	if (!player)
	{
		player = g_player31;
	}

	rage::datBitBuffer rlBuffer(const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(serverNetGameEventV2.data.GetValue().data())), serverNetGameEventV2.data.GetValue().size());
	rlBuffer.m_f1C = 1;

	if (isReply)
	{
		auto evSetIt = g_eventsV2.find({ eventNameHash, eventHeader });

		if (evSetIt != g_eventsV2.end())
		{
			auto ev = evSetIt->second.ev;

			if (ev)
			{
				ev->HandleReply(&rlBuffer, player);

#ifdef GTA_FIVE
				ev->HandleExtraData(&rlBuffer, true, player, rage::GetLocalPlayer());
#elif IS_RDR3
				ev->HandleExtraData(&rlBuffer, player, rage::GetLocalPlayer());
#endif

				delete ev;
				g_eventsV2.erase({ eventNameHash, eventHeader });
			}
		}
	}
	else
	{
		using TEventHandlerFn = void(*)(rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t, uint32_t, uint32_t);
		if (netEventMgr_IsBlacklistedEventV2(eventNameHash))
		{
			//trace("Rejecting Blacklisted Event: %d\n", eventType);
			return;
		}

		bool rejected = false;

		// for all intents and purposes, the player will be 31
		auto lastIndex = player->physicalPlayerIndex();
		player->physicalPlayerIndex() = 31;

		auto eventMgr = *(char**)g_netEventMgr;

		static std::unordered_map<uint32_t, uint16_t> eventIdents = netEventMgr_GetEventHashIdents();

		const auto eventIdentRes = eventIdents.find(eventNameHash);

		if (eventIdentRes == eventIdents.end())
		{
			// unknown event
			return;
		}

		if (eventMgr)
		{
#ifdef GTA_FIVE
			auto eventHandlerList = (TEventHandlerFn*)(eventMgr + (xbr::IsGameBuildOrGreater<2372>() ? 0x3B3D0 : xbr::IsGameBuildOrGreater<2060>() ? 0x3ABD0 : 0x3AB80));
#elif IS_RDR3
			auto eventHandlerList = (TEventHandlerFn*)(eventMgr + 0x3BF10);
#else
			auto eventHandlerList = (TEventHandlerFn*)(nullptr);
#endif

			auto eh = eventHandlerList[eventIdentRes->second];

			if (eh && (uintptr_t)eh >= hook::get_adjusted(0x140000000) && (uintptr_t)eh < hook::get_adjusted(hook::exe_end()))
			{
				eh(&rlBuffer, player, rage::GetLocalPlayer(), eventHeader, 0, 0);
				rejected = g_lastEventGotRejected;
			}
		}

		player->physicalPlayerIndex() = lastIndex;

		if (rejected)
		{
			g_reEventQueueV2.emplace_back(serverNetGameEventV2);
		}
	}
}

#ifdef IS_RDR3
static void* RegisterNetGameEvent(void* eventMgr, uint16_t eventId, void* func, const char* name)
{
	g_eventNames.insert({ eventId, name });
	return g_origRegisterNetGameEvent(eventMgr, eventId, func, name);
}
#endif

static bool SendGameEvent(void* eventMgr, void* ev)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origSendGameEvent(eventMgr, ev);
	}

	return 1;
}

#if GTA_FIVE
static uint32_t GetFireApplicability(void* event, void* pos)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetFireApplicability(event, pos);
	}

	// send all fires to all remote players
	return (1 << 31);
}
#elif IS_RDR3
static uint32_t* GetFireApplicability(void* event, uint32_t* result, void* pos)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origGetFireApplicability(event, result, pos);
	}

	// send all fires to all remote players
	uint32_t value = (1 << 31);

	*result = value;
	return &value;
}
#endif

#if GTA_FIVE
/// CPlayerTauntEvent: Ensure CNetGamePlayer::GetPlayerPed returns a value Ped.
static bool CPlayerTauntEvent_Decide(void* self, CNetGamePlayer* sourcePlayer, void* connUnk)
{
	if (!CNetGamePlayer_GetPlayerPed(sourcePlayer))
	{
		return true;
	}
	return g_origCPlayerTauntEventDecide(self, sourcePlayer, connUnk);
}
#endif

#if defined(GTA_FIVE) || defined(IS_RDR3)
/// ReportCashSpawnEvent: Sanitize the gamer handle pointer since player31 may
/// not include a reference that value. Since we really aren't doing much with
/// rlMetric, this path will be nop'd.
static bool MetricCASH_IsGamerHandleValid(void* pGamerHandle)
{
	return false;
}
#endif

static void UnkEventMgr(void* mgr, void* ply)
{
	if (!icgi->OneSyncEnabled)
	{
		g_origUnkEventMgr(mgr, ply);
	}
}

inline std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static void EventMgr_AddEvent(void* eventMgr, rage::netGameEvent* ev)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origAddEvent(eventMgr, ev);
	}

	if (netEventMgr_IsBlacklistedEvent(ev->eventType))
	{
		delete ev;
		return;
	}

	// TEMPORARY: use event type mapping
	ev->eventType = netEventMgr_MapEventId(ev->eventType, true);

#ifdef GTA_FIVE
	if (strcmp(ev->GetName(), "ALTER_WANTED_LEVEL_EVENT") == 0)
	{
		// do we already have 5 ALTER_WANTED_LEVEL_EVENT instances?
		int count = 0;

		for (auto& eventPair : g_events)
		{
			auto [key, tup] = eventPair;

			if (tup.ev && strcmp(tup.ev->GetName(), "ALTER_WANTED_LEVEL_EVENT") == 0)
			{
				count++;
			}
		}

		for (auto& eventPair : g_eventsV2)
		{
			auto [key, tup] = eventPair;

			if (tup.ev && strcmp(tup.ev->GetName(), "ALTER_WANTED_LEVEL_EVENT") == 0)
			{
				count++;
			}
		}

		if (count >= 5)
		{
			delete ev;
			return;
		}
	}
#elif IS_RDR3
	// speech events (PED_SPEECH_*_EVENT) in RDR3 are very spammy sometimes and can cause pool overflow
	if (strcmp(ev->GetName(), "PED_SPEECH_") != -1)
	{
		int count = 0;

		for (auto& eventPair : g_events)
		{
			auto [key, tup] = eventPair;

			if (tup.ev && strcmp(tup.ev->GetName(), "PED_SPEECH_") != -1)
			{
				count++;
			}
		}

		for (auto& eventPair : g_eventsV2)
		{
			auto [key, tup] = eventPair;

			if (tup.ev && strcmp(tup.ev->GetName(), "PED_SPEECH_") != -1)
			{
				count++;
			}
		}

		if (count >= 50)
		{
			delete ev;
			return;
		}
	}
#endif

	// checks (for events where Equals() may modify the left-hand-side event) if the event
	// has already been sent, and Equals() + deletion may therefore be destructive to the unique data
	//
	// see GH-1490
	auto isSentModifying = [](const netGameEventState& eventTuple)
	{
		static const auto weaponDamageEventHash = HashString("WEAPON_DAMAGE_EVENT");
		static const auto giveWeaponEventHash = HashString("GIVE_WEAPON_EVENT");
		static const auto updateSyncedSceneEventHash = HashString("NETWORK_UPDATE_SYNCED_SCENE_EVENT");
		static const auto givePickupRewardsEventHash = HashString("NETWORK_GIVE_PICKUP_REWARDS_EVENT");
		static const auto scriptedGameEventHash = HashString("SCRIPTED_GAME_EVENT");

		if (eventTuple.sent)
		{
#if defined(GTA_FIVE) || defined(IS_RDR3)
			auto thisEventHash = HashString(eventTuple.ev->GetName());

			return thisEventHash == weaponDamageEventHash ||
				thisEventHash == giveWeaponEventHash ||
				thisEventHash == updateSyncedSceneEventHash ||
				thisEventHash == givePickupRewardsEventHash ||
				thisEventHash == scriptedGameEventHash;
#endif
		}

		return false;
	};

	// is this a duplicate event?
	for (auto& eventPair : g_events)
	{
		auto [key, tup] = eventPair;

		if (tup.ev && !isSentModifying(tup) && tup.ev->Equals(ev))
		{
			delete ev;
			return;
		}
	}

	for (auto& eventPair : g_eventsV2)
	{
		auto [key, tup] = eventPair;

		if (tup.ev && !isSentModifying(tup) && tup.ev->Equals(ev))
		{
			delete ev;
			return;
		}
	}

	auto eventId = (ev->hasEventId) ? ev->eventId : g_eventHeader++;

	if (icgi->IsNetVersionOrHigher(net::NetBitVersion::netVersion4))
	{
		static std::vector<uint32_t> eventIdentsToHash = netEventMgr_GetIdentsToEventHash();
		if (ev->eventType >= eventIdentsToHash.size())
		{
			// invalid event id
			delete ev;
			return;
		}

		auto [it, inserted] = g_eventsV2.insert({ { eventIdentsToHash[ev->eventType], eventId }, { ev, msec() } });

		if (!inserted)
		{
			delete ev;
		}
	}
	else
	{
		auto [it, inserted] = g_events.insert({ { ev->eventType, eventId }, { ev, msec() } });

		if (!inserted)
		{
			delete ev;
		}
		else
		{
#if defined(GTA_FIVE) && 0
			auto em = reinterpret_cast<rage::netEventMgr*>(eventMgr);
			em->AddEvent(ev);
#endif
		}
	}
}

static void DecideNetGameEvent(rage::netGameEvent* ev, CNetGamePlayer* player, CNetGamePlayer* unkConn, rage::datBitBuffer* buffer, uint16_t evH)
{
	g_lastEventGotRejected = false;

	if (ev->Decide(player, unkConn))
	{
#ifdef GTA_FIVE
		ev->HandleExtraData(buffer, false, player, unkConn);
#elif IS_RDR3
		ev->HandleExtraData(buffer, player, unkConn);
#endif

		if (ev->requiresReply)
		{
			uint8_t packetStub[1024];
			rage::datBitBuffer rlBuffer(packetStub, sizeof(packetStub));

			ev->PrepareReply(&rlBuffer, player);

#ifdef GTA_FIVE
			ev->PrepareExtraData(&rlBuffer, true, player, nullptr);
#elif IS_RDR3
			ev->PrepareExtraData(&rlBuffer, player, nullptr);
#endif

			if (icgi->IsNetVersionOrHigher(net::NetBitVersion::netVersion4))
			{
				static std::vector<uint32_t> eventIdentsToHash = netEventMgr_GetIdentsToEventHash();

				if (ev->eventType >= eventIdentsToHash.size())
				{
					// invalid event id
					return;
				}

				const auto targetPlayerRes = g_netIdsByPlayer.find(player);

				net::packet::ClientNetGameEventV2Packet clientNetGameEvent;
				uint16_t targetPlayerId;
				if (targetPlayerRes != g_netIdsByPlayer.end())
				{
					// target player is not available for the reply
					// but sending it to the server for event processing is fine
					targetPlayerId = targetPlayerRes->second;
					clientNetGameEvent.event.targetPlayers.SetValue({&targetPlayerId, 1});
				}
				clientNetGameEvent.event.eventId = evH;
				clientNetGameEvent.event.isReply = true;
				clientNetGameEvent.event.eventNameHash = eventIdentsToHash[ev->eventType];
				clientNetGameEvent.event.data = {static_cast<uint8_t*>(rlBuffer.m_data), rlBuffer.GetDataLength()};

				if (!g_netLibrary->SendNetPacket(clientNetGameEvent))
				{
					trace("Serialization of the net game event reply packet failed. Event target count: %d, Data length: %d\n", clientNetGameEvent.event.targetPlayers.GetValue().size(), rlBuffer.GetDataLength());
				}
			} else
			{
				net::packet::ClientNetGameEventPacket clientNetGameEvent;
				uint16_t targetPlayerId;
				if (const auto targetPlayerRes = g_netIdsByPlayer.find(player); targetPlayerRes != g_netIdsByPlayer.end())
				{
					// target player is not available for the reply
					// but sending it to the server for event processing is fine
					targetPlayerId = targetPlayerRes->second;
					clientNetGameEvent.event.targetPlayers.SetValue({&targetPlayerId, 1});
				}

				clientNetGameEvent.event.eventId = evH;
				clientNetGameEvent.event.isReply = 1;
				clientNetGameEvent.event.eventType = ev->eventType;
				clientNetGameEvent.event.data = {static_cast<uint8_t*>(rlBuffer.m_data), rlBuffer.GetDataLength()};

				if (!g_netLibrary->SendNetPacket(clientNetGameEvent))
				{
					trace("Serialization of the net game event reply packet failed. Event target count: %d, Data length: %d\n", clientNetGameEvent.event.targetPlayers.GetValue().size(), rlBuffer.GetDataLength());
				}
			}
		}
	}
	else
	{
		g_lastEventGotRejected = !ev->HasTimedOut() && ev->MustPersist();
	}
}

static void ExecuteNetGameEvent(void* eventMgr, rage::netGameEvent* ev, rage::datBitBuffer* buffer, CNetGamePlayer* player, CNetGamePlayer* unkConn, uint16_t evH, uint32_t a, uint32_t b)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origExecuteNetGameEvent(eventMgr, ev, buffer, player, unkConn, evH, a, b);
	}

	ev->Handle(buffer, player, unkConn);

	// missing: some checks
	DecideNetGameEvent(ev, player, unkConn, buffer, evH);
}

static void NetEventError()
{
	auto pool = rage::GetPoolBase("netGameEvent");

	std::map<std::string, int> poolCount;

	for (int i = 0; i < pool->GetSize(); i++)
	{
		if (const auto netGameEvent = pool->GetAt<rage::netGameEvent>(i))
		{
#if defined(GTA_FIVE) || defined(IS_RDR3)
			poolCount[netGameEvent->GetName()]++;
#endif
		}
	}

	std::vector<std::pair<int, std::string>> entries;

	for (const auto& [ type, count ] : poolCount)
	{
		entries.push_back({ count, type });
	}

	std::sort(entries.begin(), entries.end(), [](const auto& l, const auto& r)
	{
		return r.first < l.first;
	});

	std::string poolSummary;

	for (const auto& [count, type] : entries)
	{
		poolSummary += fmt::sprintf("  %s: %d entries\n", type, count);
	}

	FatalError("Ran out of rage::netGameEvent pool space.\n\nPool usage:\n%s", poolSummary);
}

namespace rage
{
#ifdef GTA_FIVE
	void netEventMgr::AddEvent(netGameEvent* event)
	{
		auto node = new atDNetEventNode();
		if (node)
		{
			node->SetData(event);
			eventList.Add(node);
		}
	}

	void netEventMgr::RemoveEvent(netGameEvent* event)
	{
		eventList.RemoveData(event);
	}

	bool netEventMgr::HasEvent(netGameEvent* event)
	{
		return eventList.Has(event);
	}

	void netEventMgr::ClearEvents()
	{
		eventList.Clear();
	}
#endif

	void EventManager_Update()
	{
#ifdef IS_RDR3
		if (!g_netGameEventPool)
		{
			auto pool = rage::GetPoolBase("netGameEvent");

			if (pool)
			{
				g_netGameEventPool = &pool;
			}
		}
#endif

		if (!g_netGameEventPool || !*g_netGameEventPool)
		{
			return;
		}

		std::set<decltype(g_events)::key_type> toRemove;

		for (auto& eventPair : g_events)
		{
			auto& evSet = eventPair.second;
			auto ev = evSet.ev;

			if (ev)
			{
				if (!evSet.sent)
				{
					SendGameEventRaw(std::get<1>(eventPair.first), ev);

					evSet.sent = true;
				}

				auto expiryDuration = 5s;

				if (ev->HasTimedOut() || (msec() - evSet.time) > expiryDuration)
				{
#if defined(GTA_FIVE) && 0
					auto em = reinterpret_cast<rage::netEventMgr*>(*(char**)g_netEventMgr);
					em->RemoveEvent(ev);
#endif

					delete ev;

					toRemove.insert(eventPair.first);
				}
			}
		}

		for (auto var : toRemove)
		{
			g_events.erase(var);
		}

		std::set<decltype(g_eventsV2)::key_type> toRemoveV2;

		for (auto& eventPair : g_eventsV2)
		{
			auto& evSet = eventPair.second;
			auto ev = evSet.ev;

			if (ev)
			{
				if (!evSet.sent)
				{
					SendGameEventRaw(std::get<1>(eventPair.first), ev);

					evSet.sent = true;
				}

				auto expiryDuration = 5s;

				if (ev->HasTimedOut() || (msec() - evSet.time) > expiryDuration)
				{
					delete ev;

					toRemoveV2.insert(eventPair.first);
				}
			}
		}

		for (auto var : toRemoveV2)
		{
			g_eventsV2.erase(var);
		}

		// re-events
		std::vector<net::Buffer> reEvents;

		while (!g_reEventQueue.empty())
		{
			reEvents.push_back(std::move(g_reEventQueue.front()));
			g_reEventQueue.pop_front();
		}

		while (!g_reEventQueueV2.empty())
		{
			net::packet::ServerNetGameEventV2 serverNetGameEvent;
			serverNetGameEvent.eventNameHash = g_reEventQueueV2.front().eventNameHash;
			serverNetGameEvent.clientNetId = g_reEventQueueV2.front().clientNetId;
			serverNetGameEvent.eventId = g_reEventQueueV2.front().eventId;
			serverNetGameEvent.isReply = g_reEventQueueV2.front().isReply;
			serverNetGameEvent.data = {g_reEventQueueV2.front().eventData.data(), g_reEventQueueV2.front().eventData.size()};

			HandleNetGameEventV2(serverNetGameEvent);

			g_reEventQueueV2.pop_front();
		}

		for (auto& evBuf : reEvents)
		{
			evBuf.Seek(0);
			HandleNetGameEvent(reinterpret_cast<const char*>(evBuf.GetBuffer()), evBuf.GetLength());
		}
	}

	bool EnsurePlayer31()
	{
		if (!g_player31)
		{
			g_player31 = AllocateNetPlayer(nullptr);
			g_player31->physicalPlayerIndex() = 31;
		}

		return (g_player31 != nullptr);
	}

	CNetGamePlayer* GetPlayer31()
	{
		return g_player31;
	}

} // rage

#ifdef IS_RDR3
const char* rage::netGameEvent::GetName()
{
	auto findEvent = g_eventNames.find(this->eventType);

	if (findEvent == g_eventNames.end())
	{
		return "UNKNOWN_EVENT";
	}

	return findEvent->second;
}
#endif

static InitFunction initFunction([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		icgi = Instance<ICoreGameInit>::Get();

		netLibrary->AddPacketHandler<fx::NetGameEventPacketHandler>(true);
		netLibrary->AddPacketHandler<fx::NetGameEventV2PacketHandler>(true);
	});

	OnKillNetworkDone.Connect([]()
	{
#if defined(GTA_FIVE) && 0
		auto em = reinterpret_cast<rage::netEventMgr*>(*(char**)g_netEventMgr);

		if (em)
		{
			em->ClearEvents();
		}
#endif

		g_events.clear();
		g_eventsV2.clear();
		g_reEventQueue.clear();
	});
});

static HookFunction hookFunction([]()
{
	MH_Initialize();

	{
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 84 C0 74 11 48 8B 0D", 0x14);

		g_netEventMgr = hook::get_address<void*>(location);
		MH_CreateHook(hook::get_call(location + 7), EventMgr_AddEvent, (void**)&g_origAddEvent);

#if 0
		static auto eventLoc = hook::get_pattern("4D 85 FF 0F 84 ? ? ? ? 4D 8B 77 08 4D 8B", 3); //, 0xE990);
		static auto origEvent = *(uint16_t*)eventLoc;

		Instance<ICoreGameInit>::Get()->OnSetVariable.Connect([](const std::string& varName, bool newValue)
		{
			if (varName == "onesync")
			{
				if (!newValue)
				{
					// who knows, put back origEvent
				}
				else
				{
					// write 0xE990;
				}
			}
		});
#endif
#elif IS_RDR3
		auto location = hook::get_pattern<char>("C6 47 50 01 4C 8B C3 49 8B D7", (xbr::IsGameBuildOrGreater<1436>()) ? 0x59 : 0x21);

		g_netEventMgr = hook::get_address<void*>(location);
		MH_CreateHook(hook::get_call(location + 7), EventMgr_AddEvent, (void**)&g_origAddEvent);
#endif
	}

	// we hook game event registration to store event names
#ifdef IS_RDR3
	MH_CreateHook(hook::get_pattern("48 83 C1 08 0F B7 FA E8 ? ? ? ? B8", -0x1A), RegisterNetGameEvent, (void**)&g_origRegisterNetGameEvent);
#endif

#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("48 8B DA 48 8B F1 41 81 FF 00", -0x2A), ExecuteNetGameEvent, (void**)&g_origExecuteNetGameEvent);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("48 89 6C 24 60 4D 8B F1 49 8B ? 48 8B DA E8", -0x25), ExecuteNetGameEvent, (void**)&g_origExecuteNetGameEvent);
#endif

	// can cause crashes due to high player indices, default event sending
#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("48 83 EC 30 80 7A ? FF 4C 8B D2", -0xC), SendGameEvent, (void**)&g_origSendGameEvent);
#elif IS_RDR3
	MH_CreateHook((xbr::IsGameBuildOrGreater<1436>()) ? hook::get_pattern("41 8A 5E 19 45 33 E4 80 FB 20 72", -0x27) : hook::get_pattern("48 83 EC 30 48 8B F9 4C 8B F2 48 83 C1 08", -0xE), SendGameEvent, (void**)&g_origSendGameEvent);
#endif

	// fire applicability
#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("85 DB 74 78 44 8B F3 48", -0x30), GetFireApplicability, (void**)&g_origGetFireApplicability);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("48 8B 0C C1 4C 39 24 0A 75 04 33 C0", -0x3A), GetFireApplicability, (void**)&g_origGetFireApplicability);
#endif

#ifdef GTA_FIVE
	// CAlterWantedLevelEvent pool check
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		MH_CreateHook(hook::get_call(hook::get_pattern("45 8A C4 48 8B C8 41 8B D7", 8)), SendAlterWantedLevelEvent1Hook, (void**)&g_origSendAlterWantedLevelEvent1);
		MH_CreateHook(hook::get_pattern("4C 8B 78 10 48 85 F6", -0x58), SendAlterWantedLevelEvent2Hook, (void**)&g_origSendAlterWantedLevelEvent2);
	}
	else
	{
		MH_CreateHook(hook::get_call(hook::get_pattern("45 8A C6 48 8B C8 8B D5 E8 ? ? ? ? 45 32 E4", 8)), SendAlterWantedLevelEvent1Hook, (void**)&g_origSendAlterWantedLevelEvent1);
		MH_CreateHook(hook::get_pattern("4C 8B 78 10 48 85 ED 74 74 66 39 55", -0x58), SendAlterWantedLevelEvent2Hook, (void**)&g_origSendAlterWantedLevelEvent2);
	}
#endif

	// CPlayerTauntEvent may interact negatively with player31.
#ifdef GTA_FIVE
	{
		auto location = hook::get_pattern("33 F6 48 39 B0 ? ? ? ? 74 7B 48 8B CB 48 C7 45", -41);
		MH_CreateHook(location, CPlayerTauntEvent_Decide, (void**)&g_origCPlayerTauntEventDecide);
	}
#endif

	// CReportCashSpawnEvent may interact negatively with player31.
	{
#if defined(GTA_FIVE) || defined(IS_RDR3)
#ifdef GTA_FIVE
		auto location = hook::get_pattern<char>("8B 44 24 50 48 89 5E 18 89 7E 40 89 46 44", 0x15);
#else
		auto location = hook::get_pattern<char>("8B 44 24 ? 89 46 44 89 7E 40 C6 46 20 00", 0xE);
#endif
		hook::set_call(&g_origMetricCASHIsGamerHandleValid, location);
		hook::call(location, MetricCASH_IsGamerHandleValid);
#endif
	}

	// CheckForSpaceInPool error display
#ifdef GTA_FIVE
	if (xbr::IsGameBuildOrGreater<2802>())
	{
		hook::call(hook::get_pattern("33 C9 E8 ? ? ? ? E9 FC FE FF FF", 2), NetEventError);
	}
	else
	{
		hook::call(hook::get_pattern("33 C9 E8 ? ? ? ? E9 FD FE FF FF", 2), NetEventError);
	}
#elif IS_RDR3
	hook::call((xbr::IsGameBuildOrGreater<1436>()) ? hook::get_pattern("74 ? 48 8B 01 40 8A D6 FF ? ? BA", 25) : hook::get_pattern("BA 01 00 00 00 FF ? ? BA 5B 52 1C A4", 22), NetEventError);
#endif

	// func that reads neteventmgr by player idx, crashes page heap
#ifdef GTA_FIVE
	MH_CreateHook(hook::get_pattern("80 7A ? FF 48 8B EA 48 8B F1 0F", -0x13), UnkEventMgr, (void**)&g_origUnkEventMgr);
#elif IS_RDR3
	MH_CreateHook(hook::get_pattern("41 57 48 83 EC 30 ? 8B ? ? 8B ? 48 83 C1 08 E8", -0x12), UnkEventMgr, (void**)&g_origUnkEventMgr);
#endif

	MH_EnableHook(MH_ALL_HOOKS);

#ifdef GTA_FIVE
	{
		auto location = hook::get_pattern("44 8B 40 20 8B 40 10 41 C1 E0 02 41 C1 F8 02 41 2B C0 0F 85", -7);
		g_netGameEventPool = hook::get_address<decltype(g_netGameEventPool)>(location);
	}
#endif
});
