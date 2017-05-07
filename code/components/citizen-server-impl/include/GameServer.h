#pragma once

#include <ClientRegistry.h>

#include <MapComponent.h>
#include <NetAddress.h>

#include <enet/enet.h>

namespace fx
{
	template<void* Fn>
	struct enet_deleter
	{
		template<typename T>
		void operator()(T* data)
		{
			((void(*)(T*))Fn)(data);
		}
	};

	ENetAddress GetENetAddress(const net::PeerAddress& peerAddress);

	net::PeerAddress GetPeerAddress(const ENetAddress& enetAddress);

	using AddressPair = std::tuple<ENetHost*, net::PeerAddress>;

	class GameServer : public fwRefCountable, public IAttached<ServerInstanceBase>, public ComponentHolderImpl<GameServer>
	{
	public:
		GameServer();

		virtual ~GameServer() override;

		virtual void AttachToObject(ServerInstanceBase* instance) override;

		virtual void SendOutOfBand(const AddressPair& to, const std::string_view& oob);

		void ProcessHost(ENetHost* host);

		void ProcessServerFrame(int frameTime);

		void Broadcast(const net::Buffer& buffer);

		inline void SetRunLoop(const std::function<void()>& runLoop)
		{
			m_runLoop = runLoop;
		}

		inline ServerInstanceBase* GetInstance()
		{
			return m_instance;
		}

	private:
		void Run();

		void ProcessPacket(ENetPeer* peer, const uint8_t* data, size_t size);

	public:
		using TPacketHandler = std::function<void(uint32_t packetId, const std::shared_ptr<Client>& client, net::Buffer& packet)>;

		inline void SetPacketHandler(const TPacketHandler& handler)
		{
			m_packetHandler = handler;
		}

	public:
		using THostPtr = std::unique_ptr<ENetHost, enet_deleter<&enet_host_destroy>>;

		std::vector<THostPtr> hosts;

		fwEvent<> OnHostsRegistered;

		fwEvent<> OnTick;

		fwEvent<fx::ServerInstanceBase*> OnAttached;

	private:
		std::thread m_thread;

		TPacketHandler m_packetHandler;

		std::function<void()> m_runLoop;

		uint64_t m_residualTime;

		uint64_t m_serverTime;

		ClientRegistry* m_clientRegistry;

		ServerInstanceBase* m_instance;
	};

	using TPacketTypeHandler = std::function<void(const std::shared_ptr<Client>& client, net::Buffer& packet)>;
	using HandlerMapComponent = MapComponent<uint32_t, TPacketTypeHandler>;
}

DECLARE_INSTANCE_TYPE(fx::GameServer);
DECLARE_INSTANCE_TYPE(fx::HandlerMapComponent);