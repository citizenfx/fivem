#pragma once
#pragma once

#include <ScriptEngine.h>

#include <state/ServerGameState.h>

#include <ClientRegistry.h>
#include <ResourceManager.h>
#include <ServerInstanceBaseRef.h>

template<typename TFn>
inline auto MakePlayerEntityFunction(TFn fn, uintptr_t defaultValue = 0)
{
	return [=](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		// get the server's client registry
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		// parse the client ID
		const char* id = context.CheckArgument<const char*>(0);

		if (!id)
		{
			context.SetResult(defaultValue);
			return;
		}

		uint32_t netId = atoi(id);

		auto client = clientRegistry->GetClientByNetID(netId);

		if (!client)
		{
			context.SetResult(defaultValue);
			return;
		}

		uint32_t entityHandle;

		try
		{
			entityHandle = std::any_cast<uint32_t>(client->GetData("playerEntity"));
		}
		catch (std::bad_any_cast&)
		{
			context.SetResult(defaultValue);
			return;
		}

		auto entity = gameState->GetEntity(entityHandle);

		if (!entity)
		{
			throw std::runtime_error(va("Tried to access invalid entity: %d", id));

			context.SetResult(defaultValue);
			return;
		}

		context.SetResult(fn(context, entity));
	};
};
