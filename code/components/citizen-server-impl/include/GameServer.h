#pragma once

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

	class GameServer : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	public:
		GameServer();

		virtual ~GameServer() override;

		virtual void AttachToObject(ServerInstanceBase* instance) override;

		virtual void SendOutOfBand(const AddressPair& to, const std::string_view& oob);

		void ProcessHost(ENetHost* host);

		void ProcessServerFrame(int frameTime);

		inline void SetRunLoop(const std::function<void()>& runLoop)
		{
			m_runLoop = runLoop;
		}

	private:
		void Run();

	public:
		using THostPtr = std::unique_ptr<ENetHost, enet_deleter<&enet_host_destroy>>;

		std::vector<THostPtr> hosts;

		fwEvent<> OnHostsRegistered;

		fwEvent<fx::ServerInstanceBase*> OnAttached;

	private:
		std::thread m_thread;

		std::function<void()> m_runLoop;

		uint64_t m_residualTime;

		uint64_t m_serverTime;
	};
}

DECLARE_INSTANCE_TYPE(fx::GameServer);