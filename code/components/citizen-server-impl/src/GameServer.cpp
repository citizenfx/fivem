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

			addr.host.s6_addr[10] = 0xFF;
			addr.host.s6_addr[11] = 0xFF;

			memcpy(&addr.host.s6_addr[12], &in4->sin_addr.s_addr, 4);
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

		m_rconPassword = instance->AddVariable<std::string>("rcon_password", ConVar_None, "");
		m_hostname = instance->AddVariable<std::string>("sv_hostname", ConVar_None, "default FXServer");

		instance->OnInitialConfiguration.Connect([=]()
		{
			m_thread = std::thread([=]()
			{
				Run();
			});
		}, 100);

		m_clientRegistry = instance->GetComponent<ClientRegistry>().GetRef();
		m_instance = instance;
	}

	void GameServer::Run()
	{
		if (m_runLoop)
		{
			m_runLoop();
		}
	}

	std::string GameServer::GetVariable(const std::string& key)
	{
		auto consoleCtx = m_instance->GetComponent<console::Context>();
		auto variable = consoleCtx->GetVariableManager()->FindEntryRaw(key);

		if (!variable)
		{
			return "";
		}

		return variable->GetValue();
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
				auto token = postMap["token"];

				client = m_clientRegistry->GetClientByGuid(guid);

				if (client)
				{
					if (token != client->GetConnectionToken())
					{
						SendOutOfBand(AddressPair{ peer->host, client->GetAddress() }, "error Invalid connection token received.");

						m_clientRegistry->RemoveClient(client);

						return;
					}

					client->Touch();

					client->SetPeer(peer);

					if (client->GetNetId() == 0xFFFF)
					{
						m_clientRegistry->HandleConnectingClient(client);
					}

					net::Buffer outMsg;
					outMsg.Write(1);

					auto host = m_clientRegistry->GetHost();

					auto outStr = fmt::sprintf(" %d %d %d", client->GetNetId(), (host) ? host->GetNetId() : -1, (host) ? host->GetNetBase() : -1);
					outMsg.Write(outStr.c_str(), outStr.size());

					client->SendPacket(0, outMsg, ENET_PACKET_FLAG_RELIABLE);

					m_clientRegistry->HandleConnectedClient(client);
				}
			}

			return;
		}

		// if not type 1, and no client, bail out
		if (!client)
		{
			return;
		}

		if (m_packetHandler)
		{
			m_packetHandler(msgType, client, msg);
		}

		client->Touch();
	}

	void GameServer::Broadcast(const net::Buffer& buffer)
	{
		m_clientRegistry->ForAllClients([&](const std::shared_ptr<Client>& client)
		{
			client->SendPacket(0, buffer, ENET_PACKET_FLAG_RELIABLE);
		});
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

				client->SendPacket(0, outMsg, ENET_PACKET_FLAG_RELIABLE);
			}
		});

		OnTick();
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

	struct FxPrintListener
	{
		static thread_local std::function<void(const std::string_view& cb)> listener;

		FxPrintListener()
		{
			console::CoreAddPrintListener([](ConsoleChannel channel, const char* data)
			{
				if (listener)
				{
					listener(data);
				}
			});
		}
	};

	FxPrintListener printListener;

	thread_local std::function<void(const std::string_view& cb)> FxPrintListener::listener;

	struct PrintListenerContext
	{
		PrintListenerContext(const std::function<void(const std::string_view& cb)>& fn)
		{
			oldFn = FxPrintListener::listener;

			FxPrintListener::listener = fn;
		}

		~PrintListenerContext()
		{
			FxPrintListener::listener = oldFn;
		}

	private:
		std::function<void(const std::string_view& cb)> oldFn;
	};

	struct ScopeDestructor
	{
		ScopeDestructor(const std::function<void()>& fn)
			: m_fn(fn)
		{
			
		}

		~ScopeDestructor()
		{
			if (m_fn)
			{
				m_fn();
			}
		}

	private:
		std::function<void()> m_fn;
	};

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
				int numClients = 0;

				server->GetInstance()->GetComponent<fx::ClientRegistry>()->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
				{
					if (client->GetNetId() != -1)
					{
						++numClients;
					}
				});

				server->SendOutOfBand(from, fmt::format(
					"infoResponse\n"
					"\\sv_maxclients\\24\\clients\\{4}\\challenge\\{0}\\gamename\\CitizenFX\\protocol\\4\\hostname\\{1}\\gametype\\{2}\\mapname\\{3}\\iv\\0",
					std::string(data.substr(0, data.find_first_of(" \n"))),
					server->GetVariable("sv_hostname"),
					server->GetVariable("gametype"),
					server->GetVariable("mapname"),
					numClients
				));
			}

			inline const char* GetName() const
			{
				return "getinfo";
			}
		};

		struct RconOOB
		{
			void Process(const fwRefContainer<fx::GameServer>& server, const AddressPair& from, const std::string_view& data) const
			{
				int spacePos = data.find_first_of(" \n");

				auto password = data.substr(0, spacePos);
				auto command = data.substr(spacePos);

				auto serverPassword = server->GetRconPassword();

				std::string printString;

				PrintListenerContext context([&](const std::string_view& print)
				{
					printString += print;
				});

				ScopeDestructor destructor([&]()
				{
					server->SendOutOfBand(from, "print " + printString);
				});

				if (serverPassword.empty())
				{
					trace("The server must set rcon_password to be able to use this command.\n");
					return;
				}

				if (password != serverPassword)
				{
					trace("Invalid password.\n");
					return;
				}

				server->GetInstance()->GetComponent<console::Context>()->ExecuteSingleCommand(std::string(command));
			}

			inline const char* GetName() const
			{
				return "rcon";
			}
		};

		struct RoutingPacketHandler
		{
			inline static void Handle(ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client, net::Buffer& packet)
			{
				uint16_t targetNetId = packet.Read<uint16_t>();
				uint16_t packetLength = packet.Read<uint16_t>();

				std::vector<uint8_t> packetData(packetLength);
				if (packet.Read(packetData.data(), packetData.size()))
				{
					auto targetClient = instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(targetNetId);

					if (targetClient)
					{
						net::Buffer outPacket;
						outPacket.Write(0xE938445B);
						outPacket.Write(client->GetNetId());
						outPacket.Write(packetLength);
						outPacket.Write(packetData.data(), packetLength);

						targetClient->SendPacket(1, outPacket, ENET_PACKET_FLAG_UNSEQUENCED);
					}
				}
			}

			inline static constexpr const char* GetPacketId()
			{
				return "msgRoute";
			}
		};

		struct IHostPacketHandler
		{
			inline static void Handle(ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client, net::Buffer& packet)
			{
				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
				auto gameServer = instance->GetComponent<fx::GameServer>();

				auto baseNum = packet.Read<uint32_t>();
				auto currentHost = clientRegistry->GetHost();

				if (!currentHost || currentHost->IsDead())
				{
					client->SetNetBase(baseNum);
					clientRegistry->SetHost(client);

					net::Buffer hostBroadcast;
					hostBroadcast.Write(0xB3EA30DE);
					hostBroadcast.Write(client->GetNetId());
					hostBroadcast.Write(client->GetNetBase());

					gameServer->Broadcast(hostBroadcast);
				}
			}

			inline static constexpr const char* GetPacketId()
			{
				return "msgIHost";
			}
		};
	}
}

#include <decorators/WithEndpoints.h>
#include <decorators/WithOutOfBand.h>
#include <decorators/WithProcessTick.h>
#include <decorators/WithPacketHandler.h>

static InitFunction initFunction([]()
{
	enet_initialize();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		using namespace fx::ServerDecorators;

		instance->SetComponent(
			WithPacketHandler<RoutingPacketHandler, IHostPacketHandler>(
				WithProcessTick<ENetWait, GameServerTick>(
					WithOutOfBand<GetInfoOOB, RconOOB>(
						WithEndPoints(
							NewGameServer()
						)
					),
					20
				)
			)
		);
	});
});
