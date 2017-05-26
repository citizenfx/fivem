#include "StdInc.h"

#include <ResourceManager.h>

#include <GameServer.h>

#include <ClientRegistry.h>
#include <ServerInstanceBaseRef.h>

#include <ScriptEngine.h>

inline static uint64_t msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

static InitFunction initFunction([]()
{
	auto makeClientFunction = [](auto fn)
	{
		return [=](fx::ScriptContext& context)
		{
			// get the current resource manager
			auto resourceManager = fx::ResourceManager::GetCurrent();

			// get the owning server instance
			auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

			// get the server's client registry
			auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

			// parse the client ID
			std::string id = context.GetArgument<const char*>(0);

			uint16_t netId;

			netId = atoi(id.c_str());

			auto client = clientRegistry->GetClientByNetID(netId);

			if (!client)
			{
				context.SetResult(nullptr);
				return;
			}

			context.SetResult(fn(context, client));
		};
	};

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_NAME", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return client->GetName().c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_GUID", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return "";
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_PLAYER_IDENTIFIERS", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return client->GetIdentifiers().size();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_IDENTIFIER", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		int idx = context.GetArgument<int>(1);

		if (idx < 0 || idx >= client->GetIdentifiers().size())
		{
			return (const char*)nullptr;
		}

		return client->GetIdentifiers()[idx].c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_ENDPOINT", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		static thread_local std::string str;
		str = client->GetTcpEndPoint();

		return str.c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_PING", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		auto peer = client->GetPeer();

		if (!peer)
		{
			return -1;
		}

		return (int)peer->lastRoundTripTime;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_LAST_MSG", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return msec() - client->GetLastSeen().count();
	}));

	fx::ScriptEngine::RegisterNativeHandler("DROP_PLAYER", makeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the game server
		auto server = instance->GetComponent<fx::GameServer>();

		server->DropClient(client, context.GetArgument<const char*>(1));

		return true;
	}));

	static thread_local std::vector<std::weak_ptr<fx::Client>> clients;

	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_PLAYER_INDICES", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the client registry
		auto registry = instance->GetComponent<fx::ClientRegistry>();

		registry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
		{
			if (client->GetNetId() >= 0xFFFF)
			{
				return;
			}

			clients.push_back(client);
		});

		context.SetResult(clients.size());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_FROM_INDEX", [](fx::ScriptContext& context)
	{
		int i = context.GetArgument<int>(0);
		if (i < 0 || i >= clients.size())
		{
			context.SetResult(nullptr);
			return;
		}

		if (clients[i].expired())
		{
			context.SetResult(nullptr);
			return;
		}

		static thread_local std::string clientId;
		clientId = fmt::sprintf("net:%d", clients[i].lock()->GetNetId());

		context.SetResult(clientId.c_str());
	});
});
