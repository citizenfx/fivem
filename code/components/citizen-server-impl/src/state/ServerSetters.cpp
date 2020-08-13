#include "StdInc.h"
#include <state/SyncTrees_Five.h>

#include <state/ServerGameState.h>
#include <ScriptEngine.h>

#include <GameServer.h>

#include <Resource.h>
#include <fxScripting.h>

namespace fx
{
template<typename TNode, typename TWrapper>
void UnparseTo(TNode& node, TWrapper wrapper)
{
	rl::MessageBuffer mb(wrapper->data.size());
	node.Unparse(mb);

	memcpy(wrapper->data.data(), mb.GetBuffer().data(), mb.GetBuffer().size());

	wrapper->length = mb.GetCurrentBit();
	wrapper->node = node;
}

std::shared_ptr<sync::SyncTreeBase> MakeAutomobile(uint32_t model, float posX, float posY, float posZ, uint32_t resourceHash)
{
	auto tree = std::make_shared<sync::CAutomobileSyncTree>();
	
	{
		auto n = tree->GetNode<sync::CVehicleCreationDataNode>();
		auto& cdn = n->node;
		cdn.m_model = model;
		cdn.m_creationToken = msec().count();
		cdn.m_needsToBeHotwired = false;
		cdn.m_maxHealth = 1000;
		cdn.m_popType = sync::POPTYPE_MISSION;
		cdn.m_randomSeed = rand();
		cdn.m_tyresDontBurst = false;
		cdn.m_vehicleStatus = 0;
		cdn.m_unk5 = false;
		UnparseTo(cdn, n);

		n->frameIndex = 12;
		n->timestamp = msec().count();
	}

	{
		auto n = tree->GetNode<sync::CAutomobileCreationDataNode>();
		auto& cdn = n->node;
		cdn.allDoorsClosed = true;
		UnparseTo(cdn, n);

		n->frameIndex = 12;
		n->timestamp = msec().count();
	}

	int sectorX = int((posX / 54.0f) + 512.0f);
	int sectorY = int((posY / 54.0f) + 512.0f);
	int sectorZ = int((posZ + 1700.0f) / 69.0f);

	float sectorPosX = posX - ((sectorX - 512.0f) * 54.0f);
	float sectorPosY = posY - ((sectorY - 512.0f) * 54.0f);
	float sectorPosZ = posZ - ((sectorZ * 69.0f) - 1700.0f);

	{
		auto n = tree->GetNode<sync::CSectorDataNode>();
		auto& cdn = n->node;
		cdn.m_sectorX = sectorX;
		cdn.m_sectorY = sectorY;
		cdn.m_sectorZ = sectorZ;
		UnparseTo(cdn, n);

		n->frameIndex = 12;
		n->timestamp = msec().count();
	}

	{
		auto n = tree->GetNode<sync::CSectorPositionDataNode>();
		auto& cdn = n->node;
		cdn.m_posX = sectorPosX;
		cdn.m_posY = sectorPosY;
		cdn.m_posZ = sectorPosZ;
		UnparseTo(cdn, n);

		n->frameIndex = 12;
		n->timestamp = msec().count();
	}

	{
		auto n = tree->GetNode<sync::CEntityScriptInfoDataNode>();
		auto& cdn = n->node;
		cdn.m_scriptHash = resourceHash;
		cdn.m_timestamp = msec().count();
		UnparseTo(cdn, n);

		n->frameIndex = 12;
		n->timestamp = msec().count();
	}

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

			auto tree = MakeAutomobile(ctx.GetArgument<uint32_t>(0), ctx.GetArgument<float>(1), ctx.GetArgument<float>(2), ctx.GetArgument<float>(3), resourceHash);

			auto sgs = ref->GetComponent<fx::ServerGameState>();
			auto entity = sgs->CreateEntityFromTree(sync::NetObjEntityType::Automobile, tree);

			ctx.SetResult(sgs->MakeScriptHandle(entity));
		});
	});
});
}
