/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "TcpServerManager.h"
#include "UvLoopManager.h"
#include "memdbgon.h"

namespace net
{
TcpServerManager::TcpServerManager(const std::string& loopName)
{
	m_uvLoop = Instance<UvLoopManager>::Get()->GetOrCreate(loopName);
}

TcpServerManager::~TcpServerManager()
{
	
}

fwRefContainer<TcpServer> TcpServerManager::CreateServer(const PeerAddress& bindAddress)
{
	// allocate an owning pointer for the server handle
	auto serverHandle = m_uvLoop->Get()->resource<uvw::TCPHandle>();

	// set the socket binding to the peer address
	serverHandle->bind(*bindAddress.GetSocketAddress());

	// create a server instance and associate it with the handle
	fwRefContainer<UvTcpServer> tcpServer = new UvTcpServer(this);

	// attempt listening on the socket
	if (tcpServer->Listen(std::move(serverHandle)))
	{
		// insert to the owned list
		m_servers.insert(tcpServer);
	}
	else
	{
		tcpServer = nullptr;
		trace("net-tcp-server failed to create server: couldn't listen");
	}

	return tcpServer;
}
}

#if 0
#include "MultiplexTcpServer.h"

static InitFunction initFunction([] ()
{
	{
		/*fwRefContainer<net::TcpServerManager> sm = new net::TcpServerManager();
		fwRefContainer<net::TcpServer> server = sm->CreateServer(net::PeerAddress::FromString("localhost:30150").get());
		fwRefContainer<net::TcpServer> server6 = sm->CreateServer(net::PeerAddress::FromString("[::]:30150").get());

		server->SetConnectionCallback([=] (fwRefContainer<net::TcpServerStream> stream)
		{
			trace("Received a connection from %s.\n", stream->GetPeerAddress().ToString().c_str());

			stream->SetReadCallback([=] (const std::vector<uint8_t>& buf)
			{
				for (auto& entry : buf)
				{
					trace("%c", entry);
				}
			});
		});

		server6->SetConnectionCallback([=] (fwRefContainer<net::TcpServerStream> stream)
		{
			trace("Received a connection from %s.\n", stream->GetPeerAddress().ToString().c_str());
		});

		std::this_thread::sleep_for(std::chrono::seconds(45));*/

		fwRefContainer<net::TcpServerManager> sm = new net::TcpServerManager();
		fwRefContainer<net::MultiplexTcpServer> ms = new net::MultiplexTcpServer(sm);
		ms->Bind(net::PeerAddress::FromString("0.0.0.0:30150").get());

		fwRefContainer<net::TcpServer> cakeServer = ms->CreateServer([] (const std::vector<uint8_t>& data)
		{
			if (data.size() < 4)
			{
				return net::MultiplexPatternMatchResult::InsufficientData;
			}

			return (memcmp(&data[0], "cake", 4) == 0) ? net::MultiplexPatternMatchResult::Match : net::MultiplexPatternMatchResult::NoMatch;
		});

		fwRefContainer<net::TcpServer> bakeServer = ms->CreateServer([] (const std::vector<uint8_t>& data)
		{
			if (data.size() < 4)
			{
				return net::MultiplexPatternMatchResult::InsufficientData;
			}

			return (memcmp(&data[0], "bake", 4) == 0) ? net::MultiplexPatternMatchResult::Match : net::MultiplexPatternMatchResult::NoMatch;
		});

		cakeServer->SetConnectionCallback([=] (fwRefContainer<net::TcpServerStream> stream)
		{
			trace("Received a cake connection from %s.\n", stream->GetPeerAddress().ToString().c_str());

			stream->SetReadCallback([=] (const std::vector<uint8_t>& buf)
			{
				trace("[cake]: ");

				for (auto& entry : buf)
				{
					trace("%c", entry);
				}

				trace("\n");
			});
		});

		bakeServer->SetConnectionCallback([=] (fwRefContainer<net::TcpServerStream> stream)
		{
			trace("Received a bake connection from %s.\n", stream->GetPeerAddress().ToString().c_str());

			stream->SetReadCallback([=] (const std::vector<uint8_t>& buf)
			{
				trace("[bake]: ");

				for (auto& entry : buf)
				{
					trace("%c", entry);
				}

				trace("\n");

				stream->Write(buf);
			});
		});

		//std::this_thread::sleep_for(std::chrono::seconds(3600));
	}
});
#endif
