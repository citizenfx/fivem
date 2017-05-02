#pragma once

namespace fx
{
	namespace ServerDecorators
	{
		const fwRefContainer<fx::GameServer>& WithEndPoints(const fwRefContainer<fx::GameServer>& server)
		{
			server->OnAttached.Connect([=](fx::ServerInstanceBase* instance)
			{
				instance->OnReadConfiguration.Connect([=](const boost::property_tree::ptree& pt)
				{
					// for each defined endpoint
					for (auto& child : pt.get_child("server.endpoints"))
					{
						// parse the endpoint to a peer address
						boost::optional<net::PeerAddress> peerAddress = net::PeerAddress::FromString(child.second.get_value<std::string>());

						// if a peer address is set
						if (peerAddress.is_initialized())
						{
							// create an ENet host
							ENetAddress addr = GetENetAddress(*peerAddress);
							ENetHost* host = enet_host_create(&addr, 64, 2, 0, 0);

							// ensure the host exists
							assert(host);

							// register the global host
							g_hostInstances[host] = server.GetRef();

							server->hosts.push_back(fx::GameServer::THostPtr{ host });
						}
					}

					server->OnHostsRegistered();
				});
			});

			return server;
		}
	}
}