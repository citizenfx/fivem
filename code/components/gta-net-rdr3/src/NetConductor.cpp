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

extern NetLibrary* g_netLibrary;

DLL_IMPORT fwEvent<net::TcpServer*> OnConfigureWebSocket;

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
							msgpack::sbuffer nameArgs;
							msgpack::packer<msgpack::sbuffer> packer(nameArgs);

							packer.pack_array(1);
							packer.pack_bin(payload.size() - 4);
							packer.pack_bin_body(payload.data() + 4, payload.size() - 4);

							if (g_netLibrary)
							{
								g_netLibrary->SendNetEvent("__cfx_internal:pbRlScSession", std::string(nameArgs.data(), nameArgs.size()), -2);
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

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		std::function<void()> fn;

		while (g_mainFrameQueue.try_pop(fn))
		{
			fn();
		}
	});

	static auto wsServer = std::make_shared<ConductorServer>();

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		fwRefContainer<fx::ResourceEventManagerComponent> eventComponent = manager->GetComponent<fx::ResourceEventManagerComponent>();

		if (eventComponent.GetRef())
		{
			eventComponent->OnQueueEvent.Connect([](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource)
			{
				// if this is the event 'we' handle...
				if (eventName == "__cfx_internal:pbRlScSession")
				{
					// deserialize the arguments
					msgpack::unpacked msg;
					msgpack::unpack(msg, eventPayload.c_str(), eventPayload.size());

					msgpack::object obj = msg.get();

					// convert to an array
					std::vector<msgpack::object> arguments;
					obj.convert(arguments);

					wsServer->Route(arguments[0].as<std::string>());
				}
			});
		}
	});


	OnConfigureWebSocket.Connect([](net::TcpServer* server)
	{
		wsServer->AttachTo(server);
	});
});
