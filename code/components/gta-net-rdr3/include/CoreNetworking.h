#pragma once

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
	netPeerId peerId;
	rlGamerHandle gamerHandle;
	uint8_t peerKey[32];
	uint8_t hasPeerKey;
	netSocketAddress relayAddr;
	netSocketAddress publicAddr;
	netSocketAddress localAddr;
	netPeerUnkStruct unk;
	int natType;
};

struct rlSessionToken
{
	uint64_t token;
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
