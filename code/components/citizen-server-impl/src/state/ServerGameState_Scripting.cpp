#include <StdInc.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <state/ServerGameState.h>

#include <ResourceManager.h>
#include <ScriptEngine.h>

#include <ScriptSerialization.h>

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

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_FROM_NETWORK_ID", [](fx::ScriptContext& context)
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
			context.SetResult(0);
			return;
		}

		auto entity = gameState->GetEntity(0, id);

		if (!entity)
		{
			context.SetResult(0);
			return;
		}

		context.SetResult(gameState->MakeScriptHandle(entity));
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_OWNER", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		int retval = -1;
		auto entry = entity->client.lock();

		if (entry)
		{
			retval = entry->GetNetId();
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

		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			resultVec.x = 0.0f;
			resultVec.y = 0.0f;
			resultVec.z = entity->GetData("currentHeading", 0.0f) * 180.0 / pi;
		}
		else
		{
			resultVec.x = entity->GetData("rotX", 0.0f) * 180.0 / pi;
			resultVec.y = entity->GetData("rotY", 0.0f) * 180.0 / pi;
			resultVec.z = entity->GetData("rotZ", 0.0f) * 180.0 / pi;
		}

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_HEADING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		float heading;

		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			heading = entity->GetData("currentHeading", 0.0f) * 180.0 / pi;
		}
		else
		{
			heading = entity->GetData("rotZ", 0.0f) * 180.0 / pi;
		}

		return (heading < 0) ? 360.0f + heading : heading;
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

	fx::ScriptEngine::RegisterNativeHandler("SET_SYNC_ENTITY_LOCKDOWN_MODE", [](fx::ScriptContext& context)
	{
		std::string_view sv = context.CheckArgument<const char*>(0);

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		if (sv == "strict")
		{
			gameState->SetEntityLockdownMode(fx::EntityLockdownMode::Strict);
		}
		else if (sv == "relaxed")
		{
			gameState->SetEntityLockdownMode(fx::EntityLockdownMode::Relaxed);
		}
		else if (sv == "inactive")
		{
			gameState->SetEntityLockdownMode(fx::EntityLockdownMode::Inactive);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_NETWORK_ID_FROM_ENTITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->handle & 0xFFFF;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_HASH_KEY", [](fx::ScriptContext& context)
	{
		context.SetResult(HashString(context.GetArgument<const char*>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_RADIO_STATION_INDEX", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("radioStation", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LIGHTS_STATE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			*context.GetArgument<int*>(1) = entity->GetData("lightsOn", 0);
			*context.GetArgument<int*>(2) = entity->GetData("highbeamsOn", 0);
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_ENGINE_RUNNING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("isEngineOn", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ENGINE_STARTING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("isEngineStarting", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDBRAKE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("handbrake", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HEADLIGHTS_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		int headlightsColour = 0;

		int unk8 = entity->GetData("unk8", 0);
		int defaultHeadlights = entity->GetData("defaultHeadlights", 0);
		
		if (defaultHeadlights)
		{
			headlightsColour = entity->GetData("headlightsColour", 0);
		}

		return headlightsColour;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_SIREN_ON", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("sirenOn", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOOR_STATUS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		int doorStatus = 0;
		int doorsOpen = entity->GetData("doorsOpen", 0);

		if (context.GetArgumentCount() > 1 && doorsOpen)
		{
			doorStatus = entity->GetData("doorPosition" + context.GetArgument<int>(1), 0);
		}

		return doorStatus;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOOR_LOCK_STATUS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("lockStatus", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOORS_LOCKED_FOR_PLAYER", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		int lockedForPlayer = 0;
		int hasLock = entity->GetData("hasLock", 0);

		if (hasLock)
		{
			int lockedPlayers = entity->GetData("lockedPlayers", 0);

			if (lockedPlayers == -1) // ALL
			{
				lockedForPlayer = true;
			}
		}

		return lockedForPlayer;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ENGINE_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("engineHealth", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_PETROL_TANK_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("petrolTankHealth", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_TYRE_BURST", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		bool tyreBurst = false;
		bool wheelsFine = entity->GetData("tyresFine", true);

		if (!wheelsFine && context.GetArgumentCount() > 1)
		{
			int tyreID = context.GetArgument<int>(1);
			bool completely = context.GetArgument<bool>(2);

			int tyreStatus = entity->GetData("tyreStatus" + tyreID, 0);

			if (completely && tyreStatus == 2 || !completely && tyreStatus == 1)
			{
				tyreBurst = true;
			}
		}

		return tyreBurst;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_BODY_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("bodyHealth", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_MAX_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("maxHealth", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_ARMOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("armour", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_CAUSE_OF_DEATH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("causeOfDeath", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DESIRED_HEADING", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			float heading = entity->GetData("desiredHeading", 0.0f) * 180.0 / pi;
			return (heading < 0) ? 360.0f + heading : heading;
		}

		return 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_MAX_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Player:
			return entity->GetData("maxHealth", 0);
		case fx::sync::NetObjEntityType::Ped:
			return entity->GetData("maxHealth", 0);
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Player:
			return entity->GetData("health", 0);
		case fx::sync::NetObjEntityType::Ped:
			return entity->GetData("health", 0);
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_COLOURS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			*context.GetArgument<int*>(1) = entity->GetData("primaryColour", 0);
			*context.GetArgument<int*>(2) = entity->GetData("secondaryColour", 0);
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_EXTRA_COLOURS", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			*context.GetArgument<int*>(1) = entity->GetData("pearlColour", 0);
			*context.GetArgument<int*>(2) = entity->GetData("wheelColour", 0);
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_INTERIOR_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			*context.GetArgument<int*>(1) = entity->GetData("interiorColour", 0);
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			*context.GetArgument<int*>(1) = entity->GetData("dashboardColour", 0);
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CUSTOM_PRIMARY_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 3)
		{
			bool isPrimaryColour = entity->GetData("isPrimaryColour", 0);

			*context.GetArgument<int*>(1) = isPrimaryColour ? entity->GetData("primaryRedColour", 0) : 0;
			*context.GetArgument<int*>(2) = isPrimaryColour ? entity->GetData("primaryGreenColour", 0) : 0;
			*context.GetArgument<int*>(3) = isPrimaryColour ? entity->GetData("primaryBlueColour", 0) : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CUSTOM_SECONDARY_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 3)
		{
			bool isSecondaryColour = entity->GetData("isSecondaryColour", 0);

			*context.GetArgument<int*>(1) = isSecondaryColour ? entity->GetData("secondaryRedColour", 0) : 0;
			*context.GetArgument<int*>(2) = isSecondaryColour ? entity->GetData("secondaryGreenColour", 0) : 0;
			*context.GetArgument<int*>(3) = isSecondaryColour ? entity->GetData("secondaryBlueColour", 0) : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("isPrimaryColour", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("isSecondaryColour", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LIVERY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("liveryIndex", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ROOF_LIVERY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("roofLiveryIndex", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DIRT_LEVEL", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("dirtLevel", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_TYPE", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("wheelType", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WINDOW_TINT", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		int windowTintIndex = entity->GetData("windowTintIndex", 0);
		return windowTintIndex == 255 ? -1 : windowTintIndex;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TYRE_SMOKE_COLOR", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			*context.GetArgument<int*>(1) = entity->GetData("tyreSmokeRedColour", 0);
			*context.GetArgument<int*>(2) = entity->GetData("tyreSmokeGreenColour", 0);
			*context.GetArgument<int*>(3) = entity->GetData("tyreSmokeBlueColour", 0);
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_PLATE_TEXT", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		static std::string numberPlateText;
		numberPlateText.clear();

		for (int i = 0; i < 8; i++)
		{
			char letter = entity->GetData(fmt::sprintf("plate%d", i), 32);
			numberPlateText.push_back(letter);
		}

		return numberPlateText.c_str();
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("numberPlateTextIndex", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("HAS_VEHICLE_BEEN_OWNED_BY_PLAYER", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("hasBeenOwnedByPlayer", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("HAS_ENTITY_BEEN_MARKED_AS_NO_LONGER_NEEDED", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return entity->GetData("noLongerNeeded", 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_VEHICLES", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		std::vector<int> entityList;
		std::shared_lock<std::shared_mutex> lock(gameState->m_entityListMutex);

		for (auto& entity : gameState->m_entityList)
		{
			if (entity && (entity->type == fx::sync::NetObjEntityType::Automobile ||
				entity->type == fx::sync::NetObjEntityType::Bike ||
				entity->type == fx::sync::NetObjEntityType::Boat ||
				entity->type == fx::sync::NetObjEntityType::Heli ||
				entity->type == fx::sync::NetObjEntityType::Plane ||
				entity->type == fx::sync::NetObjEntityType::Submarine ||
				entity->type == fx::sync::NetObjEntityType::Trailer ||
				entity->type == fx::sync::NetObjEntityType::Train))
			{
				entityList.push_back(gameState->MakeScriptHandle(entity));
			}
		}

		context.SetResult(fx::SerializeObject(entityList));
	});

	fx::ScriptEngine::RegisterNativeHandler("DELETE_ENTITY", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		gameState->DeleteEntity(entity);

		return 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_SELECTED_PED_WEAPON", makeEntityFunction([](fx::ScriptContext& context, const std::shared_ptr<fx::sync::SyncEntityState>& entity)
	{
		return uint32_t(entity->GetData("curWeapon", 0));
	}));
});
