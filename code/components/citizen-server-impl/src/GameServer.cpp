#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <GameServer.h>

#include <NetBuffer.h>

inline static uint64_t msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

namespace fx
{
	ENetAddress GetENetAddress(const net::PeerAddress& peerAddress)
	{
		ENetAddress addr = { 0 };

		auto sa = peerAddress.GetSocketAddress();

		if (sa->sa_family == AF_INET)
		{
			auto in4 = (sockaddr_in*)sa;

			addr.host.u.Byte[10] = 0xFF;
			addr.host.u.Byte[11] = 0xFF;

			memcpy(&addr.host.u.Byte[12], &in4->sin_addr.S_un.S_un_b.s_b1, 4);
			addr.port = ntohs(in4->sin_port);
			addr.sin6_scope_id = 0;
		}
		else if (sa->sa_family == AF_INET6)
		{
			auto in6 = (sockaddr_in6*)sa;

			addr.host = in6->sin6_addr;
			addr.port = ntohs(in6->sin6_port);
			addr.sin6_scope_id = in6->sin6_scope_id;
		}

		return addr;
	}

	net::PeerAddress GetPeerAddress(const ENetAddress& enetAddress)
	{
		sockaddr_in6 in6 = { 0 };
		in6.sin6_family = AF_INET6;
		in6.sin6_addr = enetAddress.host;
		in6.sin6_port = htons(enetAddress.port);
		in6.sin6_scope_id = enetAddress.sin6_scope_id;

		return net::PeerAddress((sockaddr*)&in6, sizeof(in6));
	}

	static std::map<ENetHost*, GameServer*> g_hostInstances;

	GameServer::GameServer()
		: m_residualTime(0), m_serverTime(msec())
	{

	}

	GameServer::~GameServer()
	{
		// TODO: fix if ever multi-instancing
		m_thread.detach();
	}

	void GameServer::AttachToObject(ServerInstanceBase* instance)
	{
		OnAttached(instance);

		instance->OnReadConfiguration.Connect([=](const boost::property_tree::ptree& pt)
		{
			m_thread = std::thread([=]()
			{
				Run();
			});
		}, 100);

		m_clientRegistry = instance->GetComponent<ClientRegistry>().GetRef();
	}

	void GameServer::Run()
	{
		if (m_runLoop)
		{
			m_runLoop();
		}
	}

	std::map<std::string, std::string> ParsePOSTString(const std::string_view& postDataString);

	void GameServer::ProcessPacket(ENetPeer* peer, const uint8_t* data, size_t size)
	{
		auto peerAddr = GetPeerAddress(peer->address);

		// create a netbuffer and read the message type
		net::Buffer msg(data, size);
		uint32_t msgType = msg.Read<uint32_t>();

		// get the client
		auto client = m_clientRegistry->GetClientByPeer(peer);

		// handle connection handshake message
		if (msgType == 1)
		{
			if (!client)
			{
				std::vector<char> dataBuffer(msg.GetRemainingBytes());
				msg.Read(dataBuffer.data(), dataBuffer.size());

				auto postMap = ParsePOSTString(std::string_view(dataBuffer.data(), dataBuffer.size()));
				auto guid = postMap["guid"];

				client = m_clientRegistry->GetClientByGuid(guid);

				if (client)
				{
					client->Touch();

					if (client->GetNetId() == 0xFFFF)
					{
						m_clientRegistry->HandleConnectingClient(client);
					}

					client->SetNetId(1);
					client->SetPeer(peer);

					net::Buffer outMsg;
					outMsg.Write(1);

					auto outStr = fmt::sprintf(" %d -1 -1", 1);
					outMsg.Write(outStr.c_str(), outStr.size());

					auto packet = enet_packet_create(outMsg.GetBuffer(), outMsg.GetLength(), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet);
				}
			}

			return;
		}

		// if not type 1, and no client, bail out
		if (!client)
		{
			return;
		}

		client->Touch();
	}

	void GameServer::ProcessHost(ENetHost* host)
	{
		ENetEvent event;

		while (enet_host_service(host, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				ProcessPacket(event.peer, event.packet->data, event.packet->dataLength);
				enet_packet_destroy(event.packet);
				break;
			}
		}
	}

	void GameServer::ProcessServerFrame(int frameTime)
	{
		m_serverTime += frameTime;

		m_clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
		{
			auto peer = client->GetPeer();

			if (peer)
			{
				net::Buffer outMsg;
				outMsg.Write(0x53FFFA3F);
				outMsg.Write(0);

				auto packet = enet_packet_create(outMsg.GetBuffer(), outMsg.GetLength(), ENET_PACKET_FLAG_RELIABLE);
				enet_peer_send(peer, 0, packet);
			}
		});
	}

	void GameServer::SendOutOfBand(const AddressPair& to, const std::string_view& oob)
	{
		auto addr = GetENetAddress(std::get<net::PeerAddress>(to));

		auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(oob);

		ENetBuffer buffer;
		buffer.data = (uint8_t*)oobMsg.c_str();
		buffer.dataLength = oobMsg.size();

		enet_socket_send(std::get<ENetHost*>(to)->socket, &addr, &buffer, 1);
	}

	namespace ServerDecorators
	{
		fwRefContainer<fx::GameServer> NewGameServer()
		{
			return new fx::GameServer();
		}

		struct ENetWait
		{
			inline void operator()(const fwRefContainer<fx::GameServer>& server, int maxTime)
			{
				// service enet with our remaining waits
				ENetSocketSet readfds;
				ENET_SOCKETSET_EMPTY(readfds);

				for (auto& host : server->hosts)
				{
					ENET_SOCKETSET_ADD(readfds, host->socket);
				}

				enet_socketset_select(server->hosts.size(), &readfds, nullptr, maxTime);

				for (auto& host : server->hosts)
				{
					server->ProcessHost(host.get());
				}
			}
		};

		struct GameServerTick
		{
			inline void operator()(const fwRefContainer<fx::GameServer>& server, int frameTime)
			{
				server->ProcessServerFrame(frameTime);
			}
		};

		struct GetInfoOOB
		{
			inline void Process(const fwRefContainer<fx::GameServer>& server, const AddressPair& from, const std::string_view& data) const
			{
				server->SendOutOfBand(from, fmt::format(
					"infoResponse\n"
					"\\sv_maxclients\\24\\clients\\0\\challenge\\{0}\\gamename\\CitizenFX\\protocol\\4\\hostname\\empty\\gametype\\\\mapname\\\\iv\\0",
					std::string(data.substr(0, data.find_first_of(" \n")))
				));
			}

			inline const char* GetName() const
			{
				return "getinfo";
			}
		};
	}
}

#include <decorators/WithEndpoints.h>
#include <decorators/WithOutOfBand.h>
#include <decorators/WithProcessTick.h>

static InitFunction initFunction([]()
{
	enet_initialize();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		using namespace fx::ServerDecorators;

		instance->SetComponent(
			WithProcessTick<ENetWait, GameServerTick>(
				WithOutOfBand<GetInfoOOB>(
					WithEndPoints(
						NewGameServer()
					)
				),
				20
			)
		);
	});
});