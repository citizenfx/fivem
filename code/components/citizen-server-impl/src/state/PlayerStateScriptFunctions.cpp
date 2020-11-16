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
		if (!IsStateGame())
		{
			return;
		}

		CreatePlayerCommands();
	});
});

static void CreatePlayerCommands()
{
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
