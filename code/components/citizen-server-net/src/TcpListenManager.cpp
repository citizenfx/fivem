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
	TcpListenManager::TcpListenManager(const std::string& loopName)
		: m_primaryPort(0)
	{
		Initialize(loopName);
	}

	void TcpListenManager::Initialize(const std::string& loopName)
	{
		// initialize a TCP stack
		m_tcpStack = new net::TcpServerManager(loopName);
	}

	void TcpListenManager::AddEndpoint(const std::string& endPoint)
	{
		// parse the endpoint to a peer address
		boost::optional<net::PeerAddress> peerAddress = net::PeerAddress::FromString(endPoint);

		// if a peer address is set
		if (peerAddress.is_initialized())
		{
			// if the primary port isn't set, set it
			if (m_primaryPort == 0)
			{
				m_primaryPort = peerAddress->GetPort();
				m_primaryPortVar->GetHelper()->SetRawValue(m_primaryPort);
			}

			// create a multiplexable TCP server and bind it
			fwRefContainer<net::MultiplexTcpBindServer> server = new net::MultiplexTcpBindServer(m_tcpStack);
			server->Bind(peerAddress.get());

			// add the server to the list
			m_multiplexServers.push_back(server);

			// trigger event
			OnInitializeMultiplexServer(server);
		}
	}

	void TcpListenManager::AddExternalServer(const fwRefContainer<net::TcpServer>& server)
	{
		// attach and create a multiplex
		fwRefContainer<net::MultiplexTcpServer> multiplex = new net::MultiplexTcpServer();
		multiplex->AttachToServer(server);

		// store the server
		m_externalServers.push_back(server);
		m_multiplexServers.push_back(multiplex);

		// set up the multiplex
		OnInitializeMultiplexServer(multiplex);
	}

	void TcpListenManager::AttachToObject(ServerInstanceBase* instance)
	{
		instance->SetComponent(m_tcpStack);

		m_addEndpointCommand = instance->AddCommand("endpoint_add_tcp", [=](const std::string& endPoint)
		{
			AddEndpoint(endPoint);
		});

		m_primaryPortVar = instance->AddVariable<int>("netPort", ConVar_None, m_primaryPort);
	}
}

static InitFunction initFunction([] ()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([] (fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::TcpListenManager());
	}, -1000);
});
