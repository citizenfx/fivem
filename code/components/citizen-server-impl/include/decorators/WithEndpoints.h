#pragma once

namespace fx
{
	namespace ServerDecorators
	{
		const fwRefContainer<fx::GameServer>& WithEndPoints(const fwRefContainer<fx::GameServer>& server)
		{
			static std::shared_ptr<ConsoleCommand> cmd;

			server->OnAttached.Connect([=](fx::ServerInstanceBase* instance)
			{
				cmd = std::move(instance->AddCommand("endpoint_add_udp", [=](const std::string& endPoint)
				{
					// parse the endpoint to a peer address
					boost::optional<net::PeerAddress> peerAddress = net::PeerAddress::FromString(endPoint);

					// if a peer address is set
					if (peerAddress.is_initialized())
					{
						server->CreateUdpHost(*peerAddress);
					}
				}));
			});

			return server;
		}
	}
}
