#include "StdInc.h"

#include <ResourceManager.h>

#include <GameServer.h>

#include <ClientRegistry.h>
#include <ServerInstanceBaseRef.h>

#include <ScriptEngine.h>

#include <se/Security.h>

#include <MakeClientFunction.h>

static void CreatePlayerCommands();

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase*)
	{
		CreatePlayerCommands();
	});
});

static void CreatePlayerCommands()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_NAME", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return client->GetName().c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_GUID", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return client->GetGuid().c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_PLAYER_IDENTIFIERS", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return client->GetIdentifiers().size();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_IDENTIFIER", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		int idx = context.GetArgument<int>(1);

		if (idx < 0 || idx >= client->GetIdentifiers().size())
		{
			return (const char*)nullptr;
		}

		return client->GetIdentifiers()[idx].c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_ENDPOINT", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		static thread_local std::string str;
		str = client->GetTcpEndPoint();

		return str.c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_PING", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		auto peer = gscomms_get_peer(client->GetPeer());

		if (!peer.GetRef())
		{
			return -1;
		}

		return int(peer->GetPing());
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_LAST_MSG", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		return (msec() - client->GetLastSeen()).count();
	}, 0x7fffffff));

	fx::ScriptEngine::RegisterNativeHandler("DROP_PLAYER", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		// don't allow dropping of a player that hasn't finished connecting/configuring
		if (client->GetNetId() > 0xFFFF)
		{
			return false;
		}

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the game server
		auto server = instance->GetComponent<fx::GameServer>();

		server->DropClient(client, context.CheckArgument<const char*>(1));

		return true;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_ACE_ALLOWED", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client)
	{
		const char* object = context.CheckArgument<const char*>(1);

		se::ScopedPrincipalReset reset;

		std::vector<std::unique_ptr<se::ScopedPrincipal>> principals;

		for (auto& identifier : client->GetIdentifiers())
		{
			principals.emplace_back(std::make_unique<se::ScopedPrincipal>(se::Principal{ fmt::sprintf("identifier.%s", identifier) }));
		}

		return seCheckPrivilege(object);
	}));

	static thread_local std::vector<std::weak_ptr<fx::Client>> clients;

	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_PLAYER_INDICES", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// clear the old list
		clients.clear();

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
		clientId = fmt::sprintf("%d", clients[i].lock()->GetNetId());

		context.SetResult(clientId.c_str());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_HOST_ID", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the client registry
		auto registry = instance->GetComponent<fx::ClientRegistry>();

		// get the host
		auto host = registry->GetHost();

		if (!host)
		{
			context.SetResult(nullptr);
		}
		else
		{
			static thread_local std::string id;
			id = std::to_string(host->GetNetId());

			context.SetResult(id.c_str());
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_PED", MakeClientFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::Client>& client) -> uint32_t
	{
		try
		{
			return std::any_cast<uint32_t>(client->GetData("playerEntity"));
		}
		catch (std::bad_any_cast&)
		{
			return 0;
		}
	}));
}
