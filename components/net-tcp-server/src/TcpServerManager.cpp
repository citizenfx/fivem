/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "TcpServerManager.h"

namespace net
{
TcpServerManager::TcpServerManager()
{
	UvLoopHolder::UvLoopHolder();
}

TcpServerManager::~TcpServerManager()
{
	UvLoopHolder::~UvLoopHolder();
}

fwRefContainer<TcpServer> TcpServerManager::CreateServer(const PeerAddress& bindAddress)
{
	// allocate an owning pointer for the server handle
	std::unique_ptr<uv_tcp_t> serverHandle = std::make_unique<uv_tcp_t>();

	// clear and associate the server handle with our loop
	uv_tcp_init(GetLoop(), serverHandle.get());

	// set the socket binding to the peer address
	uv_tcp_bind(serverHandle.get(), bindAddress.GetSocketAddress(), 0);

	// create a server instance and associate it with the handle
	fwRefContainer<UvTcpServer> tcpServer = new UvTcpServer(this);
	serverHandle->data = tcpServer.GetRef();

	// attempt listening on the socket
	if (tcpServer->Listen(std::move(serverHandle)))
	{
		// insert to the owned list
		m_servers.insert(tcpServer);
	}
	else
	{
		tcpServer = nullptr;
	}

	return tcpServer;
}
}

static InitFunction initFunction([] ()
{
	{
		fwRefContainer<net::TcpServerManager> sm = new net::TcpServerManager();
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

		std::this_thread::sleep_for(std::chrono::seconds(45));
	}
});