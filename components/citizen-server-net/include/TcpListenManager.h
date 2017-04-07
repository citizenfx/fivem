#pragma once

#include <MultiplexTcpServer.h>
#include <TcpServerManager.h>

namespace fx
{
	class TcpListenManager : public fwRefCountable
	{
	private:
		fwRefContainer<net::TcpServerManager> m_tcpStack;

		std::vector<fwRefContainer<net::MultiplexTcpServer>> m_multiplexServers;

	public:
		TcpListenManager();

		void Initialize(const boost::property_tree::ptree& pt);

	public:
		fwEvent<fwRefContainer<net::MultiplexTcpServer>> OnInitializeMultiplexServer;
	};
}

DECLARE_INSTANCE_TYPE(fx::TcpListenManager);