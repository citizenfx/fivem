#pragma once

#include <CoreConsole.h>

#include <se/Security.h>

#include <ClientRegistry.h>

#include <MapComponent.h>
#include <NetAddress.h>

#include <nng/nng.h>

#include <boost/bimap.hpp>

#include <tbb/concurrent_queue.h>
#include <concurrentqueue.h>

#include <ServerTime.h>

#include <GameServerNet.h>

#include <UvLoopHolder.h>
#include <UvTcpServer.h>

#include <GameNames.h>

namespace fx
{
	class GameServer : public fwRefCountable, public IAttached<ServerInstanceBase>, public ComponentHolderImpl<GameServer>
	{
	public:
		GameServer();

		virtual ~GameServer() override;

		virtual void AttachToObject(ServerInstanceBase* instance) override;

		void SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob, bool prefix = true);

		void ProcessServerFrame(int frameTime);

		void Broadcast(const net::Buffer& buffer);

		std::string GetVariable(const std::string& key);

		void DropClientv(const std::shared_ptr<Client>& client, const std::string& reason, fmt::printf_args args);

		template<typename... TArgs>
		inline void DropClient(const std::shared_ptr<Client>& client, const std::string& reason, const TArgs&... args)
		{
			DropClientv(client, reason, fmt::make_printf_args(args...));
		}

		void ForceHeartbeat();

		void ForceHeartbeatSoon();

		void DeferCall(int inMsec, const std::function<void()>& fn);

		void CreateUdpHost(const net::PeerAddress& address);

		void AddRawInterceptor(const std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)>& interceptor);

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

		inline int GetNetLibVersion()
		{
			return m_net->GetClientVersion();
		}

		inline auto GetIpOverrideVar()
		{
			return m_listingIpOverride;
		}

	public:
		struct CallbackListBase
		{
			inline CallbackListBase()
				: threadId(std::this_thread::get_id())
			{

			}

			virtual ~CallbackListBase() = default;

			inline void AttachToThread()
			{
				threadId = std::this_thread::get_id();
			}

			void Add(const std::function<void()>& fn, bool force = false);

			void Run();

		protected:
			virtual void SignalThread() = 0;

		private:
			moodycamel::ConcurrentQueue<std::function<void()>> callbacks;

			std::thread::id threadId;
		};

		struct CallbackListNng : public CallbackListBase
		{
			inline CallbackListNng(const std::string& socketName, int socketIdx)
				: m_socketName(socketName), m_socketIdx(socketIdx)
			{

			}

		protected:
			virtual void SignalThread() override;

		private:
			std::string m_socketName;

			int m_socketIdx;
		};

		struct CallbackListUv : public CallbackListBase
		{
			inline CallbackListUv(std::weak_ptr<std::unique_ptr<UvHandleContainer<uv_async_t>>> async)
				: m_async(async)
			{

			}

		protected:
			virtual void SignalThread() override;

		private:
			std::weak_ptr<std::unique_ptr<UvHandleContainer<uv_async_t>>> m_async;
		};

		inline void InternalAddMainThreadCb(const std::function<void()>& fn, bool force = false)
		{
			m_mainThreadCallbacks->Add(fn, force);
		}

		inline void InternalAddNetThreadCb(const std::function<void()>& fn)
		{
			m_netThreadCallbacks->Add(fn);
		}

		inline void InternalAddSyncThreadCb(const std::function<void()>& fn)
		{
			m_syncThreadCallbacks->Add(fn);
		}

		fwRefContainer<NetPeerBase> InternalGetPeer(int peerId);

		void InternalResetPeer(int peerId);

		void InternalSendPacket(const std::shared_ptr<fx::Client>& client, int peer, int channel, const net::Buffer& buffer, NetPacketType type);

		void InternalRunMainThreadCbs(nng_socket socket);

	private:
		void Run();

	public:
		void ProcessPacket(const fwRefContainer<NetPeerBase>& peer, const uint8_t* data, size_t size);

	public:
		using TPacketHandler = std::function<void(uint32_t packetId, const std::shared_ptr<Client>& client, net::Buffer& packet)>;

		inline void SetPacketHandler(const TPacketHandler& handler)
		{
			m_packetHandler = handler;
		}

		inline fx::GameName GetGameName()
		{
			return m_gamename->GetValue();
		}

		inline bool HasSettled() const
		{
			return m_hasSettled;
		}

		inline bool UseAccurateSends()
		{
			return m_useAccurateSendsVar->GetValue();
		}

	private:
		void InitializeSyncUv();

		void InitializeNetUv();

		void InitializeNetThread();

	public:
		fwEvent<> OnTick;

		fwEvent<> OnNetworkTick;

		fwEvent<> OnSyncTick;

		fwEvent<fx::ServerInstanceBase*> OnAttached;

	private:
		std::thread m_thread;

		TPacketHandler m_packetHandler;

		bool m_hasSettled;

		std::function<void()> m_runLoop;

		uint64_t m_residualTime;

		uint64_t m_serverTime;

		ClientRegistry* m_clientRegistry;

		ServerInstanceBase* m_instance;

		std::shared_ptr<ConsoleCommand> m_heartbeatCommand;

		std::shared_ptr<ConVar<std::string>> m_listingIpOverride;

		std::shared_ptr<ConVar<std::string>> m_listingHostOverride;

		std::shared_ptr<ConVar<bool>> m_forceIndirectListing;

		std::shared_ptr<ConVar<bool>> m_useAccurateSendsVar;

		std::shared_ptr<ConVar<std::string>> m_rconPassword;

		std::shared_ptr<ConVar<std::string>> m_hostname;

		std::shared_ptr<ConVar<GameName>> m_gamename;

		std::shared_ptr<ConVar<std::string>> m_masters[3];

		std::string m_lastGameName;

		tbb::concurrent_unordered_map<std::string, net::PeerAddress> m_masterCache;

		fwRefContainer<se::Context> m_seContext;

		std::chrono::milliseconds m_nextHeartbeatTime;

		tbb::concurrent_unordered_map<int, std::tuple<int, std::function<void()>>> m_deferCallbacks;

		fwRefContainer<GameServerNetBase> m_net;

		fwRefContainer<net::UvLoopHolder> m_syncThreadLoop;

		fwRefContainer<net::UvLoopHolder> m_netThreadLoop;

		fwRefContainer<net::UvLoopHolder> m_mainThreadLoop;

		std::any m_syncThreadData;

		std::any m_netThreadData;

		std::any m_mainThreadData;

		std::function<bool(const uint8_t *, size_t, const net::PeerAddress &)> m_interceptor;

		std::unique_ptr<CallbackListBase> m_mainThreadCallbacks;

		std::unique_ptr<CallbackListBase> m_netThreadCallbacks;

		std::unique_ptr<CallbackListBase> m_syncThreadCallbacks;

		// only touched on network thread(!)
		std::list<std::tuple<int, int, net::Buffer, NetPacketType>> m_netSendList;
	};

	using TPacketTypeHandler = std::function<void(const std::shared_ptr<Client>& client, net::Buffer& packet)>;

	enum class ThreadIdx
	{
		Main,
		Net,
		Sync
	};

	class HandlerMapComponent : public MapComponent<uint32_t, std::tuple<ThreadIdx, TPacketTypeHandler>>
	{
	public:
		inline void Add(const uint32_t& key, const std::tuple<ThreadIdx, TPacketTypeHandler>& value)
		{
			MapComponent::Add(key, value);
		}

		inline void Add(const uint32_t& key, std::tuple<ThreadIdx, TPacketTypeHandler>&& value)
		{
			MapComponent::Add(key, std::move(value));
		}

		inline void Add(const uint32_t& key, const TPacketTypeHandler& value)
		{
			MapComponent::Add(key, std::make_tuple(ThreadIdx::Main, value));
		}

		inline void Add(const uint32_t& key, TPacketTypeHandler&& value)
		{
			MapComponent::Add(key, std::make_tuple(ThreadIdx::Main, std::move(value)));
		}
	};

	bool IsBigMode();
}

DECLARE_INSTANCE_TYPE(fx::GameServer);
DECLARE_INSTANCE_TYPE(fx::HandlerMapComponent);
