#pragma once

#include <FixedBuffer.h>

namespace net
{
	class Buffer;
	class PeerAddress;
}

enum NetPacketType
{
	NetPacketType_Unreliable,
	NetPacketType_Reliable
};

namespace fx
{
	class GameServer;

	enum ENetPeerStatistics : uint8_t
	{
		// PacketLoss will only update once every 10 seconds, use PacketLossEpoch if you want the time
		// since the last time the packet loss was updated.

		// the amount of packet loss the player has, needs to be scaled with PACKET_LOSS_SCALE
		PacketLoss = 0,
		// The variance in the packet loss
		PacketLossVariance = 1,
		// The time since the last packet update in ms, relative to the peers connection time
		PacketLossEpoch = 2,
		// The mean amount of time it takes for a packet to get to the client (ping)
		RoundTripTime = 3,
		// The variance in the round trip time
		RoundTripTimeVariance = 4,
		// Despite their name, these are only updated once every 5 seconds, you can get the last time this was updated with PacketThrottleEpoch
		// The last recorded round trip time of a packet
		LastRoundTripTime = 5,
		// The last round trip time variance
		LastRoundTripTimeVariance = 6,
		// The time since the last packet throttle update, relative to the peers connection time
		PacketThrottleEpoch = 7,

		MAX
	};

	class NetPeerBase : public fwRefCountable
	{
	public:
		virtual int GetId() = 0;

		virtual int GetPing() = 0;

		virtual uint32_t GetENetStatistics(ENetPeerStatistics statisticType) = 0;

		virtual net::PeerAddress GetAddress() = 0;

		virtual void OnSendConnectOK() = 0;
	};
	using NetPeerStackBuffer = FixedBuffer<NetPeerBase, 32>;

	class GameServerNetBase : public fwRefCountable
	{
	public:
		struct OutgoingCommandInfo
		{
			uint32_t type;
			uint32_t timeAgo;
			size_t size;
			// if type is msgNetEvent this will be the event name
			// if the type is a known packet it will be the packet name, i.e. msgStateBag
			std::string eventName;
		};

		struct TimeoutInfo
		{
			std::vector<OutgoingCommandInfo> bigCommandList;
			size_t pendingCommands = 0;
		};

		virtual void Process() = 0;

		virtual void Select(const std::vector<uintptr_t>& fds, int timeout) = 0;

		virtual void GetPeer(int id, NetPeerStackBuffer& buffer) = 0;

		virtual void ResetPeer(int id) = 0;

		virtual void SendPacket(int peer, int channel, const net::Buffer& buffer, NetPacketType type) = 0;

		virtual void SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob, bool prefix = true) = 0;

		virtual void CreateUdpHost(const net::PeerAddress& address) = 0;

		virtual void AddRawInterceptor(const std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)>& interceptor) = 0;

		virtual int GetClientVersion() = 0;

		virtual TimeoutInfo GatherTimeoutInfo(int peer) = 0;

		virtual bool SupportsUvUdp()
		{
			return false;
		}
	};

	fwRefContainer<GameServerNetBase> CreateGSNet(fx::GameServer* server);
	fwRefContainer<GameServerNetBase> CreateGSNet_ENet(fx::GameServer* server);
}
