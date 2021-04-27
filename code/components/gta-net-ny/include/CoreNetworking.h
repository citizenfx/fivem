#pragma once

namespace rage
{
// rline
struct netPeerId
{
	uint64_t val;
};

struct rlGamerHandle
{
	uint8_t handle[16];
};

struct netIpAddress
{
	union
	{
		uint32_t addr;
		uint8_t bytes[4];
	};
};

struct netSocketAddress
{
	netIpAddress ip;
	uint16_t port;
};

struct netPeerUnkStructInner
{
	netSocketAddress addr;
	uint32_t unk;
};

struct netPeerUnkStruct
{
	netPeerUnkStructInner unks[4];
	int numUnks;
};

struct netPeerAddress
{
	uint64_t unkKey1;
	uint64_t unkKey2;
	uint32_t secKeyTime; // added in 393
	netSocketAddress relayAddr;
	netSocketAddress publicAddr;
	netSocketAddress localAddr;
	uint32_t unkVal;
	uint64_t rockstarAccountId; // 463/505
};

struct rlSessionToken
{
	uint64_t token1;
	uint64_t token2;
};

struct rlSessionInfo
{
	rlSessionToken sessionToken;
	netPeerAddress peerAddress;
};

struct rlGamerInfo
{
	netPeerAddress peerAddress;
	char m_pad[0xB4 - sizeof(netPeerAddress)];
	char name[16];
};
}