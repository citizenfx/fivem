#include <StdInc.h>

#include <TcpServer.h>

#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>

#include <NetLibrary.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <msgpack.hpp>

#include <nutsnbolts.h>

#include <botan/base64.h>

#include <tbb/concurrent_queue.h>

#include "rline.pb.h"
#include <ICoreGameInit.h>

extern NetLibrary* g_netLibrary;
extern ICoreGameInit* icgi;

DLL_IMPORT fwEvent<net::TcpServer*> OnConfigureWebSocket;
fwEvent<std::string> OnConductorMessage;

struct Timer
{
	std::chrono::steady_clock::time_point fireAt;
	std::function<void()> fn;

	Timer(std::chrono::steady_clock::time_point fireAt, std::function<void()> fn)
		: fireAt(fireAt), fn(fn)
	{
	}
};
static std::vector<Timer> g_netConductorTimers;
static tbb::concurrent_queue<std::function<void()>> g_mainFrameQueue;

namespace websocketpp
{
	namespace config
	{
		struct core_no_utf8 : public core
		{
			static bool const disable_utf8 = true;
		};
	}
}

class ConductorServer
{
private:
	struct ConnectionData
	{
		net::TcpServerStream* stream;
		websocketpp::server<websocketpp::config::core_no_utf8> server;
		websocketpp::connection<websocketpp::config::core_no_utf8>::ptr conn;

		std::deque<uint8_t> readBuffer;
	};

	std::set<ConnectionData*> datas;

public:
	void Route(const std::string& msg)
	{
		std::vector<char> msgOut(msg.size() + 4);
		*(uint32_t*)(&msgOut[0]) = _byteswap_ulong(0x6F23E446);
		memcpy(&msgOut[4], msg.data(), msg.size());

		for (auto& conn : datas)
		{
			conn->server.send(conn->conn, msgOut.data(), msgOut.size(), websocketpp::frame::opcode::BINARY);
		}
	}

	void AttachTo(net::TcpServer* server)
	{
		server->SetConnectionCallback([this](fwRefContainer<net::TcpServerStream> stream)
		{
			auto dataRef = std::make_shared<ConnectionData>();
			dataRef->stream = stream.GetRef();

			std::weak_ptr<ConnectionData> weakRef = dataRef;

			datas.insert(dataRef.get());

			dataRef->conn = dataRef->server.get_connection();

			dataRef->conn->set_message_handler([weakRef](websocketpp::connection_hdl hdl, websocketpp::server<websocketpp::config::core_no_utf8>::message_ptr msg)
			{
				auto dataRef = weakRef.lock();

				if (dataRef)
				{
					const auto& payload = msg->get_payload();

					if (payload.size() < 5)
					{
						return;
					}

					auto rpcNamespace = _byteswap_ulong(*(uint32_t*)(payload.data()));

					if (rpcNamespace == 0x6F23E446)
					{
						g_mainFrameQueue.push([payload]()
						{
							if (icgi->OneSyncEnabled)
							{
								std::string rawProtobuf(payload.data() + 4, payload.size() - 4);
								OnConductorMessage(rawProtobuf);
								return;
							}

							msgpack::sbuffer nameArgs;
							msgpack::packer<msgpack::sbuffer> packer(nameArgs);

							packer.pack_array(1);
							packer.pack_bin(static_cast<uint32_t>(payload.size() - 4));
							packer.pack_bin_body(payload.data() + 4, static_cast<uint32_t>(payload.size() - 4));

							if (g_netLibrary)
							{
								g_netLibrary->SendNetEvent("__cfx_internal:pbRlScSession", std::string(nameArgs.data(), nameArgs.size()));
							}
						});
					}
					else
					{
						auto rawData = Botan::base64_decode("CiIKIDE4MmEyYzAxNjEwYjEyNDFhMGU3MDZiMmNkMzYyNTZmEhQKEgoQAAAAAAAAAAAAAAAAAAAAAA==");
						std::vector<uint8_t> data(rawData.size() + 4);
						*(uint32_t*)(&data[0]) = _byteswap_ulong(rpcNamespace);
						memcpy(data.data() + 4, rawData.data(), rawData.size());

						dataRef->conn->send(data.data(), data.size(), websocketpp::frame::opcode::BINARY);
					}
				}
			});

			dataRef->conn->set_write_handler([weakRef](websocketpp::connection_hdl, char const* data, size_t size) -> websocketpp::lib::error_code
			{
				auto dataRef = weakRef.lock();

				if (dataRef)
				{
					dataRef->stream->Write(std::string{ data, size });
				}

				return {};
			});

			dataRef->conn->set_close_handler([weakRef](websocketpp::connection_hdl)
			{
				auto dataRef = weakRef.lock();

				if (dataRef)
				{
					dataRef->stream->Close();
				}
			});

			dataRef->conn->start();

			stream->SetReadCallback([dataRef](const std::vector<uint8_t>& data)
			{
				auto& readQueue = dataRef->readBuffer;

				size_t origSize = readQueue.size();
				readQueue.resize(origSize + data.size());

				std::copy(data.begin(), data.end(), readQueue.begin() + origSize);

				size_t didRead = 0;

				do 
				{
					// copy the deque into a vector for data purposes
					std::vector<uint8_t> requestData(readQueue.begin(), readQueue.end());

					didRead = dataRef->conn->read_all(reinterpret_cast<const char*>(&requestData[0]), requestData.size());

					if (didRead > 0)
					{
						readQueue.erase(readQueue.begin(), readQueue.begin() + didRead);
					}
				} while (didRead > 0 && readQueue.size() > 0);
			});

			stream->SetCloseCallback([this, dataRef]()
			{
				//dataRef->conn->eof();
				datas.erase(dataRef.get());
			});
		});
	}
};

static std::shared_ptr<ConductorServer> g_wsServer;
static constexpr int kSlotIndex = 16;

namespace rline
{
template<typename T>
static rline::RpcResponseMessage MakeResponse(const T& data)
{
	rline::RpcResponseMessage response;

	std::string content;
	data.SerializeToString(&content);
	response.mutable_container()->set_content(content);

	return response;
}

static void SendResponse(const std::string& methodName, const std::string& content)
{
	rline::RpcMessage msg;
	msg.mutable_header()->set_methodname(methodName);
	msg.set_content(content);

	std::string buffer;
	msg.SerializeToString(&buffer);
	g_wsServer->Route(buffer);
}
}

namespace rline::handlers
{
	static rline::RpcResponseMessage HandleInitSession(rline::RpcMessage& message)
	{
		rline::InitSessionResponse resp;
		resp.set_sesid(std::string(16, '\0'));
		return MakeResponse(resp);
	}

	static rline::RpcResponseMessage HandleInitPlayer2(rline::RpcMessage& message)
	{
		rline::InitPlayerResult initPlayer;
		initPlayer.set_code(0);
		return MakeResponse(initPlayer);
	}

	static rline::RpcResponseMessage HandleGetRestrictions(rline::RpcMessage& message)
	{
		rline::GetRestrictionsResult getRestrictions;
		getRestrictions.mutable_data();
		return MakeResponse(getRestrictions);
	}

	static rline::RpcResponseMessage HandleConfirmSessionEntered(rline::RpcMessage& message)
	{
		return {};
	}

	static rline::RpcResponseMessage HandleUpdateP2pConnectionQualityInfo(rline::RpcMessage& message)
	{
		return {};
	}

	static rline::RpcResponseMessage HandleUpdateSocialRelationships(rline::RpcMessage& message)
	{
		return {};
	}

	static rline::RpcResponseMessage HandleTransitionToSession(rline::RpcMessage& message)
	{
		rline::TransitionToSessionResult transitionToSession;
		transitionToSession.set_code(1);
		return MakeResponse(transitionToSession);
	}

	static rline::RpcResponseMessage HandleQueueForSession_Seamless(rline::RpcMessage& message)
	{
		rline::QueueForSession_Seamless_Parameters req;
		req.ParseFromString(message.content());

		using namespace std::chrono_literals;
		g_netConductorTimers.emplace_back(std::chrono::steady_clock::now() + 250ms, [req]()
		{
			{
				rline::QueueEntered_Parameters params;
				params.set_queuegroup(69);
				*params.mutable_requestid() = req.requestid();
				params.set_optionflags(req.optionflags());

				std::string content;
				params.SerializeToString(&content);
				SendResponse("QueueEntered", content);
			}

			{
				rline::TransitionReady_PlayerQueue_Parameters params;
				params.mutable_serveruri()->set_url("");
				params.mutable_id()->mutable_value()->set_a(2);
				params.mutable_id()->mutable_value()->set_b(0);
				params.set_serversandbox(0xD656C677);
				params.set_sessiontype(3);
				*params.mutable_requestid() = req.requestid();
				params.mutable_transferid()->mutable_value()->set_a(2);
				params.mutable_transferid()->mutable_value()->set_b(2);

				std::string content;
				params.SerializeToString(&content);
				SendResponse("TransitionReady_PlayerQueue", content);
			}

			g_netConductorTimers.emplace_back(std::chrono::steady_clock::now() + 50ms, [req]()
			{
				rline::scmds_Parameters scmds;
				scmds.mutable_sid()->mutable_value()->set_a(2);
				scmds.mutable_sid()->mutable_value()->set_b(2);
				scmds.set_ncmds(1);

				auto* cmd = scmds.add_cmds();
				cmd->set_cmd(0);
				cmd->set_cmdname("EnterSession");

				auto* enter = cmd->mutable_entersession();
				enter->set_index(kSlotIndex);
				enter->set_hindex(kSlotIndex);
				enter->set_sessionflags(0);
				enter->set_mode(0);
				enter->set_size(0);
				enter->set_teamindex(0);
				enter->mutable_transitionid()->mutable_value()->set_a(2);
				enter->mutable_transitionid()->mutable_value()->set_b(0);
				enter->set_sessionmanagertype(0);
				enter->set_slotcount(32);

				std::string content;
				scmds.SerializeToString(&content);
				SendResponse("scmds", content);
			});
		});

		rline::QueueForSessionResult resp;
		resp.set_code(1);
		return MakeResponse(resp);
	}
}

using HandlerFn = std::function<rline::RpcResponseMessage(rline::RpcMessage&)>;
static const std::unordered_map<std::string, HandlerFn> g_handlers = {
	{ "InitSession", rline::handlers::HandleInitSession },
	{ "InitPlayer2", rline::handlers::HandleInitPlayer2 },
	{ "GetRestrictions", rline::handlers::HandleGetRestrictions },
	{ "ConfirmSessionEntered", rline::handlers::HandleConfirmSessionEntered },
	{ "TransitionToSession", rline::handlers::HandleTransitionToSession },
	{ "QueueForSession_Seamless", rline::handlers::HandleQueueForSession_Seamless },
	{ "UpdateSocialRelationships", rline::handlers::HandleUpdateSocialRelationships },
	{ "UpdateP2pConnectionQualityInfo", rline::handlers::HandleUpdateP2pConnectionQualityInfo }
};

static std::string HandleMessage(const std::string& methodName, rline::RpcMessage& message)
{
    auto it = g_handlers.find(methodName);
	if (it == g_handlers.end())
	{
		return "";
	}

	auto response = it->second(message);
	if (!response.has_header() && !response.has_container())
	{
		return "";
	}

	response.mutable_header()->set_requestid(message.header().requestid());   
	std::string outBuffer;
	response.SerializeToString(&outBuffer);
	return outBuffer;
}

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		std::function<void()> fn;

		while (g_mainFrameQueue.try_pop(fn))
		{
			fn();
		}
		
		if (g_netConductorTimers.size() > 0)
		{
			auto now = std::chrono::steady_clock::now();
			for (size_t i = 0; i < g_netConductorTimers.size(); i++)
			{
				if (now >= g_netConductorTimers[i].fireAt)
				{
					auto fn = std::move(g_netConductorTimers[i].fn);
					g_netConductorTimers.erase(g_netConductorTimers.begin() + i);
					i--;
					fn();
				}
			}
		}
	});

	g_wsServer = std::make_shared<ConductorServer>();

	OnConductorMessage.Connect([](const std::string& eventPayload)
	{
		rline::RpcMessage message;
		if (!message.ParseFromArray(eventPayload.data(), static_cast<int>(eventPayload.size())))
		{
			return;
		}

		const std::string& methodName = message.header().methodname();
		std::string response = HandleMessage(methodName, message);
		if (!response.empty())
		{
			g_wsServer->Route(response);
		}
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		fwRefContainer<fx::ResourceEventManagerComponent> eventComponent = manager->GetComponent<fx::ResourceEventManagerComponent>();

		if (eventComponent.GetRef())
		{
			eventComponent->OnQueueEvent.Connect([](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource)
			{
				// if this is the event 'we' handle...
				if (!icgi->OneSyncEnabled && eventName == "__cfx_internal:pbRlScSession")
				{
					// deserialize the arguments
					msgpack::unpacked msg;
					msgpack::unpack(msg, eventPayload.c_str(), eventPayload.size());

					msgpack::object obj = msg.get();

					// convert to an array
					std::vector<msgpack::object> arguments;
					obj.convert(arguments);

					g_wsServer->Route(arguments[0].as<std::string>());
				}
			});
		}
	});

	OnConfigureWebSocket.Connect([](net::TcpServer* server)
	{
		g_wsServer->AttachTo(server);
	});
});
