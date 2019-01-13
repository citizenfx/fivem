#include <StdInc.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <state/ServerGameState.h>

#include <ResourceManager.h>
#include <ScriptEngine.h>

static InitFunction initFunction([]()
{
	auto makeEntityFunction = [](auto fn, uintptr_t defaultValue = 0)
	{
		return [=](fx::ScriptContext& context)
		{
			// get the current resource manager
			auto resourceManager = fx::ResourceManager::GetCurrent();

			// get the owning server instance
			auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

			// get the server's game state
			auto gameState = instance->GetComponent<fx::ServerGameState>();

			// parse the client ID
			auto id = context.GetArgument<uint32_t>(0);

			if (!id)
			{
				context.SetResult(defaultValue);
				return;
			}

			auto entity = gameState->GetEntity(id);

			if (!entity)
			{
				throw std::runtime_error(va("Tried to access invalid entity: %d", id));

				context.SetResult(defaultValue);
				return;
			}

			context.SetResult(fn(context, entity));
		};
	};

	struct scrVector
	{
		float x;
		int pad;
		float y;
		int pad2;
		float z;
		int pad3;
	};

	fx::ScriptEngine::RegisterNativeHandler("DOES_ENTITY_EXIST", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		// parse the client ID
		auto id = context.GetArgument<uint32_t>(0);

		if (!id)
		{
			context.SetResult(false);
			return;
		}

		auto entity = gameState->GetEntity(id);

		if (!entity)
		{
			context.SetResult(false);
			return;
		}

		context.SetResult(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_OWNER", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		const char* retval = nullptr;
		static std::string retPtr;

		auto entry = entity->client.lock();

		if (entry)
		{
			retPtr = fmt::sprintf("%d", entry->GetNetId());
			retval = retPtr.c_str();
		}

		return retval;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_COORDS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		scrVector resultVec = { 0 };
		resultVec.x = entity->GetData("posX", 0.0f);
		resultVec.y = entity->GetData("posY", 0.0f);
		resultVec.z = entity->GetData("posZ", 0.0f);

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_VELOCITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		scrVector resultVec = { 0 };
		resultVec.x = entity->GetData("velX", 0.0f);
		resultVec.y = entity->GetData("velY", 0.0f);
		resultVec.z = entity->GetData("velZ", 0.0f);

		return resultVec;
	}));

	static const float pi = 3.14159265358979323846f;

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ROTATION_VELOCITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		scrVector resultVec = { 0 };
		resultVec.x = entity->GetData("angVelX", 0.0f);
		resultVec.y = entity->GetData("angVelY", 0.0f);
		resultVec.z = entity->GetData("angVelZ", 0.0f);

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ROTATION", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		scrVector resultVec = { 0 };
		resultVec.x = entity->GetData("rotX", 0.0f) * 180.0 / pi;
		resultVec.y = entity->GetData("rotY", 0.0f) * 180.0 / pi;
		resultVec.z = entity->GetData("rotZ", 0.0f) * 180.0 / pi;

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_HEADING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("angVelZ", 0.0f) * 180.0 / pi;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_POPULATION_TYPE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		fx::sync::ePopType popType = fx::sync::POPTYPE_UNKNOWN;
		entity->syncTree->GetPopulationType(&popType);

		return popType;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_MODEL", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		uint32_t model = 0;
		entity->syncTree->GetModelHash(&model);

		return model;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_SCRIPT", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		uint32_t script = 0;
		if (entity->syncTree->GetScriptHash(&script))
		{
			static std::string scriptName;
			scriptName.clear();

			auto resourceManager = fx::ResourceManager::GetCurrent();
			resourceManager->ForAllResources([script](const fwRefContainer<fx::Resource>& resource)
			{
				if (scriptName.empty())
				{
					std::string subName = resource->GetName();

					if (subName.length() > 63)
					{
						subName = subName.substr(0, 63);
					}

					if (HashString(subName.c_str()) == script)
					{
						scriptName = resource->GetName();
					}
				}
			});

			if (!scriptName.empty())
			{
				return scriptName.c_str();
			}
		}

		return (const char*)nullptr;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_TYPE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Automobile:
		case fx::sync::NetObjEntityType::Bike:
		case fx::sync::NetObjEntityType::Boat:
		case fx::sync::NetObjEntityType::Heli:
		case fx::sync::NetObjEntityType::Plane:
		case fx::sync::NetObjEntityType::Submarine:
		case fx::sync::NetObjEntityType::Trailer:
		case fx::sync::NetObjEntityType::Train:
			return 2;
		case fx::sync::NetObjEntityType::Ped:
		case fx::sync::NetObjEntityType::Player:
			return 1;
		case fx::sync::NetObjEntityType::Object:
		case fx::sync::NetObjEntityType::Door:
		case fx::sync::NetObjEntityType::Pickup:
			return 3;
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_NETWORK_ID_FROM_ENTITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->handle & 0xFFFF;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_HASH_KEY", [](fx::ScriptContext& context)
	{
		context.SetResult(HashString(context.GetArgument<const char*>(0)));
	});
});
