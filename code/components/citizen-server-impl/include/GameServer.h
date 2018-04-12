#pragma once

#include <CoreConsole.h>

#include <se/Security.h>

#include <ClientRegistry.h>

#include <MapComponent.h>
#include <NetAddress.h>

#include <nng.h>

#include <boost/bimap.hpp>

#include <enet/enet.h>

#include <tbb/concurrent_queue.h>

inline std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

namespace fx
{
	struct enet_host_deleter
	{
		inline void operator()(ENetHost* data)
		{
			enet_host_destroy(data);
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

		std::string GetVariable(const std::string& key);

		void DropClient(const std::shared_ptr<Client>& client, const std::string& reason, const fmt::ArgList& args);

		FMT_VARIADIC(void, DropClient, const std::shared_ptr<Client>&, const std::string&);

		void ForceHeartbeat();

		void DeferCall(int inMsec, const std::function<void()>& fn);

		inline void SetRunLoop(const std::function<void()>& runLoop)
		{
			m_runLoop = runLoop;
		}

		inline ServerInstanceBase* GetInstance()
		{
			return m_instance;
		}

		inline std::string GetRconPassword()
		{
			return m_rconPassword->GetValue();
		}

	public:
		struct CallbackList
		{
			inline CallbackList(const std::string& socketName, int socketIdx)
				: m_socketName(socketName), m_socketIdx(socketIdx)
			{

			}

			void Add(const std::function<void()>& fn);

			void Run();

		private:
			tbb::concurrent_queue<std::function<void()>> callbacks;

			std::string m_socketName;

			int m_socketIdx;
		};

		inline void InternalAddMainThreadCb(const std::function<void()>& fn)
		{
			m_mainThreadCallbacks.Add(fn);
		}

		inline void InternalAddNetThreadCb(const std::function<void()>& fn)
		{
			m_netThreadCallbacks.Add(fn);
		}

		const ENetPeer* InternalGetPeer(int peerId);

		void InternalResetPeer(int peerId);

		void InternalSendPacket(int peer, int channel, const net::Buffer& buffer, ENetPacketFlag flags);

		void InternalRunMainThreadCbs(nng_socket socket);

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
		using THostPtr = std::unique_ptr<ENetHost, enet_host_deleter>;

		std::vector<THostPtr> hosts;

		fwEvent<ENetHost*> OnHostRegistered;

		fwEvent<> OnTick;

		fwEvent<fx::ServerInstanceBase*> OnAttached;

	private:
		std::thread m_thread;

		TPacketHandler m_packetHandler;

		std::function<void()> m_runLoop;

		uint64_t m_residualTime;

		uint64_t m_serverTime;

		int m_basePeerId;

		ClientRegistry* m_clientRegistry;

		ServerInstanceBase* m_instance;

		std::shared_ptr<ConsoleCommand> m_heartbeatCommand;

		std::shared_ptr<ConVar<std::string>> m_rconPassword;

		std::shared_ptr<ConVar<std::string>> m_hostname;

		std::shared_ptr<ConVar<std::string>> m_masters[3];

		tbb::concurrent_unordered_map<std::string, net::PeerAddress> m_masterCache;

		fwRefContainer<se::Context> m_seContext;

		int64_t m_nextHeartbeatTime;

		tbb::concurrent_unordered_map<int, std::tuple<int, std::function<void()>>> m_deferCallbacks;

		boost::bimap<int, ENetPeer*> m_peerHandles;

		CallbackList m_mainThreadCallbacks{ "inproc://main_client", 0 };

		CallbackList m_netThreadCallbacks{ "inproc://netlib_client", 1 };
	};

	using TPacketTypeHandler = std::function<void(const std::shared_ptr<Client>& client, net::Buffer& packet)>;
	using HandlerMapComponent = MapComponent<uint32_t, TPacketTypeHandler>;
}

DECLARE_INSTANCE_TYPE(fx::GameServer);
DECLARE_INSTANCE_TYPE(fx::HandlerMapComponent);
