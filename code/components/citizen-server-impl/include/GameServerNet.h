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
	NetPacketType_Reliable,
	NetPacketType_ReliableReplayed
};

namespace fx
{
	class GameServer;
	class NetPeerBase : public fwRefCountable
	{
	public:
		virtual int GetId() = 0;

		virtual int GetPing() = 0;

		virtual int GetPingVariance() = 0;

		virtual net::PeerAddress GetAddress() = 0;

		virtual void OnSendConnectOK() = 0;
	};
	using NetPeerStackBuffer = FixedBuffer<NetPeerBase, 32>;

	class GameServerNetBase : public fwRefCountable
	{
	public:
		virtual void Process() = 0;

		virtual void Select(const std::vector<uintptr_t>& fds, int timeout) = 0;

		virtual void GetPeer(int id, NetPeerStackBuffer& buffer) = 0;

		virtual void ResetPeer(int id) = 0;

		virtual void SendPacket(int peer, int channel, const net::Buffer& buffer, NetPacketType type) = 0;

		virtual void SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob, bool prefix = true) = 0;

		virtual void CreateUdpHost(const net::PeerAddress& address) = 0;

		virtual void AddRawInterceptor(const std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)>& interceptor) = 0;

		virtual int GetClientVersion() = 0;

		virtual bool SupportsUvUdp()
		{
			return false;
		}
	};

	fwRefContainer<GameServerNetBase> CreateGSNet(fx::GameServer* server);
	fwRefContainer<GameServerNetBase> CreateGSNet_ENet(fx::GameServer* server);
}
