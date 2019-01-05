#pragma once

#include <NetAddress.h>
#include <yojimbo.h>

namespace fx
{
	yojimbo::Address GetYojimboAddress(const net::PeerAddress& peerAddress)
	{
		auto sa = peerAddress.GetSocketAddress();

		if (sa->sa_family == AF_INET)
		{
			auto in4 = (sockaddr_in*)sa;

			uint8_t addr[4];
			memcpy(addr, &in4->sin_addr.s_addr, 4);

			return yojimbo::Address{ addr, ntohs(in4->sin_port) };
		}
		else if (sa->sa_family == AF_INET6)
		{
			auto in6 = (sockaddr_in6*)sa;

			uint16_t addr[8];

			for (int i = 0; i < 8; ++i)
			{
				addr[i] = htons(((uint16_t*)&in6->sin6_addr)[i]);
			}

			return yojimbo::Address{ addr, ntohs(in6->sin6_port) };
		}

		return yojimbo::Address{};
	}

	net::PeerAddress GetPeerAddress(const yojimbo::Address& yjAddress)
	{
		if (yjAddress.GetType() == yojimbo::ADDRESS_IPV4)
		{
			sockaddr_in in = { 0 };
			in.sin_family = AF_INET;
			memcpy(&in.sin_addr.s_addr, yjAddress.GetAddress4(), 4);
			in.sin_port = htons(yjAddress.GetPort());

			return net::PeerAddress{ (sockaddr*)&in, sizeof(in) };
		}
		else if (yjAddress.GetType() == yojimbo::ADDRESS_IPV6)
		{
			sockaddr_in6 in6 = { 0 };
			in6.sin6_family = AF_INET6;

			auto addr = yjAddress.GetAddress6();

			for (int i = 0; i < 8; ++i)
			{
				((uint16_t*)&in6.sin6_addr)[i] = ntohs(addr[i]);
			}

			in6.sin6_port = htons(yjAddress.GetPort());

			return net::PeerAddress{ (sockaddr*)&in6, sizeof(in6) };
		}
		else
		{
			return net::PeerAddress{};
		}
	}

	class NetConnectionConfig : public yojimbo::ClientServerConfig
	{
	public:
		NetConnectionConfig()
		{
			// Cfx exposes 2 channels, we need to distinguish between reliable and unreliable using this
			numChannels = 4;
			channel[0].type = yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
			channel[1].type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
			channel[2].type = yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
			channel[3].type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;

			timeout = 15;

#ifdef _DEBUG
			timeout = -1;
#endif
		}
	};

	class NetCfxBaseMessage
	{
	public:
		virtual const uint8_t* GetData() = 0;

		virtual size_t GetDataLength() = 0;
	};

	class NetCfxMessage : public yojimbo::Message, public NetCfxBaseMessage
	{
	public:
		int m_length;
		uint8_t m_inlineData[1024];

		NetCfxMessage() :
			m_length(0) {}

		template <typename Stream>
		bool Serialize(Stream& stream) {
			serialize_int(stream, m_length, 0, 1024);
			serialize_bytes(stream, m_inlineData, m_length);

			return true;
		}

		inline void SetData(const uint8_t* data, size_t size)
		{
			m_length = size;
			memcpy(m_inlineData, data, size);
		}

		inline const uint8_t* GetData()
		{
			return m_inlineData;
		}

		inline size_t GetDataLength()
		{
			return m_length;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	class NetCfxBlockMessage : public yojimbo::BlockMessage, public NetCfxBaseMessage
	{
	public:
		NetCfxBlockMessage() {}

		template <typename Stream>
		bool Serialize(Stream& stream) {
			return true;
		}

		inline const uint8_t* GetData()
		{
			return GetBlockData();
		}

		inline size_t GetDataLength()
		{
			return GetBlockSize();
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	YOJIMBO_MESSAGE_FACTORY_START(NetMessageFactory, 2);
	YOJIMBO_DECLARE_MESSAGE_TYPE(0, NetCfxMessage);
	YOJIMBO_DECLARE_MESSAGE_TYPE(1, NetCfxBlockMessage);
	YOJIMBO_MESSAGE_FACTORY_FINISH();

	class NetAdapter : public yojimbo::Adapter
	{
	public:
		yojimbo::MessageFactory* CreateMessageFactory(yojimbo::Allocator& allocator) override
		{
			return YOJIMBO_NEW(allocator, NetMessageFactory, allocator);
		}

		void OnServerClientConnected(int clientIndex) override
		{
			OnClientConnected(clientIndex);
		}

		void OnServerClientDisconnected(int clientIndex) override
		{
			OnClientDisconnected(clientIndex);
		}

		fwEvent<int> OnClientConnected;
		fwEvent<int> OnClientDisconnected;
	};
}
