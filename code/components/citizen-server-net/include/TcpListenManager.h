#pragma once

#include <MultiplexTcpServer.h>
#include <TcpServerManager.h>

#include <CoreConsole.h>

#include <ServerInstanceBase.h>
#include <ComponentHolder.h>

#ifdef COMPILING_CITIZEN_SERVER_NET
#define CSNET_EXPORT DLL_EXPORT
#else
#define CSNET_EXPORT DLL_IMPORT
#endif

namespace fx
{
bool CSNET_EXPORT IsProxyAddress(std::string_view ep);

bool CSNET_EXPORT IsProxyAddress(const net::PeerAddress& ep);
}

namespace tbb
{
template<std::size_t Len>
size_t tbb_hasher(const std::array<uint8_t, Len>& arr)
{
	return std::hash<std::string_view>()({ (const char*)arr.data(), Len });
}
}

#include <tbb/concurrent_unordered_map.h>

namespace fx
{
	class CSNET_EXPORT TcpListenManager : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	private:
		fwRefContainer<net::TcpServerManager> m_tcpStack;

		std::vector<fwRefContainer<net::MultiplexTcpServer>> m_multiplexServers;

		std::vector<fwRefContainer<net::TcpServer>> m_externalServers;

		std::shared_ptr<ConsoleCommand> m_addEndpointCommand;

		std::shared_ptr<ConVar<int>> m_primaryPortVar;

		std::shared_ptr<ConVar<int>> m_tcpLimitVar;

		std::shared_ptr<ConVar<bool>> m_dnsRegisterVar;

		std::shared_ptr<ConVar<uint16_t>> m_tcpConnectionTimeoutSecondsVar;

		struct HostHash
		{
			inline size_t operator()(const std::array<uint8_t, 16>& a) const
			{
				return std::hash<std::string_view>()(std::string_view{ reinterpret_cast<const char*>(a.data()), a.size() });
			}
		};

		tbb::concurrent_unordered_map<std::array<uint8_t, 16>, std::atomic<int>, HostHash> m_tcpLimitByHost;

		int m_tcpLimit = 16;

		int m_primaryPort;

		void RegisterDns();

	public:
		TcpListenManager(const std::string& loopName = "default");

		void Initialize(const std::string& loopName = "default");

		void AddEndpoint(const std::string& endPoint);

		virtual void AddExternalServer(const fwRefContainer<net::TcpServer>& server);

		void BlockPeer(const net::PeerAddress& peer);

		inline fwRefContainer<net::TcpServerManager> GetTcpStack()
		{
			return m_tcpStack;
		}

		inline int GetPrimaryPort()
		{
			return m_primaryPort;
		}

		virtual void AttachToObject(ServerInstanceBase* instance) override;

	public:
		fwEvent<fwRefContainer<net::MultiplexTcpServer>> OnInitializeMultiplexServer;
	};
}

DECLARE_INSTANCE_TYPE(fx::TcpListenManager);
