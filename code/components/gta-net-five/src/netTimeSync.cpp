#include <StdInc.h>

#include <mmsystem.h>
#include <MinHook.h>

#include "Hooking.h"
#include "NetLibrary.h"
#include "CrossBuildRuntime.h"

#include "GameInit.h"
#include "ICoreGameInit.h"

#include "nutsnbolts.h"
#include "netTimeSync.h"

#include <TimeSync.h>

#include "ByteWriter.h"
#include "PacketHandler.h"

extern ICoreGameInit* icgi;
extern NetLibrary* g_netLibrary;

static bool g_initedTimeSync;

static hook::cdecl_stub<rage::netConnectionManager*()> _getConnectionManager([]()
{
#ifdef GTA_FIVE
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? C7 44 24 40 60 EA 00 00"));
#elif IS_RDR3
	return hook::get_call(hook::get_pattern("48 8B D0 8B 46 ? 44 8B CD C7", -13));
#endif
});

template<int Build>
static rage::netTimeSync<Build>** g_netTimeSync;

static bool (*g_origInitializeTime)(void* timeSync, void* connectionMgr, uint32_t flags, void* trustHost,
	uint32_t sessionSeed, int* deltaStart, int packetFlags, int initialBackoff, int maxBackoff);

template<int Build>
bool netTimeSync__InitializeTimeStub(rage::netTimeSync<Build>* timeSync, rage::netConnectionManager* connectionMgr,
	uint32_t flags, void* trustHost, uint32_t sessionSeed, int* deltaStart, int packetFlags, int initialBackoff, int maxBackoff)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origInitializeTime(timeSync, connectionMgr, flags, trustHost, sessionSeed, deltaStart, packetFlags, initialBackoff, maxBackoff);
	}

	timeSync->SetConnectionManager(connectionMgr);

	return true;
}

template<int Build>
void rage::netTimeSync<Build>::Update()
{
	if (!icgi->OneSyncEnabled || !g_netLibrary)
	{
		return;
	}

	if (/*m_connectionMgr /*&& m_flags & 2 && */ !m_disabled)
	{
		const uint32_t curTime = timeGetTime();

		if (!m_nextSync || static_cast<int32_t>(timeGetTime() - m_nextSync) >= 0)
		{
			m_requestSequence++;

			net::packet::TimeSyncRequestPacket packet;
			packet.data.requestTime = curTime;
			packet.data.requestSequence = m_requestSequence;

			static size_t kMaxTimeSyncRequestPacket = net::SerializableComponent::GetSize<net::packet::TimeSyncRequestPacket>();
			static std::vector<uint8_t> packetBuffer (kMaxTimeSyncRequestPacket);

			net::ByteWriter writer (packetBuffer.data(), kMaxTimeSyncRequestPacket);
			if (!packet.Process(writer))
			{
				trace("Serialization of the TimeSyncRequestPacket failed. Please report this error at https://github.com/citizenfx/fivem.\n");
				return;
			}

			g_netLibrary->SendReliablePacket(packet.type, reinterpret_cast<const char*>(packetBuffer.data()), writer.GetOffset());

			m_nextSync = (curTime + m_effectiveTimeBetweenSyncs) | 1;
		}
	}
}

template<int Build>
void rage::netTimeSync<Build>::HandleTimeSync(net::packet::TimeSyncResponse& packet)
{
	const uint32_t reqTime = packet.request.requestTime;
	const uint32_t reqSequence = packet.request.requestSequence;
	const uint32_t resDelta = packet.serverTimeMillis;

	if (m_disabled)
	{
		return;
	}

	/*if (!(m_flags & 2))
	{
		return;
	}*/

	// out of order?
	if (int32_t(reqSequence - m_replySequence) <= 0)
	{
		return;
	}

	const auto rtt = timeGetTime() - reqTime;

	// bad timestamp, negative time passed
	if (int32_t(rtt) <= 0)
	{
		return;
	}

	const int32_t timeDelta = resDelta + (rtt / 2) - timeGetTime();

	// is this a low RTT, or did we retry often enough?
	if (rtt <= 300 || m_retryCount >= 10)
	{
		if (!m_lastRtt)
		{
			m_lastRtt = rtt;
		}

		// is RTT within variance, low, or retried?
		if (rtt <= 100 || (rtt / m_lastRtt) < 2 || m_retryCount >= 10)
		{
			m_timeDelta = timeDelta;
			m_replySequence = reqSequence;

			// progressive backoff once we've established a valid time base
			if (m_effectiveTimeBetweenSyncs < m_configMaxBackoff)
			{
				m_effectiveTimeBetweenSyncs = std::min(m_configMaxBackoff, m_effectiveTimeBetweenSyncs * 2);
			}

			m_retryCount = 0;

			// use flag 4 to reset time at least once, even if game session code has done so to a higher value
			if (!(m_applyFlags & 4))
			{
				m_lastTime = m_timeDelta + timeGetTime();
			}

			m_applyFlags |= 7;
		}
		else
		{
			m_nextSync = 0;
			m_retryCount++;
		}

		// update average RTT
		m_lastRtt = (rtt + m_lastRtt) / 2;
	}
	else
	{
		m_nextSync = 0;
		m_retryCount++;
	}
}

template<int Build>
bool rage::netTimeSync<Build>::IsInitialized()
{
	if (!g_initedTimeSync)
	{
#ifdef IS_RDR3
		// we don't want to use cloud time
		m_useCloudTime = false;
#endif

		g_origInitializeTime(this, _getConnectionManager(), 1, nullptr, 0, nullptr, 7, 2000, 60000);

		// to make the game not try to get time from us
		m_connectionMgr = nullptr;

		g_initedTimeSync = true;

		return false;
	}

	return (m_applyFlags & 4) != 0;
}

template<int Build>
inline void rage::netTimeSync<Build>::SetConnectionManager(netConnectionManager* mgr)
{
	m_connectionMgr = mgr;
}

bool sync::IsWaitingForTimeSync()
{
#ifdef GTA_FIVE
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		return !(*g_netTimeSync<2372>)->IsInitialized();
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		return !(*g_netTimeSync<2060>)->IsInitialized();
	}

	return !(*g_netTimeSync<1604>)->IsInitialized();
#elif IS_RDR3
	if (xbr::IsGameBuildOrGreater<1355>())
	{
		return !(*g_netTimeSync<1355>)->IsInitialized();
	}

	return !(*g_netTimeSync<1311>)->IsInitialized();
#endif
}

static inline void TimeSyncMainGameFrameUpdate()
{
#if GTA_FIVE
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		(*g_netTimeSync<2372>)->Update();
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		(*g_netTimeSync<2060>)->Update();
	}
	else
	{
		(*g_netTimeSync<1604>)->Update();
	}
#elif IS_RDR3
	if (xbr::IsGameBuildOrGreater<1355>())
	{
		(*g_netTimeSync<1355>)->Update();
	}
	else
	{
		(*g_netTimeSync<1311>)->Update();
	}
#endif
}

static void rage::HandleTimeSyncUpdatePacket(net::packet::TimeSyncResponse& buf)
{
#ifdef GTA_FIVE
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		(*g_netTimeSync<2372>)->HandleTimeSync(buf);
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		(*g_netTimeSync<2060>)->HandleTimeSync(buf);
	}
	else
	{
		(*g_netTimeSync<1604>)->HandleTimeSync(buf);
	}
#elif IS_RDR3
	if (xbr::IsGameBuildOrGreater<1355>())
	{
		(*g_netTimeSync<1355>)->HandleTimeSync(buf);
	}
	else
	{
		(*g_netTimeSync<1311>)->HandleTimeSync(buf);
	}
#endif
}

namespace fx
{
class TimeSyncResponsePacketHandler : public net::PacketHandler<net::packet::TimeSyncResponse, HashRageString("msgTimeSync")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::TimeSyncResponse& timeSyncResponse)
		{
			rage::HandleTimeSyncUpdatePacket(timeSyncResponse);
		});
	}
};
}

static InitFunction initFunction([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		lib->AddPacketHandler<fx::TimeSyncResponsePacketHandler>(false);
	});
});

static HookFunction hookFunction([]()
{
	MH_Initialize();

	{
#ifdef GTA_FIVE
		void* func = xbr::IsGameBuildOrGreater<2372>() ? (void*)&netTimeSync__InitializeTimeStub<2372> :
			xbr::IsGameBuildOrGreater<2060>() ? (void*)&netTimeSync__InitializeTimeStub<2060> :
			&netTimeSync__InitializeTimeStub<1604>;

		auto location = hook::get_pattern("48 8B D9 48 39 79 08 0F 85 ? ? 00 00 41 8B E8", -32);
#elif IS_RDR3
		void* func = (xbr::IsGameBuildOrGreater<1355>()) ? (void*)&netTimeSync__InitializeTimeStub<1355> :
			&netTimeSync__InitializeTimeStub<1311>;

		auto location = xbr::IsGameBuildOrGreater<1436>() ? hook::get_pattern("83 C8 FF 4C 89 77 08 83 FD", -87) :
			hook::get_pattern("48 89 51 08 41 83 F8 02 44 0F 45 C8", -49);
#endif

		MH_CreateHook(location, func, (void**)&g_origInitializeTime);
	}

	MH_EnableHook(MH_ALL_HOOKS);

	{
#ifdef GTA_FIVE
		if (xbr::IsGameBuildOrGreater<2372>())
		{
			g_netTimeSync<2372> = hook::get_address<rage::netTimeSync<2372>**>(hook::get_pattern("48 8B 0D ? ? ? ? 45 33 C9 45 33 C0 41 8D 51 01 E8", 3));
		}
		else if (xbr::IsGameBuildOrGreater<2060>())
		{
			g_netTimeSync<2060> = hook::get_address<rage::netTimeSync<2060>**>(hook::get_pattern("48 8B 0D ? ? ? ? 45 33 C9 45 33 C0 41 8D 51 01 E8", 3));
		}
		else
		{
			g_netTimeSync<1604> = hook::get_address<rage::netTimeSync<1604>**>(hook::get_pattern("EB 16 48 8B 0D ? ? ? ? 45 33 C9 45 33 C0", 5));
		}
#elif IS_RDR3
		auto location = hook::get_pattern("4C 8D 45 50 41 03 C7 44 89 6D 50 89", -4);

		if (xbr::IsGameBuildOrGreater<1355>())
		{
			g_netTimeSync<1355> = hook::get_address<rage::netTimeSync<1355>**>(location);
		}
		else
		{
			g_netTimeSync<1311> = hook::get_address<rage::netTimeSync<1311>**>(location);
		}
#endif	
	}

	OnMainGameFrame.Connect([]()
	{
		TimeSyncMainGameFrameUpdate();
	});

#ifdef IS_RDR3
	OnKillNetworkDone.Connect([]()
	{
		// RDR3 doesn't restart netTimeSync after disconnecting from a server with enabled OneSync
		g_initedTimeSync = false;
	});
#endif
});
