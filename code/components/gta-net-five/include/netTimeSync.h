#pragma once

#include "TimeSync.h"

struct TrustAddressData
{
	uint32_t m_addr;
	uint16_t m_port;
};

struct TrustAddress1604
{
	TrustAddressData m_addr1;
	TrustAddressData m_addr2;
};

struct TrustAddress2060
{
	TrustAddressData m_addr1;
	TrustAddressData m_addr2;
	TrustAddressData m_addr3;
};

struct TrustAddress2372
{
	uint32_t m_addr;
};

template<int Build>
using TrustAddress = std::conditional_t<(Build >= 2372), TrustAddress2372, std::conditional_t<(Build >= 2060), TrustAddress2060, TrustAddress1604>>;

namespace sync
{
bool IsWaitingForTimeSync();
}

namespace rage
{
class netConnectionManager;

template<int Build>
class netTimeSync
{
public:
	virtual ~netTimeSync() = 0;

	void Update();
	void HandleTimeSync(net::packet::TimeSyncResponse& reader);
	bool IsInitialized();
	void SetConnectionManager(netConnectionManager* mgr);
private:
	netConnectionManager* m_connectionMgr; // +8 (1604)
	TrustAddress<Build> m_trustAddr; // +16
	uint32_t m_sessionKey; // +32
	int32_t m_timeDelta; // +36
	struct
	{
		void* self;
		void* cb;
	} m_messageDelegate; // +40
	char m_pad_56[32];
	uint32_t m_nextSync; // +88
	uint32_t m_configTimeBetweenSyncs; // +92
	uint32_t m_configMaxBackoff; // +96, usually 60000
	uint32_t m_effectiveTimeBetweenSyncs; // +100
	uint32_t m_lastRtt; // +104
	uint32_t m_retryCount; // +108
	uint32_t m_requestSequence; // +112
	uint32_t m_replySequence; // +116
	uint32_t m_flags; // +120
	uint32_t m_packetFlags; // +124
	uint32_t m_lastTime; // +128, used to prevent time from going backwards
	uint8_t m_applyFlags; // +132
	uint8_t m_disabled; // +133
};

static void HandleTimeSyncUpdatePacket(net::packet::TimeSyncResponse& buf);
}

static_assert(sizeof(rage::netTimeSync<1604>) == 136);
static_assert(sizeof(rage::netTimeSync<2060>) == 144);
static_assert(sizeof(rage::netTimeSync<2372>) == 128);
