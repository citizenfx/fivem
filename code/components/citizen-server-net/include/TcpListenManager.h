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
	class CSNET_EXPORT TcpListenManager : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	private:
		fwRefContainer<net::TcpServerManager> m_tcpStack;

		std::vector<fwRefContainer<net::MultiplexTcpServer>> m_multiplexServers;

		std::vector<fwRefContainer<net::TcpServer>> m_externalServers;

		std::shared_ptr<ConsoleCommand> m_addEndpointCommand;

		std::shared_ptr<ConVar<int>> m_primaryPortVar;

		int m_primaryPort;

	public:
		TcpListenManager(const std::string& loopName = "default");

		void Initialize(const std::string& loopName = "default");

		void AddEndpoint(const std::string& endPoint);

		virtual void AddExternalServer(const fwRefContainer<net::TcpServer>& server);

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
