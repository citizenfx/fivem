#include "StdInc.h"
#include "SdkIpc.h"

#include <chrono>

#include <Resource.h>
#include <ResourceMonitor.h>
#include <ResourceMetaDataComponent.h>

#include <CoreConsole.h>
#include <ComponentHolder.h>
#include <se/Security.h>

#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_queue.h"

#include <skyr/url.hpp>
#include <json.hpp>

using namespace std::chrono_literals;

static std::function<void(const ConsoleChannel&, const std::string&)> g_consoleMessageHandler;
static std::vector<std::pair<ConsoleChannel, std::string>> g_messagesQueue;

static void LogPrintListener(ConsoleChannel channel, const char* rawMessage)
{
	auto message = fmt::sprintf("%s", rawMessage);

	if (!g_consoleMessageHandler)
	{
		g_messagesQueue.push_back({ channel, message });
	}
	else
	{
		g_consoleMessageHandler(channel, message);
	}
}

static uint32_t readJsonSize(std::deque<uint8_t>& buffer)
{
	uint8_t size[4] = { buffer[0], buffer[1], buffer[2], buffer[3] };

	buffer.erase(buffer.begin(), buffer.begin() + 4);

	return *(uint32_t*)(size);
}

inline static std::shared_ptr<uvw::Loop> GetSdkIpcLoop()
{
	return Instance<net::UvLoopManager>::Get()->GetOrCreate("sdkIpc")->Get();
}

// tuple slice from https://stackoverflow.com/a/40836163/10995747
namespace detail
{
template<std::size_t Ofst, class Tuple, std::size_t... I>
constexpr auto slice_impl(Tuple&& t, std::index_sequence<I...>)
{
	return std::forward_as_tuple(
	std::get<I + Ofst>(std::forward<Tuple>(t))...);
}
}

template<std::size_t I1, std::size_t I2, class Cont>
constexpr auto tuple_slice(Cont&& t)
{
	static_assert(I2 >= I1, "invalid slice");
	static_assert(std::tuple_size<std::decay_t<Cont>>::value >= I2,
	"slice index out of bounds");

	return detail::slice_impl<I1>(std::forward<Cont>(t),
	std::make_index_sequence<I2 - I1>{});
}

namespace fxdk
{
	using json = nlohmann::json;

	enum class IpcPacketType
	{
		RPC_CALL,
		RPC_RESPONSE,
		RPC_ERROR_RESPONSE,
		EVENT,
	};

	class IpcConnection : public fwRefCountable
	{
	private:
		std::string m_pipeName;
		std::shared_ptr<uvw::PipeHandle> m_pipe;
		std::shared_ptr<uvw::AsyncHandle> m_asyncEmitPackets;
		tbb::concurrent_queue<std::pair<std::unique_ptr<char[]>, size_t>> m_packetsQueue;

	public:
		typedef std::function<void(const json&, const uint32_t, fwRefContainer<IpcConnection>)> TRpcHandler;
		typedef std::function<void(const json&, fwRefContainer<IpcConnection>)> TEventHandler;

		fwEvent<const std::string&> OnQuit;

	private:
		std::unordered_map<std::string, TRpcHandler> m_rpcHandlers;
		std::unordered_map<std::string, TEventHandler> m_eventHandlers;
	public:
		void OnRpc(const std::string& name, TRpcHandler&& handler)
		{
			m_rpcHandlers.insert(std::make_pair(name, handler));
		}
		void OnEvent(const std::string& name, TEventHandler&& handler)
		{
			m_eventHandlers.insert(std::make_pair(name, handler));
		}

	public:
		IpcConnection(const std::string& pipeName)
			: m_pipeName(pipeName)
		{
			
		}

		~IpcConnection()
		{
			if (m_pipe) {
				m_pipe->stop();
			}
		}

		void Start()
		{
			fwRefContainer<IpcConnection> thisRef(this);

			m_asyncEmitPackets = GetSdkIpcLoop()->resource<uvw::AsyncHandle>();

			m_asyncEmitPackets->on<uvw::AsyncEvent>([this](const uvw::AsyncEvent& evt, uvw::AsyncHandle& loop)
			{
				std::pair<std::unique_ptr<char[]>, size_t> packet;

				while (m_packetsQueue.try_pop(packet))
				{
					m_pipe->write(std::move(packet.first), packet.second);
				}
			});

			m_pipe = GetSdkIpcLoop()->resource<uvw::PipeHandle>();

			m_pipe->on<uvw::ErrorEvent>([thisRef](const uvw::ErrorEvent& evt, uvw::PipeHandle& handle)
			{
				thisRef->OnQuit(fmt::sprintf("IPC channel error: %s", thisRef->m_pipeName, evt.what()));
			});

			m_pipe->on<uvw::EndEvent>([thisRef](const uvw::EndEvent& evt, uvw::PipeHandle& handle)
			{
				thisRef->OnQuit("IPC channel has been closed remotely");
			});

			m_pipe->on<uvw::DataEvent>([thisRef](const uvw::DataEvent& evt, uvw::PipeHandle& handle)
			{
				if (evt.length == 0)
				{
					return;
				}

				// according to protocol we can't have event data of size < 4 bytes
				if (evt.length < 4 && thisRef->m_remainingBytesToRead > 0)
				{
					return;
				}

				std::vector<uint8_t> data(evt.length);

				memcpy(data.data(), evt.data.get(), evt.length);

				thisRef->HandleUvDataEvent(data);
			});

			m_pipe->on<uvw::ConnectEvent>([thisRef](const uvw::ConnectEvent&, uvw::PipeHandle& handle)
			{
				handle.read();
			});

			m_pipe->connect(m_pipeName);
		}

		void SendErrorResponse(const uint32_t eventId, const std::string& error)
		{
			json packet;

			packet[0] = IpcPacketType::RPC_ERROR_RESPONSE;
			packet[1] = eventId;
			packet[2] = error;

			EmitPacket(packet);
		}

		void SendResponse(const uint32_t eventId, const json& retval)
		{
			json packet;

			packet[0] = IpcPacketType::RPC_RESPONSE;
			packet[1] = eventId;
			packet[2] = retval;

			EmitPacket(packet);
		}

		void EmitEvent(const std::string& eventName, const json& payload = {})
		{
			json packet;

			packet[0] = IpcPacketType::EVENT;
			packet[1] = eventName;
			packet[2] = payload;

			EmitPacket(packet);
		}

	private:
		std::deque<uint8_t> m_dataBuffer;

		uint32_t m_pendingJsonSize = 0;
		uint32_t m_remainingBytesToRead = 0;

		std::mutex m_readMutex;

		// [uint32_t dataLength][uint8_t jsondoc[dataLength]]
		void HandleUvDataEvent(const std::vector<uint8_t>& data)
		{
			std::lock_guard<std::mutex> lock(m_readMutex);

			//trace("== 0: Packet arrived of size: %d, buffer size: %d\n", data.size(), m_dataBuffer.size());

			std::copy(data.begin(), data.end(), std::inserter(m_dataBuffer, m_dataBuffer.end()));

			while (m_dataBuffer.size() >= m_pendingJsonSize)
			{
				if (m_dataBuffer.size() == 0)
				{
					return;
				}

				if (m_pendingJsonSize == 0)
				{
					if (m_dataBuffer.size() < 4)
					{
						return;
					}

					m_pendingJsonSize = readJsonSize(m_dataBuffer);
					//trace("== 1: Pending json size: %d\n", m_pendingJsonSize);
				}

				if (m_pendingJsonSize > m_dataBuffer.size()) {
					return;
				}

				//trace("== 2: Reading json of size: %d\n", m_pendingJsonSize);

				auto jsonBegin = m_dataBuffer.begin();
				auto jsonEnd = jsonBegin + m_pendingJsonSize;
				std::vector<uint8_t> json(jsonBegin, jsonEnd);

				m_dataBuffer.erase(jsonBegin, jsonEnd);

				ProcessJSONPacket(json);

				m_pendingJsonSize = 0;
			}
		}

		void ProcessJSONPacket(const std::vector<uint8_t>& data)
		{
			//trace("BUF SIZE: %d | PACKET RAW: %s\n", m_rcvbuf.size(), std::string(data.begin(), data.end()));

			json jsonData = json::parse(data);

			if (!jsonData.is_array())
			{
				OnQuit("Invalid IPC JSON packet, expected array");
				return;
			}

			IpcPacketType packetType = jsonData[0];

			if (packetType == IpcPacketType::RPC_CALL)
			{
				if (jsonData.size() != 4)
				{
					OnQuit("Invalid IPC JSON RPC packet array length, expected 4");
					return;
				}

				std::string procName = jsonData[1];
				uint32_t callId = jsonData[2];
				json payload = jsonData[3];

				auto handler = m_rpcHandlers.find(procName);
				if (handler != m_rpcHandlers.end())
				{
					handler->second(payload, callId, fwRefContainer<IpcConnection>(this));
				}
			}
			else if (packetType == IpcPacketType::EVENT)
			{
				if (jsonData.size() != 3)
				{
					OnQuit("Invalid IPC event JSON packet array length, expected 3");
					return;
				}

				std::string eventName = jsonData[1];
				json payload = jsonData[2];

				auto handler = m_eventHandlers.find(eventName);
				if (handler != m_eventHandlers.end())
				{
					handler->second(payload, fwRefContainer<IpcConnection>(this));
				}
			}
		}

		void EmitEventResponse(const uint32_t eventId, const bool isError, const json& retval)
		{
			json packet;

			packet[0] = isError ? IpcPacketType::RPC_ERROR_RESPONSE : IpcPacketType::RPC_RESPONSE;
			packet[1] = eventId;
			packet[2] = retval;

			EmitPacket(packet);
		}

		void EmitPacket(const json& data)
		{
			std::string dataStr = data.dump();
			size_t dataStrSize = dataStr.size();
			if (dataStrSize > UINT32_MAX)
			{
				OnQuit("Exactly why would you need to send >4GB packet?!");
				return;
			}

			size_t packetSize = dataStrSize + 4;

			std::unique_ptr<char[]> packet{ new char[packetSize] };

			// hacky-wacky writing data size
			*(uint32_t*)packet.get() = (uint32_t)dataStrSize;

			memcpy(packet.get() + 4, dataStr.data(), dataStrSize);

			m_packetsQueue.push({ std::move(packet), packetSize });

			m_asyncEmitPackets->send();
		}
	};

	typedef fwRefContainer<IpcConnection> IpcRef;

	void SdkIpc::Quit(const std::string& error)
	{
		trace("\n^1ERROR: %s^0\n", error);

		se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

		g_consoleMessageHandler = nullptr;

		GetConsole()->ExecuteSingleCommandDirect(ProgramArguments{ "quit", "SdkIpc error" });
	}

	SdkIpc::SdkIpc(fwRefContainer<fx::ServerInstanceBase> instance, const std::string& pipeAppendix)
		: m_instance(instance),
		m_pipeName(fmt::sprintf("\\\\.\\pipe\\cfx-fxdk-fxserver-ipc-%s", pipeAppendix))
	{
		instance->SetComponent<IpcConnection>(new IpcConnection(m_pipeName));

		fwRefContainer<IpcConnection> ipc = instance->GetComponent<IpcConnection>();
		fwRefContainer<SdkIpc> thisRef(this);

		ipc->OnQuit.Connect([thisRef](const std::string& error)
		{
			thisRef->Quit(error);
			thisRef->NotifyInitialized();
		});

		ipc->OnEvent("cmd", [thisRef](const json& payload, IpcRef)
		{
			trace("cmd: %s\n", payload);
			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
			thisRef->GetConsole()->ExecuteSingleCommand(payload);
		});

		ipc->OnEvent("initDone", [thisRef](const json&, IpcRef ipc)
		{
			trace("Initialized\n");
			thisRef->NotifyInitialized();
		});

		ipc->OnRpc("load", [thisRef](const json& payload, const uint32_t eventId, IpcRef ipc)
		{
			if (!payload.is_array())
			{
				ipc->SendErrorResponse(eventId, "payload must be an array");
				return;
			}

			trace("Loading resources:\n");

			std::vector<pplx::task<fwRefContainer<fx::Resource>>> resourceTasks;

			std::shared_ptr<tbb::concurrent_unordered_map<std::string, bool>> resourceLoadResults = std::make_shared<tbb::concurrent_unordered_map<std::string, bool>>(0);

			for (const std::string& resourceUrl : payload)
			{
				auto parsed = skyr::make_url(resourceUrl);
				if (parsed)
				{
					std::string resourceName = parsed->hash().substr(1);

					if (thisRef->GetResourceManager()->GetResource(resourceName).GetRef())
					{
						trace(" - already-loaded %s\n", resourceUrl);
						resourceLoadResults->insert(std::make_pair(resourceUrl, true));
					}
					else
					{
						auto task = thisRef->GetResourceManager()->AddResource(resourceUrl).then([ipc, resourceName, resourceUrl, resourceLoadResults](fwRefContainer<fx::Resource> resource)
						{
							if (resource.GetRef())
							{
								trace(" - loaded %s\n", resourceUrl);
								resourceLoadResults->insert(std::make_pair(resourceUrl, true));
							}
							else
							{
								trace(" - failed-to-load %s\n", resourceUrl);
								resourceLoadResults->insert(std::make_pair(resourceUrl, false));
							}

							return resource;
						});

						resourceTasks.push_back(task);
					}
				}
				else
				{
					resourceLoadResults->insert(std::make_pair(resourceUrl, false));
				}
			}

			pplx::when_all(resourceTasks.begin(), resourceTasks.end()).wait();

			trace(" . Done loading resources\n");

			ipc->SendResponse(eventId, std::unordered_map(resourceLoadResults->begin(), resourceLoadResults->end()));
		});

		ipc->OnRpc("unload", [thisRef](const json& payload, const uint32_t eventId, IpcRef ipc)
		{
			if (!payload.is_array())
			{
				ipc->SendErrorResponse(eventId, "payload must be an array");
				return;
			}

			trace("Unloading resources:\n");

			for (const std::string& resourceName : payload)
			{
				fwRefContainer<fx::Resource> resource = thisRef->GetResourceManager()->GetResource(resourceName, false);

				if (resource.GetRef())
				{
					trace(" - unloaded %s\n", resourceName);
					resource->Stop();

					thisRef->GetResourceManager()->RemoveResource(resource);
				}
				else
				{
					trace(" - not-even-loaded %s\n", resourceName);
				}
			}

			trace(" . Done unloading resources\n");

			ipc->SendResponse(eventId, {});
		});

		ipc->OnRpc("reload", [thisRef](const json& payload, const uint32_t eventId, IpcRef ipc)
		{
			if (!payload.is_array())
			{
				ipc->SendErrorResponse(eventId, "payload must be an array");
				return;
			}

			trace("Reloading resources:\n");

			for (const std::string& resourceName : payload)
			{
				fwRefContainer<fx::Resource> resource = thisRef->GetResourceManager()->GetResource(resourceName);
				if (resource.GetRef())
				{
					resource->GetComponent<fx::ResourceMetaDataComponent>()->LoadMetaData(resource->GetPath());
					trace(" - reloaded %s\n", resourceName);
				}
				else
				{
					trace(" - not-even-loaded %s\n", resourceName);
				}
			}

			trace(" . Done reloading resources\n");

			ipc->SendResponse(eventId, {});
		});

		ipc->OnRpc("restart", [thisRef](const json& payload, const uint32_t eventId, IpcRef ipc)
		{
			if (payload.is_string())
			{
				trace("Restarting resource: %s\n", payload);

				std::string error;
				bool success = thisRef->RestartResource(payload, error);

				if (error.empty())
				{
					ipc->SendResponse(eventId, success);
				}
				else
				{
					ipc->SendErrorResponse(eventId, error);
				}
			}
			else if (payload.is_array())
			{
				std::vector<std::string> resourceNames = payload;

				trace("Restarting %d resources\n", resourceNames.size());

				json result;
				int resultIdx = 0;

				for (const std::string& resourceName : resourceNames)
				{
					std::string error;
					bool success = thisRef->RestartResource(resourceName, error);

					if (error.empty())
					{
						result[resultIdx++] = { resourceName, success };
					}
					else
					{
						result[resultIdx++] = { resourceName, error };
					}
				}

				ipc->SendResponse(eventId, result);
			}
			else
			{
				ipc->SendErrorResponse(eventId, "Payload must be a string or array of strings");
			}
		});

		ipc->OnRpc("start", [thisRef](const json& payload, const uint32_t eventId, IpcRef ipc)
		{
			if (payload.is_string()) {
				trace("Starting resource: %s\n", payload);

				std::string error;
				bool success = thisRef->StartResource(payload, error);

				if (error.empty())
				{
					ipc->SendResponse(eventId, success);
				}
				else
				{
					ipc->SendErrorResponse(eventId, error);
				}
			}
			else if (payload.is_array())
			{
				std::vector<std::string> resourceNames = payload;

				trace("Starting %d resources\n", resourceNames.size());

				json result;
				int resultIdx = 0;

				for (const std::string& resourceName : resourceNames)
				{
					std::string error;
					bool success = thisRef->StartResource(resourceName, error);

					if (error.empty())
					{
						result[resultIdx++] = { resourceName, success };
					}
					else
					{
						result[resultIdx++] = { resourceName, error };
					}
				}

				ipc->SendResponse(eventId, result);
			}
			else
			{
				ipc->SendErrorResponse(eventId, "Payload must be a string or array of strings");
			}
		});

		ipc->OnRpc("stop", [thisRef](const json& payload, const uint32_t eventId, IpcRef ipc)
		{
			if (payload.is_string())
			{
				trace("Stopping resource: %s\n", payload);

				std::string error;
				bool success = thisRef->StopResource(payload, error);

				if (error.empty())
				{
					ipc->SendResponse(eventId, success);
				}
				else
				{
					ipc->SendErrorResponse(eventId, error);
				}
			}
			else if (payload.is_array())
			{
				std::vector<std::string> resourceNames = payload;

				trace("Stopping %d resources\n", resourceNames.size());

				json result;
				int resultIdx = 0;

				for (const std::string& resourceName : resourceNames)
				{
					std::string error;
					bool success = thisRef->StopResource(resourceName, error);

					if (error.empty())
					{
						result[resultIdx++] = { resourceName, success };
					}
					else
					{
						result[resultIdx++] = { resourceName, error };
					}
				}

				ipc->SendResponse(eventId, result);
			}
			else
			{
				ipc->SendErrorResponse(eventId, "Payload must be a string or array of strings");
			}
		});

		ipc->OnRpc("cmdlist", [thisRef](const json&, const uint32_t eventId, IpcRef ipc)
		{
			ConsoleCommandManager* commandManager = thisRef->GetConsole()->GetCommandManager();
			ConsoleVariableManager* variableManager = thisRef->GetConsole()->GetVariableManager();

			std::set<std::string, console::IgnoreCaseLess> commands;

			commandManager->ForAllCommands([thisRef, &commands](const std::string& cname)
			{
				commands.insert(cname);
			});

			json cmdlist;
			int cmdidx = 0;

			for (const auto& cname : commands)
			{
				// check if this cname is actually variable
				auto cvar = variableManager->FindEntryRaw(cname);
				auto idx = cmdidx++;

				if (cvar)
				{
					cmdlist[idx] = {
						{"type", "var"},
						{"name", cname},
						{"value", cvar->GetValue()}
					};
				}
				else
				{
					cmdlist[idx] = {
						{"type", "cmd"},
						{"name", cname}
					};
				}
			}

			ipc->SendResponse(eventId, cmdlist);
		});

		fx::Resource::OnInitializeInstance.Connect([thisRef, ipc](fx::Resource* resource)
		{
			resource->OnStart.Connect([ipc, resource]()
			{
				ipc->EmitEvent("resource-start", resource->GetName());
			});

			resource->OnStop.Connect([ipc, resource]()
			{
				ipc->EmitEvent("resource-stop", resource->GetName());
			});
		});

		ipc->Start();

		g_consoleMessageHandler = [ipc](const ConsoleChannel& channel, const std::string& message)
		{
			ipc->EmitEvent("stdout", { channel, message });
		};

		// Flush buffered messages
		for (const auto& [channel, message] : g_messagesQueue)
		{
			ipc->EmitEvent("stdout", { channel, message });
		}

		g_messagesQueue.clear();

		m_timer = GetSdkIpcLoop()->resource<uvw::TimerHandle>();

		m_timer->on<uvw::TimerEvent>([ipc](const uvw::TimerEvent& evt, uvw::TimerHandle&)
		{
			const auto& resourceDatas = fx::ResourceMonitor::GetCurrent()->GetResourceDatas();

			std::vector<std::tuple<std::string, double, double, int64_t, int64_t>> resourceDatasClean;
			for (const auto& data : resourceDatas)
			{
				resourceDatasClean.push_back(tuple_slice<0, std::tuple_size_v<decltype(resourceDatasClean)::value_type>>(data));
			}

			ipc->EmitEvent("resource-datas", resourceDatasClean);
		});
	}

	SdkIpc::~SdkIpc()
	{
		m_timer->stop();
		m_inited.signal();
	}

	bool SdkIpc::StartResource(const std::string& resourceName, std::string& error)
	{
		fwRefContainer<fx::Resource> resource = GetResourceManager()->GetResource(resourceName);

		if (resource.GetRef())
		{
			if (resource->GetState() != fx::ResourceState::Started)
			{
				resource->Start();
			}

			return resource->GetState() == fx::ResourceState::Started;
		}
		else
		{
			error = "resource is not loaded";
		}

		return false;
	}

	bool SdkIpc::StopResource(const std::string& resourceName, std::string& error)
	{
		fwRefContainer<fx::Resource> resource = GetResourceManager()->GetResource(resourceName);

		if (resource.GetRef())
		{
			if (resource->GetState() != fx::ResourceState::Stopped)
			{
				resource->Stop();
			}

			return resource->GetState() == fx::ResourceState::Stopped;
		}
		else
		{
			error = "resource is not loaded";
		}

		return false;
	}

	bool SdkIpc::RestartResource(const std::string& resourceName, std::string& error)
	{
		fwRefContainer<fx::Resource> resource = GetResourceManager()->GetResource(resourceName);

		if (resource.GetRef())
		{
			resource->Stop();
			resource->Start();

			return resource->GetState() == fx::ResourceState::Started;
		}
		else
		{
			error = "resource is not loaded";
		}

		return false;
	}

	void SdkIpc::WaitForInitialized()
	{
		m_inited.wait();
	}

	void SdkIpc::NotifyStarted()
	{
		m_instance->GetComponent<IpcConnection>()->EmitEvent("started");
	}

	void SdkIpc::NotifyReady()
	{
		m_timer->start(0ms, 333ms);
		m_instance->GetComponent<IpcConnection>()->EmitEvent("ready");
	}

	void SdkIpc::NotifyInitialized()
	{
		m_inited.signal();
	}
}

DECLARE_INSTANCE_TYPE(fxdk::IpcConnection);

static InitFunction initFunction([]()
{
	console::CoreAddPrintListener(LogPrintListener);
}, INT32_MIN);
