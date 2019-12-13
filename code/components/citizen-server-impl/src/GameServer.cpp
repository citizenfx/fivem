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

#include <UvLoopManager.h>
#include <UvTcpServer.h> // for UvCallback

#include <MonoThreadAttachment.h>

#include <HttpClient.h>
#include <TcpListenManager.h>
#include <ServerLicensingComponent.h>

#include <json.hpp>

#include <nng/protocol/reqrep0/req.h>
#include <nng/protocol/reqrep0/rep.h>

#include <KeyedRateLimiter.h>

#ifdef _WIN32
#include <ResumeComponent.h>
#endif

static fx::GameServer* g_gameServer;

extern std::shared_ptr<ConVar<bool>> g_oneSyncVar;

extern fwEvent<> OnEnetReceive;

namespace fx
{
	GameServer::GameServer()
		: m_residualTime(0), m_serverTime(msec().count()), m_nextHeartbeatTime(0)
	{
		g_gameServer = this;

		// TODO: re-enable this when we actually figure out threading
		//seCreateContext(&m_seContext);
		m_seContext = seGetCurrentContext();

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
		m_instance = instance;

		m_gamename = instance->AddVariable<GameName>("gamename", ConVar_ServerInfo, GameName::GTA5);
		m_lastGameName = m_gamename->GetHelper()->GetValue();

#ifdef _WIN32
		OnAbnormalTermination.Connect([this](void* reason)
		{
			auto realReason = fmt::sprintf("Server shutting down: %s", (const char*)reason);

			m_clientRegistry->ForAllClients([this, realReason](const std::shared_ptr<fx::Client>& client)
			{
				if (client->GetPeer())
				{
					auto oob = fmt::sprintf("error %s", realReason);
					m_net->SendOutOfBand(client->GetAddress(), oob);
				}
			});
		});
#endif

		m_net = fx::CreateGSNet(this);

		if (m_interceptor)
		{
			m_net->AddRawInterceptor(m_interceptor);
		}

		OnAttached(instance);

		m_rconPassword = instance->AddVariable<std::string>("rcon_password", ConVar_None, "");
		m_hostname = instance->AddVariable<std::string>("sv_hostname", ConVar_ServerInfo, "default FXServer");
		m_masters[0] = instance->AddVariable<std::string>("sv_master1", ConVar_None, "https://servers-ingress-live.fivem.net/ingress");
		m_masters[1] = instance->AddVariable<std::string>("sv_master2", ConVar_None, "");
		m_masters[2] = instance->AddVariable<std::string>("sv_master3", ConVar_None, "");
		m_listingIpOverride = instance->AddVariable<std::string>("sv_listingIpOverride", ConVar_None, "");
		m_useDirectListing = instance->AddVariable<bool>("sv_useDirectListing", ConVar_None, false);

		m_heartbeatCommand = instance->AddCommand("heartbeat", [=]()
		{
			ForceHeartbeat();
		});

		m_mainThreadCallbacks = std::make_unique<CallbackListNng>("inproc://main_client", 0);

		instance->OnRequestQuit.Connect([this](const std::string& reason)
		{
			m_clientRegistry->ForAllClients([this, &reason](const std::shared_ptr<fx::Client>& client)
			{
				DropClient(client, "Server shutting down: %s", reason);
			});
		});

		instance->OnInitialConfiguration.Connect([=]()
		{
			if (!m_net->SupportsUvUdp())
			{
				m_thread = std::thread([=]()
				{
					SetThreadName(-1, "[Cfx] Server Thread");

					m_mainThreadCallbacks->AttachToThread();

					Run();
				});
			}
			else
			{
				m_mainThreadLoop = Instance<net::UvLoopManager>::Get()->GetOrCreate("svMain");

				static std::shared_ptr<uvw::SignalHandle> sigint = m_mainThreadLoop->Get()->resource<uvw::SignalHandle>();
				sigint->start(SIGINT);

				static std::shared_ptr<uvw::SignalHandle> sighup = m_mainThreadLoop->Get()->resource<uvw::SignalHandle>();
				sighup->start(SIGHUP);

				sigint->on<uvw::SignalEvent>([this](const uvw::SignalEvent& ev, uvw::SignalHandle& sig)
				{
					se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
					m_instance->GetComponent<console::Context>()->ExecuteSingleCommandDirect(ProgramArguments{ "quit", "SIGINT received" });
				});

				sighup->on<uvw::SignalEvent>([this](const uvw::SignalEvent& ev, uvw::SignalHandle& sig)
				{
					se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
					m_instance->GetComponent<console::Context>()->ExecuteSingleCommandDirect(ProgramArguments{ "quit", "SIGHUP received" });
				});

				auto asyncInitHandle = std::make_shared<std::unique_ptr<UvHandleContainer<uv_async_t>>>();;
				*asyncInitHandle = std::make_unique<UvHandleContainer<uv_async_t>>();

				uv_async_init(m_mainThreadLoop->GetLoop(), (*asyncInitHandle)->get(), UvPersistentCallback((*asyncInitHandle)->get(), [this, asyncInitHandle](uv_async_t*)
				{
					// private data for the net thread
					struct MainPersistentData
					{
						UvHandleContainer<uv_timer_t> tickTimer;

						std::shared_ptr<std::unique_ptr<UvHandleContainer<uv_async_t>>> callbackAsync;

						std::chrono::milliseconds lastTime;
					};

					auto mainData = std::make_shared<MainPersistentData>();
					mainData->lastTime = msec();

					auto loop = m_mainThreadLoop->GetLoop();

					// periodic timer for network ticks
					auto frameTime = 1000 / 20;

					auto mpd = mainData.get();

					uv_timer_init(loop, &mainData->tickTimer);
					uv_timer_start(&mainData->tickTimer, UvPersistentCallback(&mainData->tickTimer, [this, mpd](uv_timer_t*)
					{
						auto now = msec();
						auto thisTime = now - mpd->lastTime;
						mpd->lastTime = now;

						if (thisTime > 150ms)
						{
							trace("server thread hitch warning: timer interval of %d milliseconds\n", thisTime.count());
						}

						ProcessServerFrame(thisTime.count());
					}), frameTime, frameTime);

					// event handle for callback list evaluation
					
					// unique_ptr wrapper is a workaround for libc++ bug (fixed in LLVM r340823)
					mainData->callbackAsync = std::make_shared<std::unique_ptr<UvHandleContainer<uv_async_t>>>();
					*mainData->callbackAsync = std::make_unique<UvHandleContainer<uv_async_t>>();

					uv_async_init(loop, (*mainData->callbackAsync)->get(), UvPersistentCallback((*mainData->callbackAsync)->get(), [this](uv_async_t*)
					{
						m_mainThreadCallbacks->Run();
					}));

					m_mainThreadCallbacks = std::make_unique<CallbackListUv>(mainData->callbackAsync);
					m_mainThreadCallbacks->AttachToThread();

					// store the pointer in the class for lifetime purposes
					m_mainThreadData = std::move(mainData);
				}));

				uv_async_send((*asyncInitHandle)->get());
			}
		}, 100);

		m_clientRegistry = instance->GetComponent<ClientRegistry>().GetRef();

		std::thread([=]()
		{
			while (true)
			{
				for (auto& master : m_masters)
				{
					// if the master is set
					std::string masterName = master->GetValue();

					if (!masterName.empty() && masterName.find("https://") != 0 && masterName.find("http://") != 0)
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

		if (m_net->SupportsUvUdp())
		{
			InitializeNetUv();
		}
		else
		{
			InitializeNetThread();
		}
	}

	void GameServer::InitializeNetUv()
	{
		m_netThreadLoop = Instance<net::UvLoopManager>::Get()->GetOrCreate("svNetwork");

		auto asyncInitHandle = std::make_shared<std::unique_ptr<UvHandleContainer<uv_async_t>>>();
		*asyncInitHandle = std::make_unique<UvHandleContainer<uv_async_t>>();

		uv_async_init(m_netThreadLoop->GetLoop(), (*asyncInitHandle)->get(), UvPersistentCallback((*asyncInitHandle)->get(), [this, asyncInitHandle](uv_async_t*)
		{
			// private data for the net thread
			struct NetPersistentData
			{
				UvHandleContainer<uv_timer_t> tickTimer;

				std::shared_ptr<std::unique_ptr<UvHandleContainer<uv_async_t>>> callbackAsync;

				std::chrono::milliseconds lastTime;
			};

			auto netData = std::make_shared<NetPersistentData>();
			auto loop = m_netThreadLoop->GetLoop();

			netData->lastTime = msec();

			// periodic timer for network ticks
			auto frameTime = 1000 / 120;

			auto mpd = netData.get();
			
			uv_timer_init(loop, &netData->tickTimer);
			uv_timer_start(&netData->tickTimer, UvPersistentCallback(&netData->tickTimer, [this, mpd](uv_timer_t*)
			{
				auto now = msec();
				auto thisTime = now - mpd->lastTime;
				mpd->lastTime = now;

				if (thisTime > 150ms)
				{
					trace("network thread hitch warning: timer interval of %d milliseconds\n", thisTime.count());
				}

				m_net->Process();
				OnNetworkTick();
			}), frameTime, frameTime);

			// event handle for callback list evaluation

			// unique_ptr wrapper is a workaround for libc++ bug (fixed in LLVM r340823)
			netData->callbackAsync = std::make_shared<std::unique_ptr<UvHandleContainer<uv_async_t>>>();
			*netData->callbackAsync = std::make_unique<UvHandleContainer<uv_async_t>>();

			uv_async_init(loop, (*netData->callbackAsync)->get(), UvPersistentCallback((*netData->callbackAsync)->get(), [this](uv_async_t*)
			{
				m_netThreadCallbacks->Run();
			}));

			m_netThreadCallbacks = std::make_unique<CallbackListUv>(netData->callbackAsync);
			m_netThreadCallbacks->AttachToThread();

			// process hosts on a command
			// #TODO1SBIG: add bigmode check/convar?
			/*OnEnetReceive.Connect([this]()
			{
				m_net->Process();
			});*/

			// store the pointer in the class for lifetime purposes
			m_netThreadData = std::move(netData);
		}));

		uv_async_send((*asyncInitHandle)->get());
	}

	void GameServer::InitializeNetThread()
	{
		m_netThreadCallbacks = std::make_unique<CallbackListNng>("inproc://netlib_client", 1);

		std::thread([this]()
		{
			SetThreadName(-1, "[Cfx] Network Thread");

			m_netThreadCallbacks->AttachToThread();

			nng_socket netSocket;
			nng_rep0_open(&netSocket);

			nng_listener listener;
			nng_listen(netSocket, "inproc://netlib_client", &listener, NNG_FLAG_NONBLOCK);

			auto lastTime = msec().count();

			uint64_t residualTime = 0;
			auto frameTime = 1000 / 30;

			while (true)
			{
				// service enet with our remaining waits
				int rcvFd;
				nng_getopt_int(netSocket, NNG_OPT_RECVFD, &rcvFd);

				m_net->Select({ uintptr_t(rcvFd) }, frameTime);

				{
					m_seContext->MakeCurrent();
					m_net->Process();
				}

				{
					auto now = msec().count() - lastTime;

					if (now >= 150)
					{
						trace("hitch warning: net frame time of %d milliseconds\n", now);
					}

					// clamp time to 200ms to reduce effects of excessive hitches
					if (now > 200)
					{
						now = 200;
					}

					residualTime += now;

					lastTime = msec().count();

					// intervals
					if (residualTime > frameTime)
					{
						OnNetworkTick();

						residualTime = 0;
					}
				}

				{
					void* msgBuffer;
					size_t msgLen;

					while (nng_recv(netSocket, &msgBuffer, &msgLen, NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC) == 0)
					{
						nng_free(msgBuffer, msgLen);

						int ok = 0;
						nng_send(netSocket, &ok, 4, NNG_FLAG_NONBLOCK);

						m_netThreadCallbacks->Run();
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

			m_mainThreadCallbacks->Run();
		}
	}

	fwRefContainer<NetPeerBase> GameServer::InternalGetPeer(int peerId)
	{
		return m_net->GetPeer(peerId);
	}

	void GameServer::InternalResetPeer(int peerId)
	{
		m_net->ResetPeer(peerId);
	}

	void GameServer::InternalSendPacket(const std::shared_ptr<fx::Client>& client, int peer, int channel, const net::Buffer& buffer, NetPacketType type)
	{
		// TODO: think of a more uniform way to determine null peers
		if (m_net->GetPeer(peer)->GetPing() == -1)
		{
			if (type == NetPacketType_ReliableReplayed)
			{
				client->PushReplayPacket(channel, buffer);
			}

			return;
		}

		m_net->SendPacket(peer, channel, buffer, type);
	}

	void GameServer::Run()
	{
		if (m_runLoop)
		{
			m_runLoop();
		}
	}

	void GameServer::AddRawInterceptor(const std::function<bool(const uint8_t *, size_t, const net::PeerAddress &)>& interceptor)
	{
		m_interceptor = interceptor;
	}

	void GameServer::CreateUdpHost(const net::PeerAddress& address)
	{
		m_net->CreateUdpHost(address);
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

	void GameServer::ProcessPacket(const fwRefContainer<NetPeerBase>& peer, const uint8_t* data, size_t size)
	{
		// create a netbuffer and read the message type
		net::Buffer msg(data, size);
		uint32_t msgType = msg.Read<uint32_t>();

		// get the client
		auto peerId = peer->GetId();

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
						SendOutOfBand(peer->GetAddress(), "error Invalid connection token received.");

						m_clientRegistry->RemoveClient(client);

						return;
					}

					if (!client->GetData("passedValidation").has_value())
					{
						SendOutOfBand(peer->GetAddress(), "error Invalid connection.");

						m_clientRegistry->RemoveClient(client);

						return;
					}

					client->Touch();

					client->SetPeer(peerId, peer->GetAddress());

					if (g_oneSyncVar->GetValue())
					{
						if (client->GetSlotId() == -1)
						{
							SendOutOfBand(peer->GetAddress(), "error Not enough client slot IDs.");

							m_clientRegistry->RemoveClient(client);

							return;
						}
					}

					bool wasNew = false;

					if (client->GetNetId() >= 0xFFFF)
					{
						m_clientRegistry->HandleConnectingClient(client);

						wasNew = true;
					}

					peer->OnSendConnectOK();

					// send a connectOK
					net::Buffer outMsg;
					outMsg.Write(1);

					auto host = m_clientRegistry->GetHost();

					auto outStr = fmt::sprintf(
						" %d %d %d %d %lld",
						client->GetNetId(),
						(host) ? host->GetNetId() : -1,
						(host) ? host->GetNetBase() : -1,
						(g_oneSyncVar->GetValue())
							? ((fx::IsBigMode())
								? 128
								: client->GetSlotId())
							: -1,
						(g_oneSyncVar->GetValue()) ? msec().count() : -1);

					outMsg.Write(outStr.c_str(), outStr.size());

					client->SendPacket(0, outMsg, NetPacketType_Reliable);

					client->ReplayPackets();

					if (wasNew)
					{
						gscomms_execute_callback_on_main_thread([=]()
						{
							m_clientRegistry->HandleConnectedClient(client);
						});

						if (g_oneSyncVar->GetValue())
						{
							m_instance->GetComponent<fx::ServerGameState>()->SendObjectIds(client, fx::IsBigMode() ? 4 : 64);
						}

						ForceHeartbeat();
					}
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
			client->SendPacket(0, buffer, NetPacketType_Reliable);
		});
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

	void GameServer::CallbackListBase::Add(const std::function<void()>& fn, bool force)
	{
		if (threadId == std::this_thread::get_id() && !force)
		{
			fn();
			return;
		}

		// add to the queue
		callbacks.push(fn);

		// signal the waiting thread
		SignalThread();
	}

	void GameServer::CallbackListBase::Run()
	{
		std::function<void()> fn;

		while (callbacks.try_pop(fn))
		{
			fn();
		}
	}

	void GameServer::CallbackListNng::SignalThread()
	{
		// submit to the owning thread
		static thread_local nng_socket sockets[2];
		static thread_local nng_dialer dialers[2];

		int i = m_socketIdx;

		if (!sockets[i].id)
		{
			nng_req0_open(&sockets[i]);
			nng_dial(sockets[i], m_socketName.c_str(), &dialers[i], 0);
		}

		std::vector<int> idxList(1);
		idxList[0] = 0xFEED;

		nng_send(sockets[i], &idxList[0], idxList.size() * sizeof(int), NNG_FLAG_NONBLOCK);
	}

	void GameServer::CallbackListUv::SignalThread()
	{
		auto async = m_async.lock();

		if (async)
		{
			uv_async_send((*async)->get());
		}
	}

	void GameServer::ProcessServerFrame(int frameTime)
	{
		MonoEnsureThreadAttached();
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

					client->SendPacket(0, outMsg, NetPacketType_Unreliable);
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
					if (masterName.find("https://") != 0 && masterName.find("http://") != 0)
					{
						// find a cached address
						auto it = m_masterCache.find(masterName);

						if (it != m_masterCache.end())
						{
							// send a heartbeat to the master
							SendOutOfBand(it->second, "heartbeat DarkPlaces\n");

							trace("Sending heartbeat to %s\n", masterName);
						}
					}
					else
					{
						trace("Sending heartbeat to %s\n", masterName);

						auto json = nlohmann::json::object({
							{ "port", m_instance->GetComponent<fx::TcpListenManager>()->GetPrimaryPort() },
							{ "listingToken", m_instance->GetComponent<ServerLicensingComponent>()->GetListingToken() },
							{ "ipOverride", m_listingIpOverride->GetValue() },
							{ "useDirectListing", m_useDirectListing->GetValue() },
						});

						HttpRequestOptions ro;
						ro.ipv4 = true;
						ro.headers = std::map<std::string, std::string>{
							{ "Content-Type", "application/json; charset=utf-8" }
						};

						Instance<HttpClient>::Get()->DoPostRequest(masterName, json.dump(), ro, [](bool success, const char* d, size_t s)
						{
							if (!success)
							{
								trace("error submitting to ingress: %s\n", std::string{ d, s });
							}
						});
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

		if (m_gamename->GetHelper()->GetValue() != m_lastGameName)
		{
			if (!m_lastGameName.empty())
			{
				console::PrintError("server", "Reverted a `gamename` change. You can't change gamename while the server is running!\n");
				m_gamename->GetHelper()->SetValue(m_lastGameName);
			}

			m_lastGameName = m_gamename->GetHelper()->GetValue();
		}

		OnTick();
	}

	void GameServer::DropClientv(const std::shared_ptr<Client>& client, const std::string& reason, fmt::printf_args args)
	{
		std::string realReason = fmt::vsprintf(reason, args);

		if (reason.empty())
		{
			realReason = "Dropped.";
		}

		// send an out-of-band error to the client
		if (client->GetPeer())
		{
			SendOutOfBand(client->GetAddress(), fmt::sprintf("error %s", realReason));
		}

		// force a hearbeat
		ForceHeartbeat();

		// ensure mono thread attachment (if this was a worker thread)
		MonoEnsureThreadAttached();

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

			if (!fx::IsBigMode())
			{
				// send every player information about the dropping client
				events->TriggerClientEventReplayed("onPlayerDropped", std::optional<std::string_view>(), client->GetNetId(), client->GetName(), client->GetSlotId());
			}
		}

		// drop the client
		m_clientRegistry->RemoveClient(client);
	}

	void GameServer::ForceHeartbeat()
	{
		m_nextHeartbeatTime = -1;
	}

	void GameServer::SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob, bool prefix)
	{
		std::string oobStr(oob);

		gscomms_execute_callback_on_net_thread([this, to, oobStr, prefix]()
		{
			m_net->SendOutOfBand(to, oobStr, prefix);
		});
	}

	fwRefContainer<GameServerNetBase> CreateGSNet(fx::GameServer* server)
	{
		static auto cmd = server->GetInstance()->AddVariable<std::string>("netlib", ConVar_None, "enet");
		
		if (cmd->GetValue() == "yojimbo")
		{
			return CreateGSNet_Yojimbo(server);
		}
		else if (cmd->GetValue() == "raknet")
		{
			return CreateGSNet_RakNet(server);
		}
		else
		{
			return CreateGSNet_ENet(server);
		}
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

		struct GameServerTick
		{
			inline void operator()(const fwRefContainer<fx::GameServer>& server, int frameTime)
			{
				server->ProcessServerFrame(frameTime);
			}
		};

		struct GetInfoOOB
		{
			inline void Process(const fwRefContainer<fx::GameServer>& server, const net::PeerAddress& from, const std::string_view& data) const
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
			inline void Process(const fwRefContainer<fx::GameServer>& server, const net::PeerAddress& from, const std::string_view& data) const
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
			void Process(const fwRefContainer<fx::GameServer>& server, const net::PeerAddress& from, const std::string_view& dataView) const
			{
				auto limiter = server->GetInstance()->GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter("rcon", fx::RateLimiterDefaults{ 0.2, 5.0 });

				if (!limiter->Consume(from))
				{
					return;
				}

				std::string data(dataView);

				gscomms_execute_callback_on_main_thread([=]()
				{
					try
					{
						int spacePos = data.find_first_of(" \n");

						if (spacePos == std::string::npos)
						{
							return;
						}

						auto password = data.substr(0, spacePos);
						auto command = data.substr(spacePos);

						auto serverPassword = server->GetRconPassword();

						std::string printString;

						ScopeDestructor destructor([&]()
						{
							server->SendOutOfBand(from, "print " + printString);
						});

						if (serverPassword.empty())
						{
							printString += "The server must set rcon_password to be able to use this command.\n";
							return;
						}

						if (password != serverPassword)
						{
							printString += "Invalid password.\n";
							return;
						}

						// log rcon request
						trace("Rcon from %s\n%s\n", from.ToString(), command);

						// reset rate limit for this key
						limiter->Reset(from);

						PrintListenerContext context([&](const std::string_view& print)
						{
							printString += print;
						});

						auto ctx = server->GetInstance()->GetComponent<console::Context>();
						ctx->ExecuteBuffer();

						se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
						ctx->AddToBuffer(std::string(command));
						ctx->ExecuteBuffer();
					}
					catch (std::exception& e)
					{

					}
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
						instance->GetComponent<fx::ServerGameState>()->ParseGameStatePacket(client, packetData);

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

						targetClient->SendPacket(1, outPacket, NetPacketType_Unreliable);

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

void gscomms_execute_callback_on_main_thread(const std::function<void()>& fn, bool force)
{
	g_gameServer->InternalAddMainThreadCb(fn, force);
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

void gscomms_send_packet(const std::shared_ptr<fx::Client>& client, int peer, int channel, const net::Buffer& buffer, NetPacketType flags)
{
	gscomms_execute_callback_on_net_thread([=]()
	{
		g_gameServer->InternalSendPacket(client, peer, channel, buffer, flags);
	});
}

fwRefContainer<fx::NetPeerBase> gscomms_get_peer(int peer)
{
	return g_gameServer->InternalGetPeer(peer);
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		using namespace fx::ServerDecorators;

		instance->SetComponent(new fx::UdpInterceptor());

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

		instance->SetComponent(new fx::PeerAddressRateLimiterStore(instance->GetComponent<console::Context>().GetRef()));
		instance->SetComponent(new fx::ServerDecorators::HostVoteCount());
	});

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto consoleCtx = instance->GetComponent<console::Context>();

		// start sessionmanager
		if (instance->GetComponent<fx::GameServer>()->GetGameName() == fx::GameName::RDR3)
		{
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", "sessionmanager-rdr3" });
		}
		else
		{
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", "sessionmanager" });
		}
	}, INT32_MAX);
});
