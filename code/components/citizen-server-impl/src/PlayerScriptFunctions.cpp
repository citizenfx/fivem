#include "StdInc.h"

#include <ResourceManager.h>

#include <GameServer.h>

#include <ClientRegistry.h>
#include <ServerInstanceBaseRef.h>

#include <ScriptEngine.h>

#include <se/Security.h>

#include <MakeClientFunction.h>

#include <state/ServerGameState.h>


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
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_NAME", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		return client->GetName().c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_GUID", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		return client->GetGuid().c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_PLAYER_IDENTIFIERS", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		return client->GetIdentifiers().size();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_IDENTIFIER", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		int idx = context.GetArgument<int>(1);

		if (idx < 0 || idx >= client->GetIdentifiers().size())
		{
			return (const char*)nullptr;
		}

		return client->GetIdentifiers()[idx].c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_ENDPOINT", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		static thread_local std::string str;
		str = client->GetTcpEndPoint();

		return str.c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_PING", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		fx::NetPeerStackBuffer stackBuffer;
		gscomms_get_peer(client->GetPeer(), stackBuffer);
		auto peer = stackBuffer.GetBase();

		if (!peer)
		{
			return -1;
		}

		return int(peer->GetPing());
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_LAST_MSG", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		return (msec() - client->GetLastSeen()).count();
	}, 0x7fffffff));

	fx::ScriptEngine::RegisterNativeHandler("DROP_PLAYER", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
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

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_ACE_ALLOWED", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		const char* object = context.CheckArgument<const char*>(1);

		se::ScopedPrincipalReset reset;
		auto principalScope = client->EnterPrincipalScope();

		return seCheckPrivilege(object);
	}));

	static thread_local std::vector<fx::ClientWeakPtr> clients;

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

		registry->ForAllClients([&](const fx::ClientSharedPtr& client)
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

		auto lc = clients[i].lock();
		if (!lc)
		{
			context.SetResult(nullptr);
			return;
		}

		static thread_local std::string clientId;
		clientId = fmt::sprintf("%d", lc->GetNetId());

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

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_PED", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client) -> uint32_t
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

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WANTED_LEVEL", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();
		
		try
		{
			auto entity = gameState->GetEntity(std::any_cast<uint32_t>(client->GetData("playerEntity")));

			auto node = entity->syncTree->GetPlayerWantedAndLOS();
			
			return node ? node->wantedLevel : 0;
		}
		catch (std::bad_any_cast&)
		{
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_EVADING_WANTED_LEVEL", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		try
		{
			auto entity = gameState->GetEntity(std::any_cast<uint32_t>(client->GetData("playerEntity")));

			auto node = entity->syncTree->GetPlayerWantedAndLOS();

			return node ? node->isEvading : 0;
		}
		catch (std::bad_any_cast&)
		{
			return 0;
		}
	}));
	
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_TIME_IN_PURSUIT", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		try
		{
			auto entity = gameState->GetEntity(std::any_cast<uint32_t>(client->GetData("playerEntity")));

			auto node = entity->syncTree->GetPlayerWantedAndLOS();
			bool prevPursuitArg = context.GetArgument<bool>(1);

			if (node)
				return prevPursuitArg ? node->timeInPrevPursuit : node->timeInPursuit;
			else
				return -1;
		}
		catch (std::bad_any_cast&) {
			return -1;
		}
	}));
}
