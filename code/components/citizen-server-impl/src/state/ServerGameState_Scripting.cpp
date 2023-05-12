#include <StdInc.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <state/ServerGameState.h>

#include <ResourceManager.h>
#include <ScriptEngine.h>

#include <ScriptSerialization.h>
#include <MakeClientFunction.h>
#include <MakePlayerEntityFunction.h>

namespace fx
{
void DisownEntityScript(const fx::sync::SyncEntityPtr& entity);
}

static void Init()
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

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_ENTITY_OWNER", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		int retval = -1;
		auto entry = entity->GetClient();

		if (entry)
		{
			retval = entry->GetNetId();
		}

		return retval;
	}));

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_FIRST_ENTITY_OWNER", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
    {
        int retval = -1;
        auto firstOwner = entity->GetFirstOwner();

        if (firstOwner && !entity->firstOwnerDropped)
        {
            retval = firstOwner->GetNetId();
        }

        return retval;
    }));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_COORDS", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		float position[3];
		entity->syncTree->GetPosition(position);

		scrVector resultVec = { 0 };
		resultVec.x = position[0];
		resultVec.y = position[1];
		resultVec.z = position[2];

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_VELOCITY", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		scrVector resultVec = { 0 };

		auto v = entity->syncTree->GetVelocity();

		if (v)
		{
			resultVec.x = v->velX;
			resultVec.y = v->velY;
			resultVec.z = v->velZ;
		}

		return resultVec;
	}));

	static const float pi = 3.14159265358979323846f;

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ROTATION_VELOCITY", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto av = entity->syncTree->GetAngVelocity();

		scrVector resultVec = { 0 };

		if (av)
		{
			resultVec.x = av->angVelX;
			resultVec.y = av->angVelY;
			resultVec.z = av->angVelZ;
		}

		return resultVec;
	}));

	static auto GetEntityRotation = [](const fx::sync::SyncEntityPtr& entity, scrVector& resultVec)
	{
		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			resultVec.x = 0.0f;
			resultVec.y = 0.0f;

			auto pn = entity->syncTree->GetPedOrientation();

			if (pn)
			{
				resultVec.z = pn->currentHeading * 180.0 / pi;
			}
		}
		else
		{
			auto en = entity->syncTree->GetEntityOrientation();
			auto on = entity->syncTree->GetObjectOrientation();

			if (en || on)
			{
				bool highRes = false;
				fx::sync::compressed_quaternion<11> quat;
				float rotX, rotY, rotZ;

				if (en)
				{
					quat = en->quat;
				}
				else if (on)
				{
					highRes = on->highRes;
					quat = on->quat;
					rotX = on->rotX;
					rotY = on->rotY;
					rotZ = on->rotZ;
				}

				if (highRes)
				{
					resultVec.x = rotX * 180.0 / pi;
					resultVec.y = rotY * 180.0 / pi;
					resultVec.z = rotZ * 180.0 / pi;
				}
				else
				{
					float qx, qy, qz, qw;
					quat.Save(qx, qy, qz, qw);

					auto m4 = glm::toMat4(glm::quat{ qw, qx, qy, qz });

					// common GTA rotation (2) is ZXY
					glm::extractEulerAngleZXY(m4, resultVec.z, resultVec.x, resultVec.y);

					resultVec.x = glm::degrees(resultVec.x);
					resultVec.y = glm::degrees(resultVec.y);
					resultVec.z = glm::degrees(resultVec.z);
				}
			}
		}
	};

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ROTATION", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		scrVector resultVec = { 0 };
		GetEntityRotation(entity, resultVec);
		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_HEADING", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		scrVector resultVec = { 0 };
		GetEntityRotation(entity, resultVec);
		return (resultVec.z < 0) ? 360.0f + resultVec.z : resultVec.z;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_POPULATION_TYPE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		fx::sync::ePopType popType = fx::sync::POPTYPE_UNKNOWN;
		entity->syncTree->GetPopulationType(&popType);

		return popType;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_MODEL", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		uint32_t model = 0;
		entity->syncTree->GetModelHash(&model);

		return model;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_SCRIPT", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
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

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TYPE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity) -> const char*
	{
		switch (entity->type)
		{
			case fx::sync::NetObjEntityType::Automobile:
				return "automobile";
			case fx::sync::NetObjEntityType::Bike:
				return "bike";
			case fx::sync::NetObjEntityType::Boat:
				return "boat";
			case fx::sync::NetObjEntityType::Heli:
				return "heli";
			case fx::sync::NetObjEntityType::Plane:
				return "plane";
			case fx::sync::NetObjEntityType::Submarine:
				return "submarine";
			case fx::sync::NetObjEntityType::Trailer:
				return "trailer";
			case fx::sync::NetObjEntityType::Train:
				return "train";
		}

		return nullptr;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_TYPE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
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
#ifdef STATE_RDR3
		case fx::sync::NetObjEntityType::DraftVeh:
#endif
			return 2;
		case fx::sync::NetObjEntityType::Ped:
		case fx::sync::NetObjEntityType::Player:
#ifdef STATE_RDR3
		case fx::sync::NetObjEntityType::Animal:
		case fx::sync::NetObjEntityType::Horse:
#endif
			return 1;
		case fx::sync::NetObjEntityType::Object:
		case fx::sync::NetObjEntityType::Door:
		case fx::sync::NetObjEntityType::Pickup:
#ifdef STATE_RDR3
		case fx::sync::NetObjEntityType::WorldProjectile:
#endif
			return 3;
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_ROUTING_BUCKET_POPULATION_ENABLED", [](fx::ScriptContext& context)
	{
		int bucket = context.GetArgument<int>(0);
		bool enabled = context.GetArgument<bool>(1);

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();
		gameState->SetPopulationDisabled(bucket, !enabled);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_ROUTING_BUCKET_ENTITY_LOCKDOWN_MODE", [](fx::ScriptContext& context)
	{
		int bucket = context.GetArgument<int>(0);
		std::string_view sv = context.CheckArgument<const char*>(1);

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		if (sv == "strict")
		{
			gameState->SetEntityLockdownMode(bucket, fx::EntityLockdownMode::Strict);
		}
		else if (sv == "no_dummy")
		{
			gameState->SetEntityLockdownMode(bucket, fx::EntityLockdownMode::Dummy);
		}
		else if (sv == "relaxed")
		{
			gameState->SetEntityLockdownMode(bucket, fx::EntityLockdownMode::Relaxed);
		}
		else if (sv == "inactive")
		{
			gameState->SetEntityLockdownMode(bucket, fx::EntityLockdownMode::Inactive);
		}
	});

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
		else if (sv == "no_dummy")
		{
			gameState->SetEntityLockdownMode(fx::EntityLockdownMode::Dummy);
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

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_NETWORK_ID_FROM_ENTITY", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		return entity->handle & 0xFFFF;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_HASH_KEY", [](fx::ScriptContext& context)
	{
		context.SetResult(HashString(context.CheckArgument<const char*>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_RADIO_STATION_INDEX", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->radioStation : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LIGHTS_STATE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleGameState();

			*context.GetArgument<int*>(1) = vn ? vn->lightsOn : false;
			*context.GetArgument<int*>(2) = vn ? vn->highbeamsOn : false;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_ENGINE_RUNNING", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->isEngineOn : false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_ENGINE_STARTING", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->isEngineStarting : false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HANDBRAKE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->handbrake : false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HEADLIGHTS_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		int headlightsColour = 0;

		auto vn = entity->syncTree->GetVehicleGameState();

		if (vn)
		{
			if (vn->defaultHeadlights)
			{
				headlightsColour = vn->headlightsColour;
			}
		}

		return headlightsColour;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_SIREN_ON", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->sirenOn : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOOR_STATUS", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (!vn)
		{
			return 0;
		}

		int doorStatus = 0;
		int doorsOpen = vn->doorsOpen;

		if (context.GetArgumentCount() > 1 && doorsOpen)
		{
			doorStatus = vn->doorPositions[context.GetArgument<int>(1)];
		}

		return doorStatus;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOOR_LOCK_STATUS", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		return vn ? vn->lockStatus : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DOORS_LOCKED_FOR_PLAYER", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (!vn)
		{
			return false;
		}

		bool lockedForPlayer = false;
		int hasLock = vn->hasLock;

		if (hasLock)
		{
			int lockedPlayers = vn->lockedPlayers;

			if (lockedPlayers == -1) // ALL
			{
				lockedForPlayer = true;
			}
		}

		return lockedForPlayer;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ENGINE_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		return vn ? float(vn->engineHealth) : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_PETROL_TANK_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		return vn ? float(vn->petrolTankHealth) : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_TYRE_BURST", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		if (!vn)
		{
			return false;
		}

		bool tyreBurst = false;
		bool wheelsFine = vn->tyresFine;

		if (!wheelsFine && context.GetArgumentCount() > 1)
		{
			int tyreID = context.GetArgument<int>(1);
			bool completely = context.GetArgument<bool>(2);

			int tyreStatus = vn->tyreStatus[tyreID];

			if (completely && tyreStatus == 2 || !completely && tyreStatus == 1)
			{
				tyreBurst = true;
			}
		}

		return tyreBurst;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_BODY_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleHealth();

		return vn ? float(vn->bodyHealth) : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_MAX_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPedHealth();

		return pn ? pn->maxHealth : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_ARMOUR", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPedHealth();

		return pn ? pn->armour : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_CAUSE_OF_DEATH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPedHealth();

		return pn ? pn->causeOfDeath : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DESIRED_HEADING", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (entity->type == fx::sync::NetObjEntityType::Player || entity->type == fx::sync::NetObjEntityType::Ped)
		{
			auto pn = entity->syncTree->GetPedOrientation();

			if (pn)
			{
				float heading = pn->desiredHeading * 180.0 / pi;
				return (heading < 0) ? 360.0f + heading : heading;
			}
		}

		return 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_PEDS", [](fx::ScriptContext& context)
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
			if (entity && (entity->type == fx::sync::NetObjEntityType::Ped ||
#ifdef STATE_RDR3
				entity->type == fx::sync::NetObjEntityType::Animal ||
				entity->type == fx::sync::NetObjEntityType::Horse ||
#endif
				entity->type == fx::sync::NetObjEntityType::Player))
			{
				entityList.push_back(gameState->MakeScriptHandle(entity));
			}
		}

		context.SetResult(fx::SerializeObject(entityList));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_MAX_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Player:
		case fx::sync::NetObjEntityType::Ped:
		{
			auto pn = entity->syncTree->GetPedHealth();
			return pn ? pn->maxHealth : 0;
		}
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		switch (entity->type)
		{
		case fx::sync::NetObjEntityType::Player:
		case fx::sync::NetObjEntityType::Ped:
		{
			auto pn = entity->syncTree->GetPedHealth();
			return pn ? pn->health : 0;
		}
		default:
			return 0;
		}
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_COLOURS", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->primaryColour : 0;
			*context.GetArgument<int*>(2) = vn ? vn->secondaryColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_EXTRA_COLOURS", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->pearlColour : 0;
			*context.GetArgument<int*>(2) = vn ? vn->wheelColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_INTERIOR_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->interiorColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DASHBOARD_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->dashboardColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CUSTOM_PRIMARY_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 3)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = (vn && vn->isPrimaryColourRGB) ? vn->primaryRedColour : 0;
			*context.GetArgument<int*>(2) = (vn && vn->isPrimaryColourRGB) ? vn->primaryGreenColour : 0;
			*context.GetArgument<int*>(3) = (vn && vn->isPrimaryColourRGB) ? vn->primaryBlueColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_CUSTOM_SECONDARY_COLOUR", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 3)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = (vn && vn->isSecondaryColourRGB) ? vn->secondaryRedColour : 0;
			*context.GetArgument<int*>(2) = (vn && vn->isSecondaryColourRGB) ? vn->secondaryGreenColour : 0;
			*context.GetArgument<int*>(3) = (vn && vn->isSecondaryColourRGB) ? vn->secondaryBlueColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->isPrimaryColourRGB : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->isSecondaryColourRGB : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LIVERY", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->liveryIndex : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_ROOF_LIVERY", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->roofLiveryIndex : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DIRT_LEVEL", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? float(vn->dirtLevel) : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WHEEL_TYPE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return vn ? vn->wheelType : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_WINDOW_TINT", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		return (vn)
			? ((vn->windowTintIndex == 255) ? -1 : vn->windowTintIndex)
			: 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_TYRE_SMOKE_COLOR", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 2)
		{
			auto vn = entity->syncTree->GetVehicleAppearance();

			*context.GetArgument<int*>(1) = vn ? vn->tyreSmokeRedColour : 0;
			*context.GetArgument<int*>(2) = vn ? vn->tyreSmokeGreenColour : 0;
			*context.GetArgument<int*>(3) = vn ? vn->tyreSmokeBlueColour : 0;
		}

		return 1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_PLATE_TEXT", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		if (vn)
		{
			return (const char*)vn->plate;
		}
		
		return (const char*)"";
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();

		if (vn)
		{
			return vn->numberPlateTextIndex;
		}

		return 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("HAS_VEHICLE_BEEN_OWNED_BY_PLAYER", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (vn)
		{
			return vn->hasBeenOwnedByPlayer;
		}

		return false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("HAS_ENTITY_BEEN_MARKED_AS_NO_LONGER_NEEDED", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleGameState();

		if (vn)
		{
			return vn->noLongerNeeded;
		}

		return false;
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
#ifdef STATE_RDR3
				entity->type == fx::sync::NetObjEntityType::DraftVeh ||
#endif
				entity->type == fx::sync::NetObjEntityType::Train))
			{
				entityList.push_back(gameState->MakeScriptHandle(entity));
			}
		}

		context.SetResult(fx::SerializeObject(entityList));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_OBJECTS", [](fx::ScriptContext& context)
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
			if (entity && (entity->type == fx::sync::NetObjEntityType::Object ||
				entity->type == fx::sync::NetObjEntityType::Door))
			{
				entityList.push_back(gameState->MakeScriptHandle(entity));
			}
		}

		context.SetResult(fx::SerializeObject(entityList));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_PED_IS_IN", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto node = entity->syncTree->GetPedGameState();
		bool lastVehicleArg = context.GetArgument<bool>(1);


		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		if (!node)
			return (uint32_t)0;

		// If ped is not in a vehicle, or was not in a previous vehicle (depending on the lastVehicleArg) return 0
		if ((lastVehicleArg == true && node->lastVehiclePedWasIn == -1) || (lastVehicleArg == false && node->curVehicle == -1))
			return (uint32_t)0;

		auto returnEntity = lastVehicleArg == true ? gameState->GetEntity(0, node->lastVehiclePedWasIn) : gameState->GetEntity(0, node->curVehicle);

		if (!returnEntity)
			return (uint32_t)0;

		// Return the entity
		return gameState->MakeScriptHandle(returnEntity);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_IN_VEHICLE_SEAT", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
    {
        auto vn = entity->syncTree->GetVehicleGameState();

        int seatArg = context.GetArgument<int>(1) + 2;

        // get the current resource manager
        auto resourceManager = fx::ResourceManager::GetCurrent();

        // get the owning server instance
        auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

        // get the server's game state
        auto gameState = instance->GetComponent<fx::ServerGameState>();

        int retval = 0;

        if (vn && vn->occupants[seatArg]) 
        {
            auto pedEntity = gameState->GetEntity(0, vn->occupants[seatArg]);
            if (pedEntity) 
            {
                retval = gameState->MakeScriptHandle(pedEntity);
            }
        }

        return retval;
    }));

	fx::ScriptEngine::RegisterNativeHandler("GET_LAST_PED_IN_VEHICLE_SEAT", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
    {
        auto vn = entity->syncTree->GetVehicleGameState();

        int seatArg = context.GetArgument<int>(1) + 2;

        // get the current resource manager
        auto resourceManager = fx::ResourceManager::GetCurrent();

        // get the owning server instance
        auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

        // get the server's game state
        auto gameState = instance->GetComponent<fx::ServerGameState>();

        int retval = 0;

        if (vn && vn->lastOccupant[seatArg]) 
        {
            auto pedEntity = gameState->GetEntity(0, vn->lastOccupant[seatArg]);
            if (pedEntity) 
            {
                retval = gameState->MakeScriptHandle(pedEntity);
            }
        }

        return retval;
    }));

	fx::ScriptEngine::RegisterNativeHandler("DELETE_ENTITY", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		gameState->DeleteEntity(entity);

		return 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_AS_NO_LONGER_NEEDED", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		// parse the client ID
		auto id = context.CheckArgument<uint32_t*>(0);

		if (!*id)
		{
			return;
		}

		auto entity = gameState->GetEntity(*id);

		if (!entity)
		{
			throw std::runtime_error(va("Tried to access invalid entity: %d", *id));
			return;
		}

		if (entity->GetClient())
		{
			// TODO: client-side set-as-no-longer-needed indicator
		}
		else
		{
			fx::DisownEntityScript(entity);
		}

		*id = 0;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_SELECTED_PED_WEAPON", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto node = entity->syncTree->GetPedGameState();

		return uint32_t(node ? node->curWeapon : 0);
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_PED_A_PLAYER", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		return entity->type == fx::sync::NetObjEntityType::Player;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_TEAM", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->playerTeam : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_AIR_DRAG_MULTIPLIER_FOR_PLAYERS_VEHICLE", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->airDragMultiplier : 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_MAX_HEALTH", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->maxHealth : 100;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_MAX_ARMOUR", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->maxArmour : 100;
	}));

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_GET_VOICE_PROXIMITY_OVERRIDE_FOR_PLAYER", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		float position[3];
		auto pn = entity->syncTree->GetPlayerGameState();

		scrVector resultVec = { 0 };

		if (pn)
		{
			resultVec.x = pn->voiceProximityOverrideX;
			resultVec.y = pn->voiceProximityOverrideY;
			resultVec.z = pn->voiceProximityOverrideZ;
		}
		else
		{
			resultVec.x = 0.0f;
			resultVec.y = 0.0f;
			resultVec.z = 0.0f;
		}

		return resultVec;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WEAPON_DEFENSE_MODIFIER", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->weaponDefenseModifier : 1.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WEAPON_DEFENSE_MODIFIER_2", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->weaponDefenseModifier2 : 1.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WEAPON_DAMAGE_MODIFIER", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->weaponDamageModifier : 1.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->meleeWeaponDamageModifier : 1.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_PLAYER_USING_SUPER_JUMP", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->isSuperJumpEnabled : false;
	}));
	
	fx::ScriptEngine::RegisterNativeHandler("IS_VEHICLE_EXTRA_TURNED_ON", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto vn = entity->syncTree->GetVehicleAppearance();
		bool isExtraTurnedOn = false;

		if (context.GetArgumentCount() > 1 && vn)
		{
			// TODO: return false if the extra does not exist;

			int extraId = context.GetArgument<int>(1);
			isExtraTurnedOn = ((1 << (extraId + 1)) & vn->extras) == 0;
		}

		return isExtraTurnedOn;
	}));

	fx::ScriptEngine::RegisterNativeHandler("ENSURE_ENTITY_STATE_BAG", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (!entity->HasStateBag())
		{
			// get the current resource manager
			auto resourceManager = fx::ResourceManager::GetCurrent();

			// get the owning server instance
			auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

			// get the server's game state
			auto gameState = instance->GetComponent<fx::ServerGameState>();
			auto stateBag = gameState->GetStateBags()->RegisterStateBag(fmt::sprintf("entity:%d", entity->handle & 0xFFFF));

			std::set<int> rts{ -1 };

			for (auto i = entity->relevantTo.find_first(); i != decltype(entity->relevantTo)::kSize; i = entity->relevantTo.find_next(i))
			{
				rts.insert(i);
			}

			stateBag->SetRoutingTargets(rts);

			auto client = entity->GetClient();

			if (client)
			{
				stateBag->SetOwningPeer(client->GetSlotId());
			}
			else
			{
				stateBag->SetOwningPeer(-1);
			}

			entity->SetStateBag(std::move(stateBag));
		}

		return 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_DISTANCE_CULLING_RADIUS", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			float radius = context.GetArgument<float>(1);
			entity->overrideCullingRadius = radius * radius;
		}

		return true;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_ROUTING_BUCKET", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		auto [lock, clientData] = gameState->ExternalGetClientData(client);
		return int(clientData->routingBucket);
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_ROUTING_BUCKET", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		if (context.GetArgumentCount() > 1)
		{
			auto bucket = context.GetArgument<int>(1);

			if (bucket >= 0)
			{
				// get the current resource manager
				auto resourceManager = fx::ResourceManager::GetCurrent();

				// get the owning server instance
				auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

				// get the server's game state
				auto gameState = instance->GetComponent<fx::ServerGameState>();

				auto [lock, clientData] = gameState->ExternalGetClientData(client);
				gameState->ClearClientFromWorldGrid(client);
				clientData->routingBucket = bucket;
				
				fx::sync::SyncEntityPtr playerEntity;

				{
					std::shared_lock _lock(clientData->playerEntityMutex);
					playerEntity = clientData->playerEntity.lock();
				}

				if (playerEntity)
				{
					playerEntity->routingBucket = bucket;
				}
			}
		}

		return true;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ROUTING_BUCKET", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		return int(entity->routingBucket);
	}));

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_ROUTING_BUCKET", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		if (context.GetArgumentCount() > 1)
		{
			auto bucket = context.GetArgument<int>(1);

			if (bucket >= 0)
			{
				entity->routingBucket = bucket;
			}
		}

		return true;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_INVINCIBLE", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerGameState();

		return pn ? pn->isInvincible : false;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_CAMERA_ROTATION", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto camData = entity->syncTree->GetPlayerCamera();

		scrVector resultVector = { 0 };

		if (camData)
		{
			resultVector.x = camData->cameraX;
			resultVector.y = 0.0f;
			resultVector.z = camData->cameraZ;
		}
		else
		{
			resultVector.x = 0.0f;
			resultVector.y = 0.0f;
			resultVector.z = 0.0f;
		}

		return resultVector;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_TRAIN_CARRIAGE_ENGINE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto train = entity->syncTree->GetTrainState();

		if (!train)
		{
			return uint32_t(0);
		}

		auto resourceManager = fx::ResourceManager::GetCurrent();

		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		auto gameState = instance->GetComponent<fx::ServerGameState>();

		auto engine = gameState->GetEntity(0, train->engineCarriage);

		return engine ? gameState->MakeScriptHandle(engine) : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_TRAIN_CARRIAGE_INDEX", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto train = entity->syncTree->GetTrainState();

		return train ? train->carriageIndex : -1;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_FAKE_WANTED_LEVEL", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerWantedAndLOS();

		return pn ? pn->fakeWantedLevel : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_WANTED_CENTRE_POSITION", MakePlayerEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto pn = entity->syncTree->GetPlayerWantedAndLOS();

		scrVector resultVector = { 0 };

		if (pn)
		{
			resultVector.x = pn->wantedPositionX;
			resultVector.y = pn->wantedPositionY;
			resultVector.z = pn->wantedPositionZ;
		}
		else
		{
			resultVector.x = 0.0f;
			resultVector.y = 0.0f;
			resultVector.z = 0.0f;
		}

		return resultVector;
	}));

	fx::ScriptEngine::RegisterNativeHandler("IS_ENTITY_VISIBLE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		bool visible = false;
		entity->syncTree->IsEntityVisible(&visible);

		return visible;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_SOURCE_OF_DAMAGE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto node = entity->syncTree->GetPedHealth();


		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		if (!node || node->sourceOfDamage == 0)
			return (uint32_t)0;

		auto returnEntity = gameState->GetEntity(0, node->sourceOfDamage);

		if (!returnEntity)
			return (uint32_t)0;

		// Return the entity
		return gameState->MakeScriptHandle(returnEntity);
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_SOURCE_OF_DEATH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto node = entity->syncTree->GetPedHealth();


		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		if (!node || node->health > 0 || node->sourceOfDamage == 0)
			return (uint32_t)0;

		auto returnEntity = gameState->GetEntity(0, node->sourceOfDamage);

		if (!returnEntity)
			return (uint32_t)0;

		// Return the entity
		return gameState->MakeScriptHandle(returnEntity);
	}));
  
	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_CULLING_RADIUS", MakeClientFunction([](fx::ScriptContext& context, const fx::ClientSharedPtr& client)
	{
		if (context.GetArgumentCount() > 1)
		{
			float radius = context.GetArgument<float>(1);

			if (radius >= 0)
			{
				// get the current resource manager
				auto resourceManager = fx::ResourceManager::GetCurrent();

				// get the owning server instance
				auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

				// get the server's game state
				auto gameState = instance->GetComponent<fx::ServerGameState>();

				auto [lock, clientData] = gameState->ExternalGetClientData(client);
				clientData->playerCullingRadius = radius * radius;
			}
		}

		return true;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_LANDING_GEAR_STATE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		int gearState = 0;

		if (entity->type == fx::sync::NetObjEntityType::Heli)
		{
			auto state = entity->syncTree->GetHeliControl();
			if (state->hasLandingGear)
			{
				gearState = state->landingGearState;
			}
		}
		else if (entity->type == fx::sync::NetObjEntityType::Plane)
		{
			auto state = entity->syncTree->GetPlaneGameState();
			gearState = state ? state->landingGearState : 0;
		}

		return gearState;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_LOCK_ON_TARGET", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		int lockOnHandle = 0;

		if (auto state = entity->syncTree->GetPlaneGameState())
		{
			auto resourceManager = fx::ResourceManager::GetCurrent();

			auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

			auto gameState = instance->GetComponent<fx::ServerGameState>();

			auto lockOnEntity = gameState->GetEntity(0, state->lockOnEntity);

			lockOnHandle = lockOnEntity ? gameState->MakeScriptHandle(lockOnEntity) : 0;
		}

		return lockOnHandle;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_HOMING_LOCKON_STATE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto state = entity->syncTree->GetPlaneGameState();

		return state ? state->lockOnState : 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_FLIGHT_NOZZLE_POSITION", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto state = entity->syncTree->GetPlaneControl();

		return state ? state->nozzlePosition : 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_SCRIPT_TASK_COMMAND", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto tree = entity->syncTree->GetPedTaskTree();

		return tree ? tree->scriptCommand : 0x811E343C;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_SCRIPT_TASK_STAGE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto tree = entity->syncTree->GetPedTaskTree();

		return tree ? tree->scriptTaskStage : 3;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_SPECIFIC_TASK_TYPE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
#ifdef STATE_FIVE
		auto tree = entity->syncTree->GetPedTaskTree();
		if (!tree)
		{
			return Is2060() ? 531 : 530;
		}

		int index = context.GetArgument<int>(1);
		if (index < 0 || index > 7)
		{
			return Is2060() ? 531 : 530;
		}

		auto& task = tree->tasks[index];
		return static_cast<int>(task.type);
#endif

		return 0;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_SPEED", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto v = entity->syncTree->GetVelocity();

		float speed = 0.0f;

		if (v)
		{
			speed = std::hypot(v->velX, v->velY, v->velZ);
		}

		return speed;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_ATTACHED_TO", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		int handle = 0;

		if (auto attachment = entity->syncTree->GetAttachment())
		{
			if (attachment->attached)
			{
				auto resman = fx::ResourceManager::GetCurrent();
				auto instance = resman->GetComponent<fx::ServerInstanceBaseRef>()->Get();
				auto gameState = instance->GetComponent<fx::ServerGameState>();

				if (auto entity = gameState->GetEntity(0, attachment->attachedTo))
				{
					handle = gameState->MakeScriptHandle(entity);
				}
			}
		}

		return handle;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_HELI_MAIN_ROTOR_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto heliHealth = entity->syncTree->GetHeliHealth();

		return heliHealth ? float(heliHealth->mainRotorHealth) : 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_HELI_TAIL_ROTOR_HEALTH", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto heliHealth = entity->syncTree->GetHeliHealth();

		return heliHealth ? float(heliHealth->tailRotorHealth) : 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_STEERING_ANGLE", makeEntityFunction([](fx::ScriptContext& context, const fx::sync::SyncEntityPtr& entity)
	{
		auto steeringData = entity->syncTree->GetVehicleSteeringData();

		return steeringData ? steeringData->steeringAngle * (180.0f / pi) : 0.0f;
	}));

	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITY_FROM_STATE_BAG_NAME", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's game state
		auto gameState = instance->GetComponent<fx::ServerGameState>();

		auto entity = 0;

		// get the state bag name
		std::string bagName = context.CheckArgument<const char*>(0);

		// we only want to handle conversion for entity
		if (bagName.find("entity:") == 0)
		{
			int parsedEntityId = atoi(bagName.substr(7).c_str());

			auto entityPtr = gameState->GetEntity(0, parsedEntityId);

			if (entityPtr)
			{
				entity = gameState->MakeScriptHandle(entityPtr);
			}
		}

		context.SetResult(entity);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_FROM_STATE_BAG_NAME", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's client registry
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

		auto player = 0;

		// get the state bag name
		std::string bagName = context.CheckArgument<const char*>(0);

		// we only want to handle conversion for players
		if (bagName.find("player:") == 0)
		{
			int playerNetId = atoi(bagName.substr(7).c_str());

			// We don't want to return the player if they don't exist
			if (auto client = clientRegistry->GetClientByNetID(playerNetId))
			{
				player = client->GetNetId();
			};
		}

		context.SetResult(player);
	});
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		if (!IsStateGame())
		{
			return;
		}

		Init();
	},
	INT32_MIN);
});
