#include "StdInc.h"
#include <state/SyncTrees.h>

#ifndef STATE_RDR3
#include <state/ServerGameState.h>
#include <ScriptEngine.h>

#include <parser/TrainParser.h>
#include <GameServer.h>

#include <ResourceManager.h>
#include <Resource.h>
#include <fxScripting.h>

#include <boost/mpl/at.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <ServerInstanceBaseRef.h>

namespace fx
{
#pragma region helpers
template<typename TNode, typename TWrapper>
void UnparseTo(TNode& node, TWrapper wrapper)
{
	rl::MessageBuffer mb(wrapper->data.size());
	sync::SyncUnparseState state{ mb };
	node.Unparse(state);

	memcpy(wrapper->data.data(), mb.GetBuffer().data(), mb.GetBuffer().size());

	wrapper->length = mb.GetCurrentBit();
	wrapper->node = node;
}

template<typename TTree, typename TFn>
void SetupNode(const std::shared_ptr<TTree>& tree, TFn fn)
{
	using TArgs = typename boost::function_types::parameter_types<decltype(&TFn::operator())>::type;
	using TArg = typename boost::mpl::at_c<TArgs, 1>::type;
	using TNode = std::remove_reference_t<TArg>;

	auto n = tree->template GetNode<TNode>();
	fn(n->node);

	UnparseTo(n->node, n);

	n->frameIndex = 12;
	n->timestamp = msec().count();
}

template<typename TSectorNode, typename TPositionNode, typename TTree>
void SetupPosition(const std::shared_ptr<TTree>& tree, float posX, float posY, float posZ)
{
	int sectorX = int((posX / 54.0f) + 512.0f);
	int sectorY = int((posY / 54.0f) + 512.0f);
	int sectorZ = int((posZ + 1700.0f) / 69.0f);

	float sectorPosX = posX - ((sectorX - 512.0f) * 54.0f);
	float sectorPosY = posY - ((sectorY - 512.0f) * 54.0f);
	float sectorPosZ = posZ - ((sectorZ * 69.0f) - 1700.0f);

	SetupNode(tree, [sectorX, sectorY, sectorZ](TSectorNode& cdn)
	{
		cdn.m_sectorX = sectorX;
		cdn.m_sectorY = sectorY;
		cdn.m_sectorZ = sectorZ;
	});

	SetupNode(tree, [sectorPosX, sectorPosY, sectorPosZ](TPositionNode& cdn)
	{
		cdn.m_posX = sectorPosX;
		cdn.m_posY = sectorPosY;
		cdn.m_posZ = sectorPosZ;
	});
}

template<typename TTree>
void SetupHeading(const std::shared_ptr<TTree>& tree, float heading)
{
	SetupNode(tree, [heading](sync::CEntityOrientationDataNode& node)
	{
		glm::quat q = glm::quat(glm::vec3(0.0f, 0.0f, heading * 0.01745329252f));
		node.data.quat.Load(q.x, q.y, q.z, q.w);
	});
}
#pragma endregion

std::shared_ptr<sync::SyncTreeBase> MakeAutomobile(uint32_t model, float posX, float posY, float posZ, uint32_t resourceHash, float heading = 0.0f)
{
	auto tree = std::make_shared<sync::CAutomobileSyncTree>();
	
	SetupNode(tree, [model](sync::CVehicleCreationDataNode& cdn)
	{
		cdn.m_model = model;
		cdn.m_creationToken = msec().count();
		cdn.m_needsToBeHotwired = false;
		cdn.m_maxHealth = 1000;
		cdn.m_popType = sync::POPTYPE_MISSION;
		cdn.m_randomSeed = rand();
		cdn.m_tyresDontBurst = false;
		cdn.m_vehicleStatus = 2;
		cdn.m_unk5 = false;
	});

	SetupNode(tree, [](sync::CAutomobileCreationDataNode& cdn)
	{
		cdn.allDoorsClosed = true;
	});

	SetupPosition<sync::CSectorDataNode, sync::CSectorPositionDataNode>(tree, posX, posY, posZ);
	SetupHeading(tree, heading);

	SetupNode(tree, [resourceHash](sync::CEntityScriptInfoDataNode& cdn)
	{
		cdn.m_scriptHash = resourceHash;
		cdn.m_timestamp = msec().count();
	});

	return tree;
}

template<typename TTree>
std::shared_ptr<sync::SyncTreeBase> MakeVehicle(uint32_t model, float posX, float posY, float posZ, uint32_t resourceHash, float heading = 0.0f)
{
	auto tree = std::make_shared<TTree>();

	SetupNode(tree, [model](sync::CVehicleCreationDataNode& cdn)
	{
		cdn.m_model = model;
		cdn.m_creationToken = msec().count();
		cdn.m_needsToBeHotwired = false;
		cdn.m_maxHealth = 1000;
		cdn.m_popType = sync::POPTYPE_MISSION;
		cdn.m_randomSeed = rand();
		cdn.m_tyresDontBurst = false;
		cdn.m_vehicleStatus = 2;
		cdn.m_unk5 = false;
	});

	if constexpr (std::is_same_v<TTree, sync::CAutomobileSyncTree> ||
		std::is_same_v<TTree, sync::CTrailerSyncTree> ||
		std::is_same_v<TTree, sync::CHeliSyncTree>)
	{
		SetupNode(tree, [](sync::CAutomobileCreationDataNode& cdn)
		{
			cdn.allDoorsClosed = true;
		});
	}

	SetupPosition<sync::CSectorDataNode, sync::CSectorPositionDataNode>(tree, posX, posY, posZ);
	SetupHeading(tree, heading);

	SetupNode(tree, [resourceHash](sync::CEntityScriptInfoDataNode& cdn)
	{
		cdn.m_scriptHash = resourceHash;
		cdn.m_timestamp = msec().count();
	});

	return tree;
}

template<typename TTree>
std::shared_ptr<sync::CTrainSyncTree> MakeTrain(uint32_t model, float posX, float posY, float posZ, float heading, uint32_t resourceHash, bool isEngine, bool direction, bool stopAtStation, float speed, int trackid, int trainConfigIndex)
{
	auto tree = std::make_shared<sync::CTrainSyncTree>();

	SetupNode(tree, [model](sync::CVehicleCreationDataNode& cdn)
	{
		cdn.m_model = model;
		cdn.m_creationToken = msec().count();
		cdn.m_needsToBeHotwired = false;
		cdn.m_maxHealth = 1000;
		cdn.m_popType = sync::POPTYPE_MISSION;
		cdn.m_randomSeed = rand();
		cdn.m_tyresDontBurst = false;
		cdn.m_vehicleStatus = 2;
		cdn.m_unk5 = false;
	});

	SetupPosition<sync::CSectorDataNode, sync::CSectorPositionDataNode>(tree, posX, posY, posZ);
	SetupHeading(tree, heading);

	SetupNode(tree, [resourceHash](sync::CEntityScriptInfoDataNode& cdn)
	{
		cdn.m_scriptHash = resourceHash;
		cdn.m_timestamp = msec().count();
	});

	SetupNode(tree, [isEngine, speed, direction, trackid, trainConfigIndex, stopAtStation](sync::CTrainGameStateDataNode& cdn)
	{
		cdn.data.linkedToBackwardId = 0;
		cdn.data.linkedToForwardId = 0;
		cdn.data.distanceFromEngine = 0.0f;

		cdn.data.trainState = 3;

		cdn.data.isEngine = isEngine;
		cdn.data.cruiseSpeed = speed;
		cdn.data.direction = direction;
		cdn.data.trackId = trackid;
		cdn.data.trainConfigIndex = trainConfigIndex;

		cdn.data.isCaboose = true;

		cdn.data.isMissionTrain = true;
		cdn.data.hasPassengerCarriages = true;
		cdn.data.renderDerailed = false;

		if (Is2372())
		{
			cdn.data.allowRemovalByPopulation = false;
			cdn.data.shouldStopAtStations = stopAtStation;
			cdn.data.highPrecisionBlending = false;
		}
	});

	return tree;
}

std::shared_ptr<sync::SyncTreeBase> MakePed(uint32_t model, float posX, float posY, float posZ, uint32_t resourceHash, float heading = 0.0f)
{
	auto tree = std::make_shared<sync::CPedSyncTree>();

	SetupNode(tree, [model](sync::CPedCreationDataNode& cdn)
	{
		cdn.m_model = model;
		cdn.isRespawnObjectId = false;
		cdn.respawnFlaggedForRemoval = false;
		cdn.m_popType = sync::POPTYPE_MISSION;
		cdn.randomSeed = rand();
		cdn.vehicleId = 0;
		cdn.vehicleSeat = 0;
		cdn.prop = 0;
		cdn.voiceHash = HashString("NO_VOICE");
		cdn.isStanding = true;
		cdn.attributeDamageToPlayer = -1;
		cdn.maxHealth = 200;
		cdn.unkBool = false;
	});

	SetupNode(tree, [resourceHash](sync::CPedSectorPosMapNode& cdn)
	{
		cdn.isStandingOn = false;
		cdn.isNM = false;
	});

	SetupPosition<sync::CSectorDataNode, sync::CPedSectorPosMapNode>(tree, posX, posY, posZ);

	SetupNode(tree, [heading](sync::CPedOrientationDataNode& node)
	{
		node.data.currentHeading = heading * 0.01745329252f;
		node.data.desiredHeading = heading * 0.01745329252f;
	});

	SetupNode(tree, [resourceHash](sync::CEntityScriptInfoDataNode& cdn)
	{
		cdn.m_scriptHash = resourceHash;
		cdn.m_timestamp = msec().count();
	});

	return tree;
}

std::shared_ptr<sync::SyncTreeBase> MakeObject(uint32_t model, float posX, float posY, float posZ, uint32_t resourceHash, bool dynamic, float heading = 0.0f)
{
	auto tree = std::make_shared<sync::CObjectSyncTree>();

	SetupNode(tree, [model, dynamic](sync::CObjectCreationDataNode& cdn)
	{
		cdn.m_model = model;
		cdn.m_hasInitPhysics = dynamic;
	});

	SetupNode(tree, [resourceHash](sync::CObjectSectorPosNode& cdn)
	{
		cdn.highRes = true;
	});

	SetupPosition<sync::CSectorDataNode, sync::CObjectSectorPosNode>(tree, posX, posY, posZ);
	
	SetupNode(tree, [heading](sync::CObjectOrientationDataNode& node)
	{
		node.data.highRes = false;

		glm::quat q = glm::quat(glm::vec3(0.0f, 0.0f, heading * 0.01745329252f));
		node.data.quat.Load(q.x, q.y, q.z, q.w);
	});

	SetupNode(tree, [resourceHash](sync::CEntityScriptInfoDataNode& cdn)
	{
		cdn.m_scriptHash = resourceHash;
		cdn.m_timestamp = msec().count();
	});

	return tree;
}

template<typename TNode>
auto GetNode(sync::NetObjEntityType objectType, const std::shared_ptr<sync::SyncTreeBase>& tree)
{
	switch (objectType)
	{
		case sync::NetObjEntityType::Automobile:
			return std::static_pointer_cast<sync::CAutomobileSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Bike:
			return std::static_pointer_cast<sync::CBikeSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Boat:
			return std::static_pointer_cast<sync::CBoatSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Door:
			return std::static_pointer_cast<sync::CDoorSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Heli:
			return std::static_pointer_cast<sync::CHeliSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Object:
			return std::static_pointer_cast<sync::CObjectSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Ped:
			return std::static_pointer_cast<sync::CPedSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Pickup:
			return std::static_pointer_cast<sync::CPickupSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::PickupPlacement:
			return std::static_pointer_cast<sync::CPickupPlacementSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Plane:
			return std::static_pointer_cast<sync::CPlaneSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Submarine:
			return std::static_pointer_cast<sync::CSubmarineSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Player:
			return std::static_pointer_cast<sync::CPlayerSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Trailer:
			return std::static_pointer_cast<sync::CAutomobileSyncTree>(tree)->GetNode<TNode>();
		case sync::NetObjEntityType::Train:
			return std::static_pointer_cast<sync::CTrainSyncTree>(tree)->GetNode<TNode>();
	}

	assert(!"Invalid object type!");
	return static_cast<sync::CAutomobileSyncTree*>(nullptr)->GetNode<TNode>();
}

void DisownEntityScript(const fx::sync::SyncEntityPtr& entity)
{
	auto tree = entity->syncTree;

	if (tree)
	{
		auto n = GetNode<sync::CEntityScriptInfoDataNode>(entity->type, tree);

		if (n)
		{
			auto& cdn = n->node;
			cdn.m_scriptHash = 0;
			cdn.m_timestamp = msec().count();
			UnparseTo(cdn, n);

			n->frameIndex = 12;
			n->timestamp = msec().count();
		}
	}
}

static auto GetTrainCarriageCount(fx::ServerGameState* sgs, const fx::sync::SyncEntityPtr& engine) -> int
{
	if (engine->type != fx::sync::NetObjEntityType::Train) 
	{
		return -1;
	}

	int carriageCount = 0;
	for (auto link = GetNextTrain(sgs, engine); link; link = GetNextTrain(sgs, link))
	{
		carriageCount++;
	}

	return carriageCount;
};

static auto GetTrainCabooseCarriage(fx::ServerGameState* sgs, const fx::sync::SyncEntityPtr& engine) -> fx::sync::SyncEntityPtr
{
	fx::sync::SyncEntityPtr caboose = engine;
	for (auto link = GetNextTrain(sgs, engine); link; link = GetNextTrain(sgs, link))
	{
		auto state = link->syncTree->GetTrainState();

		if (state->linkedToBackwardId == 0 || state->isCaboose)
		{
			caboose = link;
			break;
		}
	}
	return caboose;
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* ref)
	{
		if (!IsStateGame())
		{
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler("CREATE_AUTOMOBILE", [=](fx::ScriptContext& ctx) 
		{
			uint32_t resourceHash = 0;

			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					resourceHash = HashString(resource->GetName().c_str());
				}
			}

			auto tree = MakeAutomobile(ctx.GetArgument<uint32_t>(0), ctx.GetArgument<float>(1), ctx.GetArgument<float>(2), ctx.GetArgument<float>(3), resourceHash, ctx.GetArgument<float>(4));

			auto sgs = ref->GetComponent<fx::ServerGameState>();
			auto entity = sgs->CreateEntityFromTree(sync::NetObjEntityType::Automobile, tree);

			ctx.SetResult(sgs->MakeScriptHandle(entity));
		});

		fx::ScriptEngine::RegisterNativeHandler("CREATE_VEHICLE_SERVER_SETTER", [ref](fx::ScriptContext& ctx)
		{
			uint32_t modelHash = ctx.GetArgument<uint32_t>(0);
			std::string_view type = ctx.CheckArgument<const char*>(1);
			float x = ctx.GetArgument<float>(2);
			float y = ctx.GetArgument<float>(3);
			float z = ctx.GetArgument<float>(4);
			float heading = ctx.GetArgument<float>(5);

			uint32_t resourceHash = 0;
			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					resourceHash = HashString(resource->GetName().c_str());
				}
			}

			std::shared_ptr<fx::sync::SyncTreeBase> tree;
			auto typeId = (fx::sync::NetObjEntityType)-1;

			if (type == "automobile")
			{
				typeId = sync::NetObjEntityType::Automobile;
			}
			else if (type == "bike")
			{
				typeId = sync::NetObjEntityType::Bike;
			}
			else if (type == "boat")
			{
				typeId = sync::NetObjEntityType::Boat;
			}
			else if (type == "heli")
			{
				typeId = sync::NetObjEntityType::Heli;
			}
			else if (type == "plane")
			{
				typeId = sync::NetObjEntityType::Plane;
			}
			else if (type == "submarine")
			{
				typeId = sync::NetObjEntityType::Submarine;
			}
			else if (type == "trailer")
			{
				typeId = sync::NetObjEntityType::Trailer;
			}
			else if (type == "train")
			{
				typeId = sync::NetObjEntityType::Train;
			}
			else
			{
				throw std::runtime_error(va("CREATE_VEHICLE_SERVER_SETTER: Invalid entity type %s", type));
			}

			switch (typeId)
			{
				case fx::sync::NetObjEntityType::Automobile:
					tree = MakeVehicle<fx::sync::CAutomobileSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
				case fx::sync::NetObjEntityType::Bike:
					tree = MakeVehicle<fx::sync::CBikeSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
				case fx::sync::NetObjEntityType::Boat:
					tree = MakeVehicle<fx::sync::CBoatSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
				case fx::sync::NetObjEntityType::Heli:
					tree = MakeVehicle<fx::sync::CHeliSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
				case fx::sync::NetObjEntityType::Plane:
					tree = MakeVehicle<fx::sync::CPlaneSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
				case fx::sync::NetObjEntityType::Submarine:
					tree = MakeVehicle<fx::sync::CSubmarineSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
				case fx::sync::NetObjEntityType::Trailer:
					tree = MakeVehicle<fx::sync::CTrailerSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
				case fx::sync::NetObjEntityType::Train:
					tree = MakeVehicle<fx::sync::CTrainSyncTree>(modelHash, x, y, z, resourceHash, heading);
					break;
			}

			auto sgs = ref->GetComponent<fx::ServerGameState>();
			auto entity = sgs->CreateEntityFromTree(typeId, tree);

			ctx.SetResult(sgs->MakeScriptHandle(entity));
		});

		fx::ScriptEngine::RegisterNativeHandler("CREATE_PED", [=](fx::ScriptContext& ctx)
		{
			uint32_t resourceHash = 0;

			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					resourceHash = HashString(resource->GetName().c_str());
				}
			}

			auto tree = MakePed(ctx.GetArgument<uint32_t>(1), ctx.GetArgument<float>(2), ctx.GetArgument<float>(3), ctx.GetArgument<float>(4), resourceHash, ctx.GetArgument<float>(5));

			auto sgs = ref->GetComponent<fx::ServerGameState>();
			auto entity = sgs->CreateEntityFromTree(sync::NetObjEntityType::Ped, tree);

			ctx.SetResult(sgs->MakeScriptHandle(entity));
		});

		fx::ScriptEngine::RegisterNativeHandler("CREATE_TRAIN", [=](fx::ScriptContext& ctx)
		{
			uint32_t resourceHash = 0;

			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					resourceHash = HashString(resource->GetName().c_str());
				}
			}

			uint32_t modelHash = ctx.GetArgument<uint32_t>(0);
			float x = ctx.GetArgument<float>(1);
			float y = ctx.GetArgument<float>(2);
			float z = ctx.GetArgument<float>(3);
			bool direction = ctx.GetArgument<bool>(4);
			bool stopAtStations = ctx.GetArgument<bool>(5);
			float speed = ctx.GetArgument<float>(6);
			int trackId = ctx.GetArgument<int>(7);
			int trainConfigIndex = ctx.CheckArgument<int>(8);
			
			auto& trackConfigs = ref->GetComponent<fx::CTrainTrackParser>();

			static int32_t maxTrainTrackIndex = 11;

			//Don't create train if an invalid trackId has been provided.
			if (trackId < 0 || trackId > (trackConfigs->GetData().empty() ? maxTrainTrackIndex : trackConfigs->GetData().size()))
			{
				throw std::runtime_error(va("Tried to spawn train on invalid track id: %u", trackId));

				ctx.SetResult(0);
				return;
			}

			auto& trainConfigs = ref->GetComponent<fx::CTrainConfigParser>();

			static int32_t maxTrainConfigIndex = (int32_t)(Is3095() ? 27 : (Is2802() ? 26 : (Is2372() ? 25 : 24)));

			//Don't create train if an invalid trainConfigIndex has been provided, otherwise the client crashes.
			if (trainConfigIndex < 0 || trainConfigIndex > (trainConfigs->GetData().empty() ? maxTrainConfigIndex : trainConfigs->GetData().size()))
			{
				throw std::runtime_error(va("Tried to spawn train with invalid train config index: %u", trainConfigIndex));

				ctx.SetResult(0);
				return;
			}

			auto tree = MakeTrain<sync::CTrainSyncTree>(modelHash, x, y, z, 0.0f, resourceHash, true, direction, stopAtStations, speed, trackId, trainConfigIndex);
			auto sgs = ref->GetComponent<fx::ServerGameState>();
			auto entity = sgs->CreateEntityFromTree(sync::NetObjEntityType::Train, tree);

			//Edit CTrainGameStateData node to assign engineCarriage to the newly created objectID
			/*
			SetupNode(tree, [entity](sync::CTrainGameStateDataNode& cdn)
			{
				cdn.data.engineCarriage = entity->handle;
			});
			*/

			ctx.SetResult(sgs->MakeScriptHandle(entity));
		});

		fx::ScriptEngine::RegisterNativeHandler("CREATE_TRAIN_CARRIAGE", [=](fx::ScriptContext& ctx)
		{
			// get the current resource manager
			auto resourceManager = fx::ResourceManager::GetCurrent();

			// get the owning server instance
			auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

			// get the server's game state
			auto gameState = instance->GetComponent<fx::ServerGameState>();

			// parse the client ID
			auto id = ctx.GetArgument<uint32_t>(0);

			if (!id)
			{
				ctx.SetResult(0);
				return;
			}

			auto entity = gameState->GetEntity(id);

			//Make sure entity exists.
			if (!entity)
			{
				throw std::runtime_error(va("Tried to access invalid entity: %d", id));

				ctx.SetResult(0);
				return;
			}

			auto trainState = entity->syncTree->GetTrainState();

			// Make sure entity is a train
			if (entity->type != fx::sync::NetObjEntityType::Train)
			{
				throw std::runtime_error(va("Entity is not a train: %d", id));

				ctx.SetResult(0);
				return;
			}

			// Make sure entity is the engine carriage
			if (!trainState || !trainState->isEngine)
			{
				throw std::runtime_error(va("Tried to attach train carriage to invalid train entity: %d", id));

				ctx.SetResult(0);
				return;
			}

			uint32_t resourceHash = 0;
			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					resourceHash = HashString(resource->GetName().c_str());
				}
			}

			uint32_t modelHash = ctx.GetArgument<uint32_t>(1);
			float distanceFromEngine = ctx.GetArgument<float>(2);

			float position[3];
			entity->syncTree->GetPosition(position);

			auto tree = MakeTrain<sync::CTrainSyncTree>(modelHash, position[0], position[1], position[2], 0.0f, resourceHash, false, trainState->direction, trainState->shouldStopAtStations, trainState->cruiseSpeed, trainState->trackId, trainState->trainConfigIndex);
			auto& sgs = ref->GetComponent<fx::ServerGameState>();

			if (tree) 
			{
				SetupNode(tree, [sgs, entity, trainState, distanceFromEngine](sync::CTrainGameStateDataNode& cdn){
					cdn.data.engineCarriage = entity->handle;
					cdn.data.linkedToForwardId = GetTrainCabooseCarriage(sgs.GetRef(), entity)->handle;
					cdn.data.carriageIndex = GetTrainCarriageCount(sgs.GetRef(), entity) + 1;
					// If the train direction is forward the distanceFromEngine has to be negative
					cdn.data.distanceFromEngine = trainState->direction ? -distanceFromEngine : +distanceFromEngine;
					cdn.data.isEngine = false;
					cdn.data.isCaboose = true;		
				});
			}

			auto carriageEntity = sgs->CreateEntityFromTree(fx::sync::NetObjEntityType::Train, tree);
			auto caboose = GetTrainCabooseCarriage(sgs.GetRef(), entity);

			auto trainSyncTree = std::static_pointer_cast<sync::CTrainSyncTree>(caboose->syncTree);
			SetupNode(trainSyncTree, [carriageEntity](sync::CTrainGameStateDataNode& cdn)
			{
				cdn.data.linkedToBackwardId = carriageEntity->handle;
				cdn.data.isCaboose = false;
			});

			ctx.SetResult(sgs->MakeScriptHandle(carriageEntity));
		});

		fx::ScriptEngine::RegisterNativeHandler("CREATE_OBJECT_NO_OFFSET", [=](fx::ScriptContext& ctx)
		{
			uint32_t resourceHash = 0;

			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					resourceHash = HashString(resource->GetName().c_str());
				}
			}

			auto tree = MakeObject(ctx.GetArgument<uint32_t>(0), ctx.GetArgument<float>(1), ctx.GetArgument<float>(2), ctx.GetArgument<float>(3), resourceHash, ctx.GetArgument<bool>(6));

			auto sgs = ref->GetComponent<fx::ServerGameState>();
			auto entity = sgs->CreateEntityFromTree(sync::NetObjEntityType::Object, tree);

			ctx.SetResult(sgs->MakeScriptHandle(entity));
		});
	});
});
}
#else
namespace fx
{
void DisownEntityScript(const fx::sync::SyncEntityPtr& entity)
{
}
}
#endif
