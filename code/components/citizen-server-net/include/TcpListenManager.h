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

		std::unique_ptr<ConsoleCommand> m_addEndpointCommand;

	public:
		TcpListenManager();

		void Initialize();

		void AddEndpoint(const std::string& endPoint);

		virtual void AttachToObject(ServerInstanceBase* instance) override;

	public:
		fwEvent<fwRefContainer<net::MultiplexTcpServer>> OnInitializeMultiplexServer;
	};
}

DECLARE_INSTANCE_TYPE(fx::TcpListenManager);
