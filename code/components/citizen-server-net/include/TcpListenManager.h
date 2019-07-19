#pragma once

#include <MultiplexTcpServer.h>
#include <TcpServerManager.h>

#include <CoreConsole.h>

#include <ServerInstanceBase.h>
#include <ComponentHolder.h>

namespace fx
{
	class TcpListenManager : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	private:
		fwRefContainer<net::TcpServerManager> m_tcpStack;

		std::vector<fwRefContainer<net::MultiplexTcpServer>> m_multiplexServers;

		std::shared_ptr<ConsoleCommand> m_addEndpointCommand;

		int m_primaryPort;

	public:
		TcpListenManager();

		void Initialize();

		void AddEndpoint(const std::string& endPoint);

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
