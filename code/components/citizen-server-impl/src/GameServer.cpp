#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <GameServer.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include "ServerEventComponent.h"

#include <NetBuffer.h>

#include <state/ServerGameState.h>

#include <PrintListener.h>

#include <msgpack.hpp>

#include <protocol/reqrep0/req.h>
#include <protocol/reqrep0/rep.h>

static fx::GameServer* g_gameServer;

extern std::shared_ptr<ConVar<bool>> g_oneSyncVar;

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
		: m_residualTime(0), m_serverTime(msec().count()), m_nextHeartbeatTime(0), m_basePeerId(0)
	{
		g_gameServer = this;

		seCreateContext(&m_seContext);

		m_seContext->MakeCurrent();
		m_seContext->AddAccessControlEntry(se::Principal{ "system.console" }, se::Object{ "command" }, se::AccessType::Allow);
		m_seContext->AddAccessControlEntry(se::Principal{ "builtin.everyone" }, se::Object{ "command.help" }, se::AccessType::Allow);
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
		m_hostname = instance->AddVariable<std::string>("sv_hostname", ConVar_ServerInfo, "default FXServer");
		m_masters[0] = instance->AddVariable<std::string>("sv_master1", ConVar_None, "live-internal.fivem.net:30110");
		m_masters[1] = instance->AddVariable<std::string>("sv_master2", ConVar_None, "");
		m_masters[2] = instance->AddVariable<std::string>("sv_master3", ConVar_None, "");

		m_heartbeatCommand = instance->AddCommand("heartbeat", [=]()
		{
			ForceHeartbeat();
		});

		instance->OnInitialConfiguration.Connect([=]()
		{
			m_thread = std::thread([=]()
			{
				SetThreadName(-1, "[Cfx] Server Thread");

				Run();
			});
		}, 100);

		m_clientRegistry = instance->GetComponent<ClientRegistry>().GetRef();
		m_instance = instance;

		std::thread([=]()
		{
			while (true)
			{
				for (auto& master : m_masters)
				{
					// if the master is set
					std::string masterName = master->GetValue();

					if (!masterName.empty())
					{
						// look up if not cached
						auto address = net::PeerAddress::FromString(masterName, 30110, net::PeerAddress::LookupType::ResolveName);

						if (address)
						{
							if (m_masterCache[masterName] != *address)
							{
								trace("Resolved %s to %s\n", masterName, address->ToString());

								m_masterCache[masterName] = *address;
							}
						}
					}
				}

				std::this_thread::sleep_for(60s);
			}
		}).detach();

		std::thread([this]()
		{
			SetThreadName(-1, "[Cfx] Network Thread");

			nng_socket netSocket;
			nng_rep0_open(&netSocket);

			nng_listener listener;
			nng_listen(netSocket, "inproc://netlib_client", &listener, NNG_FLAG_NONBLOCK);

			while (true)
			{
				// service enet with our remaining waits
				std::vector<ENetSocket> fds;

				for (auto& host : this->hosts)
				{
					fds.push_back(host->socket);
				}

				int rcvFd;
				nng_getopt_int(netSocket, NNG_OPT_RECVFD, &rcvFd);

				fds.push_back(rcvFd);

				ENetSocketSet readfds;
				ENET_SOCKETSET_EMPTY(readfds);

				for (auto fd : fds)
				{
					ENET_SOCKETSET_ADD(readfds, fd);
				}

				int nfds = 0;

#ifndef _WIN32
				nfds = *std::max_element(fds.begin(), fds.end());
#endif

				enet_socketset_select(nfds, &readfds, nullptr, 20);

				for (auto& host : this->hosts)
				{
					this->ProcessHost(host.get());
				}

				{
					void* msgBuffer;
					size_t msgLen;
					
					while (nng_recv(netSocket, &msgBuffer, &msgLen, NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC) == 0)
					{
						nng_free(msgBuffer, msgLen);

						int ok = 0;
						nng_send(netSocket, &ok, 4, NNG_FLAG_NONBLOCK);

						m_netThreadCallbacks.Run();
					}
				}
			}
		}).detach();
	}

	void GameServer::InternalRunMainThreadCbs(nng_socket socket)
	{
		void* msgBuffer;
		size_t msgLen;

		while (nng_recv(socket, &msgBuffer, &msgLen, NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC) == 0)
		{
			nng_free(msgBuffer, msgLen);

			int ok = 0;
			nng_send(socket, &ok, 4, NNG_FLAG_NONBLOCK);

			m_mainThreadCallbacks.Run();
		}
	}

	const ENetPeer* GameServer::InternalGetPeer(int peerId)
	{
		if (peerId == 0)
		{
			return nullptr;
		}

		return m_peerHandles.left.find(peerId)->get_right();
	}

	void GameServer::InternalResetPeer(int peerId)
	{
		enet_peer_reset(m_peerHandles.left.find(peerId)->get_right());
	}

	void GameServer::InternalSendPacket(int peer, int channel, const net::Buffer& buffer, ENetPacketFlag flags)
	{
		auto packet = enet_packet_create(buffer.GetBuffer(), buffer.GetLength(), flags);
		enet_peer_send(m_peerHandles.left.find(peer)->get_right(), channel, packet);
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
		auto peerIdIt = m_peerHandles.right.find(peer);
		int peerId = -1;

		if (peerIdIt != m_peerHandles.right.end())
		{
			peerId = peerIdIt->get_left();
		}

		auto client = m_clientRegistry->GetClientByPeer(peerId);

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

					if (!client->GetData("passedValidation").has_value())
					{
						SendOutOfBand(AddressPair{ peer->host, client->GetAddress() }, "error Invalid connection.");

						m_clientRegistry->RemoveClient(client);

						return;
					}

					client->Touch();

					client->SetPeer(peerId, GetPeerAddress(peer->address));

					if (g_oneSyncVar->GetValue())
					{
						if (client->GetSlotId() == -1)
						{
							SendOutOfBand(AddressPair{ peer->host, client->GetAddress() }, "error Not enough client slot IDs.");

							m_clientRegistry->RemoveClient(client);

							return;
						}
					}

					if (client->GetNetId() >= 0xFFFF)
					{
						m_clientRegistry->HandleConnectingClient(client);
					}

					// disable peer throttling
					enet_peer_throttle_configure(peer, 1000, ENET_PEER_PACKET_THROTTLE_SCALE, 0);

#ifdef _DEBUG
					enet_peer_timeout(peer, 86400 * 1000, 86400 * 1000, 86400 * 1000);
#endif

					// send a connectOK
					net::Buffer outMsg;
					outMsg.Write(1);

					auto host = m_clientRegistry->GetHost();

					auto outStr = fmt::sprintf(
						" %d %d %d %d %lld",
						client->GetNetId(),
						(host) ? host->GetNetId() : -1,
						(host) ? host->GetNetBase() : -1,
						(g_oneSyncVar->GetValue()) ? client->GetSlotId() : -1,
						(g_oneSyncVar->GetValue()) ? msec().count() : -1);

					outMsg.Write(outStr.c_str(), outStr.size());

					client->SendPacket(0, outMsg, ENET_PACKET_FLAG_RELIABLE);

					gscomms_execute_callback_on_main_thread([=]()
					{
						m_clientRegistry->HandleConnectedClient(client);
					});

					if (g_oneSyncVar->GetValue())
					{
						m_instance->GetComponent<fx::ServerGameState>()->SendObjectIds(client, 64);
					}

					ForceHeartbeat();
				}
			}

			return;
		}

		// if not type 1, and no client, bail out
		if (!client)
		{
			return;
		}

		auto principalScope = client->EnterPrincipalScope();

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
		m_seContext->MakeCurrent();

		ENetEvent event;

		while (enet_host_service(host, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				m_peerHandles.left.insert({ ++m_basePeerId, event.peer });
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				ProcessPacket(event.peer, event.packet->data, event.packet->dataLength);
				enet_packet_destroy(event.packet);
				break;
			}
		}
	}

	void GameServer::DeferCall(int inMsec, const std::function<void()>& fn)
	{
		static std::atomic<int> cbIdx;

		// find an empty slot first
		for (auto& pair : m_deferCallbacks)
		{
			if (std::get<int>(pair.second) == 0)
			{
				pair.second = { m_serverTime + inMsec, fn };
				return;
			}
		}

		m_deferCallbacks.insert({ cbIdx.fetch_add(1), { m_serverTime + inMsec, fn } });
	}

	void GameServer::CallbackList::Add(const std::function<void()>& fn)
	{
		// add to the queue
		callbacks.push(fn);

		// submit to the owning thread
		static thread_local nng_socket sockets[2];
		static thread_local nng_dialer dialers[2];

		int i = m_socketIdx;

		if (!sockets[i])
		{
			nng_req0_open(&sockets[i]);
			nng_dial(sockets[i], m_socketName.c_str(), &dialers[i], 0);
		}

		std::vector<int> idxList(1);
		idxList[0] = 0xFEED;

		nng_send(sockets[i], &idxList[0], idxList.size() * sizeof(int), NNG_FLAG_NONBLOCK);
	}

	void GameServer::CallbackList::Run()
	{
		std::function<void()> fn;

		while (callbacks.try_pop(fn))
		{
			fn();
		}
	}

	void GameServer::ProcessServerFrame(int frameTime)
	{
		m_seContext->MakeCurrent();

		m_serverTime += frameTime;

		// check callbacks
		for (auto& cb : m_deferCallbacks)
		{
			if (std::get<int>(cb.second) == 0)
			{
				continue;
			}

			if (m_serverTime >= std::get<int>(cb.second))
			{
				std::get<1>(cb.second)();

				cb.second = {};
			}
		}

		{
			std::vector<std::shared_ptr<fx::Client>> toRemove;

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

				// time out the client if needed
				if (client->IsDead())
				{
					toRemove.push_back(client);
				}
			});

			for (auto& client : toRemove)
			{
				DropClient(client, "Timed out after %d seconds.", std::chrono::duration_cast<std::chrono::seconds>(CLIENT_DEAD_TIMEOUT).count());
			}
		}

		// if we should heartbeat
		if (msec().count() >= m_nextHeartbeatTime)
		{
			// loop through each master
			for (auto& master : m_masters)
			{
				// if the master is set
				std::string masterName = master->GetValue();

				if (!masterName.empty())
				{
					// find a cached address
					auto it = m_masterCache.find(masterName);

					if (it != m_masterCache.end())
					{
						// loop through each primary host
						for (auto& host : hosts)
						{
							// send a heartbeat to the master
							SendOutOfBand({ host.get(), it->second }, "heartbeat DarkPlaces\n");
						}

						trace("Sending heartbeat to %s\n", masterName);
					}
				}
			}

			m_nextHeartbeatTime = msec().count() + (180 * 1000);
		}

		{
			auto ctx = GetInstance()->GetComponent<console::Context>();

			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
			ctx->ExecuteBuffer();
		}

		OnTick();
	}

	void GameServer::DropClient(const std::shared_ptr<Client>& client, const std::string& reason, const fmt::ArgList& args)
	{
		std::string realReason = fmt::sprintf(reason, args);

		if (reason.empty())
		{
			realReason = "Dropped.";
		}

		// send an out-of-band error to the client
		if (client->GetPeer())
		{
			SendOutOfBand({ m_peerHandles.left.find(client->GetPeer())->get_right()->host, client->GetAddress() }, fmt::sprintf("error %s", realReason));
		}

		// force a hearbeat
		ForceHeartbeat();

		// trigger a event signaling the player's drop
		m_instance
			->GetComponent<fx::ResourceManager>()
			->GetComponent<fx::ResourceEventManagerComponent>()
			->TriggerEvent2(
				"playerDropped",
				{ fmt::sprintf("net:%d", client->GetNetId()) },
				realReason
			);

		// remove the host if this was the host
		if (m_clientRegistry->GetHost() == client)
		{
			m_clientRegistry->SetHost(nullptr);

			// broadcast the current host
			net::Buffer hostBroadcast;
			hostBroadcast.Write(0xB3EA30DE);
			hostBroadcast.Write<uint16_t>(0xFFFF);
			hostBroadcast.Write(0xFFFF);

			Broadcast(hostBroadcast);
		}

		// signal a drop
		client->OnDrop();

		{
			// for name handling, send player state
			fwRefContainer<ServerEventComponent> events = m_instance->GetComponent<ServerEventComponent>();

			// send every player information about the joining client
			events->TriggerClientEvent("onPlayerDropped", std::optional<std::string_view>(), client->GetNetId(), client->GetName(), client->GetSlotId());
		}

		// drop the client
		m_clientRegistry->RemoveClient(client);
	}

	void GameServer::ForceHeartbeat()
	{
		m_nextHeartbeatTime = -1;
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

	FxPrintListener printListener;

	thread_local std::function<void(const std::string_view& cb)> FxPrintListener::listener;

	namespace ServerDecorators
	{
		struct pass
		{
			template<typename ...T> pass(T...) {}
		};

		fwRefContainer<fx::GameServer> NewGameServer()
		{
			return new fx::GameServer();
		}

		struct ThreadWait
		{
			ThreadWait()
			{
				nng_rep0_open(&m_socket);
				nng_listen(m_socket, "inproc://main_client", &m_listener, 0);
			}

			inline void operator()(const fwRefContainer<fx::GameServer>& server, int maxTime)
			{
				int rcvFd;
				nng_getopt_int(m_socket, NNG_OPT_RECVFD, &rcvFd);

				fd_set fds;
				FD_ZERO(&fds);
				FD_SET(rcvFd, &fds);

				timeval timeout;
				timeout.tv_sec = 0;
				timeout.tv_usec = maxTime * 1000;
				int rv = select(rcvFd + 1, &fds, nullptr, nullptr, &timeout);

				if (rv >= 0 && FD_ISSET(rcvFd, &fds))
				{
					server->InternalRunMainThreadCbs(m_socket);
				}
			}

			nng_socket m_socket;
			nng_listener m_listener;
		};

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
					if (client->GetNetId() < 0xFFFF)
					{
						++numClients;
					}
				});

				server->SendOutOfBand(from, fmt::format(
					"infoResponse\n"
					"\\sv_maxclients\\{6}\\clients\\{4}\\challenge\\{0}\\gamename\\CitizenFX\\protocol\\4\\hostname\\{1}\\gametype\\{2}\\mapname\\{3}\\iv\\{5}",
					std::string(data.substr(0, data.find_first_of(" \n"))),
					server->GetVariable("sv_hostname"),
					server->GetVariable("gametype"),
					server->GetVariable("mapname"),
					numClients,
					server->GetVariable("sv_infoVersion"),
					server->GetVariable("sv_maxclients")
				));
			}

			inline const char* GetName() const
			{
				return "getinfo";
			}
		};

		struct GetStatusOOB
		{
			inline void Process(const fwRefContainer<fx::GameServer>& server, const AddressPair& from, const std::string_view& data) const
			{
				int numClients = 0;
				std::stringstream clientList;

				server->GetInstance()->GetComponent<fx::ClientRegistry>()->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
				{
					if (client->GetNetId() < 0xFFFF)
					{
						clientList << fmt::sprintf("%d %d \"%s\"\n", 0, 0, client->GetName());

						++numClients;
					}
				});

				std::stringstream infoVars;

				auto addInfo = [&](const std::string& key, const std::string& value)
				{
					infoVars << "\\" << key << "\\" << value;
				};

				addInfo("sv_maxclients", "24");
				addInfo("clients", std::to_string(numClients));

				server->GetInstance()->GetComponent<console::Context>()->GetVariableManager()->ForAllVariables([&](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
				{
					addInfo(name, var->GetValue());
				}, ConVar_ServerInfo);

				server->SendOutOfBand(from, fmt::format(
					"statusResponse\n"
					"{0}\n"
					"{1}",
					infoVars.str(),
					clientList.str()
				));
			}

			inline const char* GetName() const
			{
				return "getstatus";
			}
		};

		struct RconOOB
		{
			void Process(const fwRefContainer<fx::GameServer>& server, const AddressPair& from, const std::string_view& dataView) const
			{
				std::string data(dataView);

				gscomms_execute_callback_on_main_thread([=]()
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

					auto ctx = server->GetInstance()->GetComponent<console::Context>();
					ctx->ExecuteBuffer();

					se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
					ctx->AddToBuffer(std::string(command));
					ctx->ExecuteBuffer();
				});
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
					if (targetNetId == 0xFFFF)
					{
						// TODO: make this run on the net thread
						gscomms_execute_callback_on_main_thread([=]()
						{
							instance->GetComponent<fx::ServerGameState>()->ParseGameStatePacket(client, packetData);
						});

						return;
					}

					auto targetClient = instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(targetNetId);

					if (targetClient)
					{
						net::Buffer outPacket;
						outPacket.Write(0xE938445B);
						outPacket.Write<uint16_t>(client->GetNetId());
						outPacket.Write(packetLength);
						outPacket.Write(packetData.data(), packetLength);

						targetClient->SendPacket(1, outPacket, (ENetPacketFlag)0);

						client->SetHasRouted();
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
				if (g_oneSyncVar->GetValue())
				{
					return;
				}

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
					hostBroadcast.Write<uint16_t>(client->GetNetId());
					hostBroadcast.Write(client->GetNetBase());

					gameServer->Broadcast(hostBroadcast);
				}
			}

			inline static constexpr const char* GetPacketId()
			{
				return "msgIHost";
			}
		};

		struct HostVoteCount : public fwRefCountable
		{
			std::map<uint32_t, int> voteCounts;
		};

		// TODO: replace with system using dissectors
		struct HeHostPacketHandler
		{
			inline static void Handle(ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client, net::Buffer& packet)
			{
				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
				auto gameServer = instance->GetComponent<fx::GameServer>();

				auto allegedNewId = packet.Read<uint32_t>();
				auto baseNum = packet.Read<uint32_t>();

				// check if the current host is being vouched for
				auto currentHost = clientRegistry->GetHost();

				if (currentHost && currentHost->GetNetId() == allegedNewId)
				{
					trace("Got a late vouch for %s - they're the current arbitrator!\n", currentHost->GetName());
					return;
				}

				// get the new client
				auto newClient = clientRegistry->GetClientByNetID(allegedNewId);

				if (!newClient)
				{
					trace("Got a late vouch for %d, who doesn't exist.\n", allegedNewId);
					return;
				}

				// count the total amount of living (networked) clients
				int numClients = 0;

				clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
				{
					if (client->HasRouted())
					{
						++numClients;
					}
				});

				// get a count of needed votes
				int votesNeeded = (int)ceil(numClients * 0.6);

				if (votesNeeded <= 0)
				{
					votesNeeded = 1;
				}

				// count votes
				auto voteComponent = instance->GetComponent<HostVoteCount>();

				auto it = voteComponent->voteCounts.find(allegedNewId);

				if (it == voteComponent->voteCounts.end())
				{
					it = voteComponent->voteCounts.insert({ allegedNewId, 1 }).first;
				}

				++it->second;

				// log
				trace("Received a vouch for %s, they have %d vouches and need %d.\n", newClient->GetName(), it->second, votesNeeded);

				// is the vote count exceeded?
				if (it->second >= votesNeeded)
				{
					// make new arbitrator
					trace("%s is the new arbitrator, with an overwhelming %d vote/s.\n", newClient->GetName(), it->second);

					// clear vote list
					voteComponent->voteCounts.clear();

					// set base
					newClient->SetNetBase(baseNum);

					// set as host and tell everyone
					clientRegistry->SetHost(newClient);

					net::Buffer hostBroadcast;
					hostBroadcast.Write(0xB3EA30DE);
					hostBroadcast.Write<uint16_t>(newClient->GetNetId());
					hostBroadcast.Write(newClient->GetNetBase());

					gameServer->Broadcast(hostBroadcast);
				}
			}

			inline static constexpr const char* GetPacketId()
			{
				return "msgHeHost";
			}
		};

		struct IQuitPacketHandler
		{
			inline static void Handle(ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client, net::Buffer& packet)
			{
				gscomms_execute_callback_on_main_thread([=]() mutable
				{
					std::vector<char> reason(packet.GetRemainingBytes());
					packet.Read(reason.data(), reason.size());

					auto gameServer = instance->GetComponent<fx::GameServer>();

					gameServer->DropClient(client, "%s", reason.data());
				});
			}

			inline static constexpr const char* GetPacketId()
			{
				return "msgIQuit";
			}
		};
	}
}

DECLARE_INSTANCE_TYPE(fx::ServerDecorators::HostVoteCount);

#include <decorators/WithEndpoints.h>
#include <decorators/WithOutOfBand.h>
#include <decorators/WithProcessTick.h>
#include <decorators/WithPacketHandler.h>

void gscomms_execute_callback_on_main_thread(const std::function<void()>& fn)
{
	g_gameServer->InternalAddMainThreadCb(fn);
}

void gscomms_execute_callback_on_net_thread(const std::function<void()>& fn)
{
	g_gameServer->InternalAddNetThreadCb(fn);
}

void gscomms_reset_peer(int peer)
{
	gscomms_execute_callback_on_net_thread([=]()
	{
		g_gameServer->InternalResetPeer(peer);
	});
}

void gscomms_send_packet(int peer, int channel, const net::Buffer& buffer, ENetPacketFlag flags)
{
	gscomms_execute_callback_on_net_thread([=]()
	{
		g_gameServer->InternalSendPacket(peer, channel, buffer, flags);
	});
}

const ENetPeer* gscomms_get_peer(int peer)
{
	return g_gameServer->InternalGetPeer(peer);
}

static InitFunction initFunction([]()
{
	enet_initialize();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		using namespace fx::ServerDecorators;

		instance->SetComponent(
			WithPacketHandler<RoutingPacketHandler, IHostPacketHandler, IQuitPacketHandler, HeHostPacketHandler>(
				WithProcessTick<ThreadWait, GameServerTick>(
					WithOutOfBand<GetInfoOOB, GetStatusOOB, RconOOB>(
						WithEndPoints(
							NewGameServer()
						)
					),
					20
				)
			)
		);

		instance->SetComponent(new fx::ServerDecorators::HostVoteCount());
	});
});
