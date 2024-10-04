#pragma once

#include "TimeSync.h"

struct TimeSyncPadding1311
{
	char m_pad[4];
};

struct TimeSyncPadding1355
{
	char m_pad[24];
};

template<bool Enable>
using TimeSyncPadding = std::conditional_t<Enable, TimeSyncPadding1355, TimeSyncPadding1311>;

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
	void HandleTimeSync(net::packet::TimeSyncResponse& packet);
	bool IsInitialized();
	void SetConnectionManager(netConnectionManager* mgr);

private:
	netConnectionManager* m_connectionMgr; // +8 (1311)
	uint32_t m_unkTrust; // +16
	uint32_t m_sessionKey; // +20
	char m_pad_24[44]; // +24
	TimeSyncPadding<(Build >= 1355)> m_pad_68;
	uint32_t m_nextSync; // +72
	uint32_t m_configTimeBetweenSyncs; // +76
	uint32_t m_configMaxBackoff; // +80, usually 60000
	uint32_t m_effectiveTimeBetweenSyncs; // +84
	uint32_t m_lastRtt; // +88
	uint32_t m_retryCount; // +92
	uint32_t m_requestSequence; // +96
	uint32_t m_replySequence; // +100
	uint32_t m_flags; // +104
	uint32_t m_packetFlags; // +108
	int32_t m_timeDelta; // +112
	char m_pad_116[28];
	uint32_t m_lastTime; // +144, used to prevent time from going backwards
	uint8_t m_applyFlags; // +148
	uint8_t m_unk5; // +149
	uint8_t m_disabled; // +150
	uint8_t m_useCloudTime; // +151
};

static void HandleTimeSyncUpdatePacket(net::packet::TimeSyncResponse& buf);
}

static_assert(sizeof(rage::netTimeSync<1311>) == 152);
static_assert(sizeof(rage::netTimeSync<1355>) == 176);
