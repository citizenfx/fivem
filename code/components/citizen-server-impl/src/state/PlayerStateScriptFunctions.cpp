#include "StdInc.h"

#include <ResourceManager.h>

#include <GameServer.h>

#include <ClientRegistry.h>
#include <ServerInstanceBaseRef.h>

#include <ScriptEngine.h>

#include <se/Security.h>

#include <state/ServerGameState.h>

#include <MakePlayerEntityFunction.h>

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
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WANTED_LEVEL", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto node = entity->syncTree->GetPlayerWantedAndLOS();
			
		return node ? node->wantedLevel : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_EVADING_WANTED_LEVEL", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto node = entity->syncTree->GetPlayerWantedAndLOS();

		return node ? node->isEvading : 0;
	}));
	
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_TIME_IN_PURSUIT", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		bool prevPursuitArg = context.GetArgument<bool>(1);

		if (auto node = entity->syncTree->GetPlayerWantedAndLOS())
		{
			return prevPursuitArg ? node->timeInPrevPursuit : node->timeInPursuit;
		}

		return -1;
	}, -1));
}
