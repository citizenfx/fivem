/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ServerInstanceBase.h>

#include <MultiplexTcpServer.h>
#include <TcpServerManager.h>

namespace fx
{
	class TcpListenManager
	{
	private:
		fwRefContainer<net::TcpServerManager> m_tcpStack;

		std::vector<fwRefContainer<net::MultiplexTcpServer>> m_multiplexServers;

	public:
		TcpListenManager(const boost::property_tree::ptree& pt);
	};

	TcpListenManager::TcpListenManager(const boost::property_tree::ptree& pt)
	{
		// initialize a TCP stack
		m_tcpStack = new net::TcpServerManager();

		// for each defined endpoint
		for (auto& child : pt.get_child("server.endpoints"))
		{
			// parse the endpoint to a peer address
			boost::optional<net::PeerAddress> peerAddress = net::PeerAddress::FromString(child.second.get_value<std::string>());

			// if a peer address is set
			if (peerAddress.is_initialized())
			{
				// create a multiplexable TCP server and bind it
				fwRefContainer<net::MultiplexTcpServer> server = new net::MultiplexTcpServer(m_tcpStack);
				server->Bind(peerAddress.get());

				// add the server to the list
				m_multiplexServers.push_back(server);
			}
		}
	}
}

static InitFunction initFunction([] ()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([] (fx::ServerInstanceBase* instance)
	{
		instance->OnReadConfiguration.Connect([] (const boost::property_tree::ptree& pt)
		{
			// TODO: someone will need to clean these up?!
			new fx::TcpListenManager(pt);
		});
	});
});