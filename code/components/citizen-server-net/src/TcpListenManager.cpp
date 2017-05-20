/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ServerInstanceBase.h>

#include <TcpListenManager.h>

#include <CoreConsole.h>

namespace fx
{
	TcpListenManager::TcpListenManager()
	{
		Initialize();
	}

	void TcpListenManager::Initialize()
	{
		// initialize a TCP stack
		m_tcpStack = new net::TcpServerManager();
	}

	void TcpListenManager::AddEndpoint(const std::string& endPoint)
	{
		// parse the endpoint to a peer address
		boost::optional<net::PeerAddress> peerAddress = net::PeerAddress::FromString(endPoint);

		// if a peer address is set
		if (peerAddress.is_initialized())
		{
			// create a multiplexable TCP server and bind it
			fwRefContainer<net::MultiplexTcpServer> server = new net::MultiplexTcpServer(m_tcpStack);
			server->Bind(peerAddress.get());

			// add the server to the list
			m_multiplexServers.push_back(server);

			// trigger event
			OnInitializeMultiplexServer(server);
		}
	}

	void TcpListenManager::AttachToObject(ServerInstanceBase* instance)
	{
		instance->SetComponent(m_tcpStack);

		m_addEndpointCommand = instance->AddCommand("endpoint_add_tcp", [=](const std::string& endPoint)
		{
			AddEndpoint(endPoint);
		});
	}
}

static InitFunction initFunction([] ()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([] (fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::TcpListenManager());
	}, -1000);
});
