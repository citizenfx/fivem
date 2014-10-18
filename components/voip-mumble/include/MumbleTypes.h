#pragma once

#include <WS2tcpip.h>

#define MAX_MUMBLE_MESSAGE 25

enum class MumbleMessageType
{
	Version,
	UDPTunnel,
	Authenticate,
	Ping,
	Reject,
	ServerSync,
	ChannelRemove,
	ChannelState,
	UserRemove,
	UserState,
	BanList, /* 10 */
	TextMessage,
	PermissionDenied,
	ACL,
	QueryUsers,
	CryptSetup,
	ContextActionAdd,
	ContextAction,
	UserList,
	VoiceTarget,
	PermissionQuery, /* 20 */
	CodecVersion,
	UserStats,
	RequestBlob,
	ServerConfig
};

#pragma pack(push, 1)
class MumblePacketHeader
{
private:
	uint16_t m_packetType;

	uint32_t m_packetLength;

public:
	inline uint16_t GetPacketType() const { return ntohs(m_packetType); }
	inline uint32_t GetPacketLength() const { return ntohl(m_packetLength); }

	inline void SetPacketType(uint16_t packetType) { m_packetType = htons(packetType); }
	inline void SetPacketLength(uint32_t packetLength) { m_packetLength = htonl(packetLength); }
};
#pragma pack(pop)

#include "Mumble.pb.h"