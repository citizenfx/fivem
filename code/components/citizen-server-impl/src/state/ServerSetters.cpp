#include "StdInc.h"
#include <state/SyncTrees.h>

#ifndef STATE_RDR3
#include <state/ServerGameState.h>
#include <ScriptEngine.h>

#include <GameServer.h>

#include <Resource.h>
#include <fxScripting.h>

#include <boost/mpl/at.hpp>
#include <boost/function_types/parameter_types.hpp>

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
		cdn.m_vehicleStatus = 0;
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
		cdn.m_dynamic = dynamic;
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
