#include <StdInc.h>
#include <HttpClient.h>

#include <MakeClientFunction.h>

#include <GameServer.h>
#include <ServerInstanceBase.h>

#include <fmt/chrono.h>
#include <fmt/time.h>
#include <json.hpp>

#include <unordered_set>

#include <ScriptEngine.h>

#include <boost/algorithm/string.hpp>

static HttpClient* httpClient;

static const std::string TEBEX_ENDPOINT = "https://plugin.tebex.io";

using json = nlohmann::json;

class ExtCommerceComponent : public fwRefCountable, public fx::IAttached<fx::ServerInstanceBase>
{
public:
	ExtCommerceComponent();

	void Tick();

private:
	void ProcessClientCommands(fx::Client* client);

	void ExecuteOfflineCommands();

	void ExecuteCommandList(const json& json, int netId = -1, const std::map<std::string, std::string>& additionalTemplateVars = {});

	void AddClientEventToQueue(fx::Client* client, std::string_view eventType);

public:
	void OnClientConnected(fx::Client* client);

	virtual void AttachToObject(fx::ServerInstanceBase* instance) override;

	inline const std::string& GetTebexKey()
	{
		return m_tebexKeyConvar->GetValue();
	}

private:
	fx::ServerInstanceBase* m_instance;

	bool m_polled;

	std::chrono::milliseconds m_nextCheck;

	std::chrono::milliseconds m_nextEvents;

	std::shared_ptr<ConVar<std::string>> m_tebexKeyConvar;

	// only access from svMain!
	std::unordered_set<int64_t> m_removalQueue;

	tbb::concurrent_queue<json> m_eventQueue;

	tbb::concurrent_unordered_map<std::string, std::optional<std::tuple<int, std::string>>> m_playerQueue;

	tbb::concurrent_unordered_map<uint32_t, tbb::concurrent_queue<std::function<bool(fx::Client*)>>> m_commandQueue;
};

DECLARE_INSTANCE_TYPE(ExtCommerceComponent);

ExtCommerceComponent::ExtCommerceComponent()
	: m_nextCheck(0), m_nextEvents(0), m_instance(nullptr), m_polled(false)
{
	
}

static std::string ParseIdentifier(const std::string& id)
{
	if (id.find(":") == std::string::npos)
	{
		return fmt::sprintf("fivem:%s", id);
	}

	return id;
}

void ExtCommerceComponent::ExecuteCommandList(const json& json, int netId /* = -1 */, const std::map<std::string, std::string>& additionalTemplateVars /* = */ )
{
	for (const auto& command : json["commands"])
	{
		auto delay = command["conditions"].value("delay", 0);
		auto executeAfter = msec();

		m_commandQueue[netId].push([this, json, command, delay, additionalTemplateVars, executeAfter](fx::Client* client) mutable
		{
			if (client && !client->HasRouted())
			{
				return false;
			}

			// handle delay after routed
			if (delay > 0)
			{
				executeAfter = msec() + std::chrono::seconds{ delay };
				delay = 0;
			}

			if (msec() < executeAfter)
			{
				return false;
			}

			// enter principal scope
			{
				se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
				auto cmd = command.value("command", "");

				try
				{
					// templated commands
					for (const auto& token : additionalTemplateVars)
					{
						boost::algorithm::replace_all(cmd, fmt::sprintf("{%s}", token.first), token.second);
					}

					const auto& player =
						(command.find("player") != command.end())
							? command["player"]
							: json["player"];

					const auto& oplayer = player.get<json::object_t>();

					if (oplayer.find("uuid") != oplayer.end())
					{
						boost::algorithm::replace_all(cmd, "{id}", player.value<std::string>("uuid", "0"));
					}

					for (const auto& token : oplayer)
					{
						if (token.second.is_object())
						{
							for (const auto& subToken : token.second.get<json::object_t>())
							{
								boost::algorithm::replace_all(cmd, fmt::sprintf("{%s.%s}", token.first, subToken.first), subToken.second.get<std::string>());
							}
						}
						else
						{
							boost::algorithm::replace_all(cmd, fmt::sprintf("{%s}", token.first), token.second.get<std::string>());
						}
					}

					boost::algorithm::replace_all(cmd, "{identifier}", ParseIdentifier(player.value("uuid", "")));
				}
				catch (json::exception & e)
				{
					// TODO: log?
				}

				m_instance->GetComponent<console::Context>()->ExecuteSingleCommand(cmd);
			}

			m_removalQueue.insert(command.value("id", int64_t(0)));

			return true;
		});
	}
}

void ExtCommerceComponent::ProcessClientCommands(fx::Client* client)
{
	for (const auto& id : client->GetIdentifiers())
	{
		auto it = m_playerQueue.find(id);

		if (it != m_playerQueue.end() && it->second)
		{
			auto netId = client->GetNetId();
			const auto& [playerId, username] = *it->second;

			client->OnDrop.Connect([this, netId]
			{
				m_commandQueue[netId].clear();
			});

			HttpRequestOptions opts;
			opts.headers["X-Tebex-Secret"] = m_tebexKeyConvar->GetValue();

			httpClient->DoGetRequest(TEBEX_ENDPOINT + "/queue/online-commands/" + std::to_string(playerId), opts, [this, netId](bool success, const char* resultData, size_t resultLength)
			{
				if (!success)
				{
					return;
				}

				try
				{
					json data = json::parse(resultData, resultData + resultLength);

					std::string steamIdentifier = "";

					auto client = m_instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(netId);

					if (client)
					{
						for (const auto& identifier : client->GetIdentifiers())
						{
							if (identifier.find("steam:") == 0)
							{
								steamIdentifier = identifier;
								break;
							}
						}
					}

					ExecuteCommandList(data, netId, { { "sid", std::to_string(netId) }, { "hexid", steamIdentifier } });
				}
				catch (std::exception & e)
				{
					trace("exception while processing tebex.io online commands: %s\n", e.what());
				}
			});

			m_playerQueue[id] = {};
		}
	}
}

void ExtCommerceComponent::Tick()
{
	if (m_tebexKeyConvar->GetValue().empty())
	{
		return;
	}

	if (!m_polled)
	{
		HttpRequestOptions opts;
		opts.headers["X-Tebex-Secret"] = m_tebexKeyConvar->GetValue();

		httpClient->DoGetRequest(TEBEX_ENDPOINT + "/information", opts, [this](bool success, const char* resultData, size_t resultLength) 
		{
			if (success)
			{
				try
				{
					json data = json::parse(resultData, resultData + resultLength);

					trace("^2Authenticated with Tebex: ^7%s\n", data["account"].value("name", ""));
				}
				catch (std::exception& e)
				{
					trace("exception while processing tebex.io information call: %s\n", e.what());
				}
			}
		});

		m_polled = true;
	}

	if (m_nextCheck < msec())
	{
		m_nextCheck = std::chrono::milliseconds::max();

		HttpRequestOptions opts;
		opts.headers["X-Tebex-Secret"] = m_tebexKeyConvar->GetValue();

		httpClient->DoGetRequest(TEBEX_ENDPOINT + "/queue", opts, [this](bool success, const char* resultData, size_t resultLength)
		{
			if (!success)
			{
				m_nextCheck = msec() + std::chrono::seconds(30);
				return;
			}

			try
			{
				json data = json::parse(resultData, resultData + resultLength);

				if (data["meta"].value("execute_offline", false))
				{
					ExecuteOfflineCommands();
				}

				m_nextCheck = msec() + std::chrono::seconds(data["meta"].value("next_check", 120));

				for (const auto& player : data["players"])
				{
					m_playerQueue[ParseIdentifier(player.value("uuid", ""))] = { { player.value("id", 0), player.value("name", "") } };
				}
			}
			catch (std::exception& e)
			{
				trace("exception while processing tebex.io queue: %s\n", e.what());

				m_nextCheck = msec() + std::chrono::seconds(30);
			}
		});
	}

	if (m_nextEvents < msec())
	{
		m_nextEvents = msec() + 60s;

		while (!m_eventQueue.empty())
		{
			json eventData = json::array();
			json entry;

			while (m_eventQueue.try_pop(entry))
			{
				eventData.push_back(std::move(entry));

				if (eventData.size() >= 750)
				{
					break;
				}
			}

			if (eventData.size() > 0)
			{
				HttpRequestOptions opts;
				opts.headers["X-Tebex-Secret"] = m_tebexKeyConvar->GetValue();

				httpClient->DoPostRequest(TEBEX_ENDPOINT + "/events", eventData.dump(), opts, [this](bool success, const char* resultData, size_t resultLength)
				{
					if (!success)
					{
						trace("posting tebex.io events failed: %s\n", std::string{ resultData, resultLength });
					}
				});
			}
		}
	}

	auto clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();

	auto processQueue = [](auto& queue, const fx::ClientSharedPtr& client)
	{
		std::function<bool(fx::Client*)> fn;
		std::list<std::function<bool(fx::Client*)>> toInsert;

		while (queue.try_pop(fn))
		{
			if (fn && !fn(client.get()))
			{
				toInsert.push_back(std::move(fn));
			}
		}

		for (auto& entry : toInsert)
		{
			queue.push(std::move(entry));
		}
	};

	clientRegistry->ForAllClients([this, processQueue](const fx::ClientSharedPtr& client)
	{
		ProcessClientCommands(client.get());

		auto& queue = m_commandQueue[client->GetNetId()];

		processQueue(queue, client);
	});

	{
		auto& queue = m_commandQueue[-1];

		processQueue(queue, {});
	}

	if (!m_removalQueue.empty())
	{
		HttpRequestOptions opts;
		opts.headers["X-Tebex-Secret"] = m_tebexKeyConvar->GetValue();

		std::stringstream idsArray;

		for (auto id : m_removalQueue)
		{
			idsArray << "ids[]=" << std::to_string(id) << "&";
		}

		auto removedQueue = std::move(m_removalQueue);

		httpClient->DoMethodRequest("DELETE", TEBEX_ENDPOINT + "/queue", idsArray.str(), opts, [this, removedQueue](bool success, const char* resultData, size_t resultLength)
		{
			if (!success)
			{
				gscomms_execute_callback_on_main_thread([this, removedQueue]()
				{
					for (auto id : removedQueue)
					{
						m_removalQueue.insert(id);
					}
				});

				return;
			}
		});
	}
}

void ExtCommerceComponent::ExecuteOfflineCommands()
{
	HttpRequestOptions opts;
	opts.headers["X-Tebex-Secret"] = m_tebexKeyConvar->GetValue();

	httpClient->DoGetRequest(TEBEX_ENDPOINT + "/queue/offline-commands", opts, [this](bool success, const char* resultData, size_t resultLength)
	{
		if (!success)
		{
			return;
		}

		try
		{
			json data = json::parse(resultData, resultData + resultLength);

			ExecuteCommandList(data);
		}
		catch (std::exception & e)
		{
			trace("exception while processing tebex.io offline commands: %s\n", e.what());
		}
	});
}

void ExtCommerceComponent::OnClientConnected(fx::Client* client)
{
	if (m_tebexKeyConvar->GetValue().empty())
	{
		return;
	}

	ProcessClientCommands(client);

	AddClientEventToQueue(client, "server.join");

	client->OnDrop.Connect([this, client]()
	{
		AddClientEventToQueue(client, "server.leave");
	});
}

class ClientExtCommerceComponent : public fwRefCountable
{
public:
	ClientExtCommerceComponent(ExtCommerceComponent* root, fx::Client* client)
		: m_commerceDataLoaded(false), m_client(client), m_root(root)
	{

	}

	inline bool HasCommerceDataLoaded()
	{
		return m_commerceDataLoaded;
	}

	void LoadCommerceData();

	void SetSkus(std::set<int>&& list);

	bool OwnsSku(int sku);

	std::optional<int> GetUserId();

private:
	fx::Client* m_client;

	bool m_commerceDataLoaded;

	std::set<int> m_ownedSkus;

	ExtCommerceComponent* m_root;
};

void ClientExtCommerceComponent::LoadCommerceData()
{
	auto userId = GetUserId();

	if (m_commerceDataLoaded || !userId)
	{
		return;
	}

	fwRefContainer<ClientExtCommerceComponent> thisRef(this);

	HttpRequestOptions opts;
	opts.headers["X-Tebex-Secret"] = m_root->GetTebexKey();

	httpClient->DoGetRequest(fmt::sprintf(TEBEX_ENDPOINT + "/player/%d/packages", *userId), opts, [thisRef](bool success, const char* data, size_t length)
	{
		if (success)
		{
			try
			{
				auto json = nlohmann::json::parse(std::string(data, length));

				std::set<int> skuIds;

				for (auto& entry : json)
				{
					auto& package = entry["package"];

					if (!package.is_null())
					{
						skuIds.insert(package.value<int>("id", 0));
					}
				}

				thisRef->SetSkus(std::move(skuIds));
			}
			catch (const std::exception & e)
			{

			}
		}
	});
}

void ExtCommerceComponent::AddClientEventToQueue(fx::Client* client, std::string_view eventType)
{
	auto id = client->GetComponent<ClientExtCommerceComponent>()->GetUserId();

	if (!id)
	{
		return;
	}

	auto t = std::time(nullptr);

	m_eventQueue.push(json::object({
		{ "username_id", *id },
		{ "username", client->GetName() },
		{ "event_type", eventType },
		{ "event_date", fmt::format("{:%Y-%m-%d %H:%M:%S}", *std::gmtime(&t)) },
		{ "ip", client->GetTcpEndPoint() }
	}));
}


std::optional<int> ClientExtCommerceComponent::GetUserId()
{
	const auto& identifiers = m_client->GetIdentifiers();

	for (const auto& identifier : identifiers)
	{
		if (identifier.find("fivem:") == 0)
		{
			int userId = atoi(identifier.substr(6).c_str());

			if (userId != 0)
			{
				return userId;
			}
		}
	}

	return {};
}

void ClientExtCommerceComponent::SetSkus(std::set<int>&& list)
{
	m_ownedSkus = std::move(list);
	m_commerceDataLoaded = true;
}

bool ClientExtCommerceComponent::OwnsSku(int sku)
{
	return m_ownedSkus.find(sku) != m_ownedSkus.end();
}

DECLARE_INSTANCE_TYPE(ClientExtCommerceComponent);

void ExtCommerceComponent::AttachToObject(fx::ServerInstanceBase* instance)
{
	m_instance = instance;

	m_tebexKeyConvar = instance->AddVariable<std::string>("sv_tebexSecret", ConVar_None, "");

	auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

	clientRegistry->OnClientCreated.Connect([this](const fx::ClientSharedPtr& client)
	{
		client->SetComponent(new ClientExtCommerceComponent(this, client.get()));
	});

	instance->OnInitialConfiguration.Connect([this, instance]()
	{
		auto gameServer = instance->GetComponent<fx::GameServer>();

		gameServer->OnTick.Connect([this]()
		{
			this->Tick();
		});

		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		clientRegistry->OnConnectedClient.Connect([this](fx::Client* client)
		{
			this->OnClientConnected(client);
		});
	});
}

static InitFunction initFunction([]()
{
	httpClient = new HttpClient(L"FXServer/CommerceClient");

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new ExtCommerceComponent());
	}, INT32_MAX);

	fx::ScriptEngine::RegisterNativeHandler("LOAD_PLAYER_COMMERCE_DATA_EXT", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
	{
		auto commerceData = client->GetComponent<ClientExtCommerceComponent>();

		commerceData->LoadCommerceData();

		return commerceData->HasCommerceDataLoaded();
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_COMMERCE_INFO_LOADED_EXT", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
	{
		auto commerceData = client->GetComponent<ClientExtCommerceComponent>();

		return commerceData->HasCommerceDataLoaded();
	}));

	fx::ScriptEngine::RegisterNativeHandler("DOES_PLAYER_OWN_SKU_EXT", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
	{
		auto commerceData = client->GetComponent<ClientExtCommerceComponent>();

		return commerceData->OwnsSku(context.GetArgument<int>(1));
	}));
});
