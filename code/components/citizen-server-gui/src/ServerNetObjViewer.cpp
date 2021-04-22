#include <StdInc.h>
#include <state/ServerGameState.h>

#include <ServerGui.h>
#include <imgui.h>

struct EntityMetaData
{
	fx::sync::NetObjEntityType entityType;
	uint32_t model;
	glm::vec3 position;
	fx::sync::ePopType popType;
};

static const auto& CollectEntities(fx::ServerInstanceBase* instance)
{
	auto sgs = instance->GetComponent<fx::ServerGameState>();

	auto curTime = msec();
	static auto lastCollect = msec();

	static std::map<std::string, std::map<uint16_t, EntityMetaData>> entities;

	if ((curTime - lastCollect) > 500ms)
	{
		entities.clear();

		// collect entities
		{
			std::shared_lock lock(sgs->m_entityListMutex);
			for (auto& entity : sgs->m_entityList)
			{
				auto owner = entity->GetClient();
				auto ownerName = (owner) ? fmt::sprintf("[%d] %s", owner->GetNetId(), owner->GetName()) : "[-1] Server";

				auto objectId = entity->handle;
				uint32_t model = 0;
				glm::vec3 position{0, 0, 0};
				fx::sync::ePopType popType = fx::sync::POPTYPE_UNKNOWN;

				if (entity->syncTree)
				{
					float positionFloat[3];
					entity->syncTree->GetPosition(positionFloat);

					position = { positionFloat[0],
						positionFloat[1],
						positionFloat[2] };

					entity->syncTree->GetModelHash(&model);

					entity->syncTree->GetPopulationType(&popType);
				}

				entities[ownerName][objectId] = {
					entity->type,
					model,
					position,
					popType
				};
			}
		}

		lastCollect = curTime;
	}

	return entities;
}

static const char* PopTypeToString(fx::sync::ePopType type)
{
	using namespace fx;

	switch (type)
	{
		case fx::sync::POPTYPE_UNKNOWN:
			return "POPTYPE_UNKNOWN";
		case fx::sync::POPTYPE_RANDOM_PERMANENT:
			return "POPTYPE_RANDOM_PERMANENT";
		case fx::sync::POPTYPE_RANDOM_PARKED:
			return "POPTYPE_RANDOM_PARKED";
		case fx::sync::POPTYPE_RANDOM_PATROL:
			return "POPTYPE_RANDOM_PATROL";
		case fx::sync::POPTYPE_RANDOM_SCENARIO:
			return "POPTYPE_RANDOM_SCENARIO";
		case fx::sync::POPTYPE_RANDOM_AMBIENT:
			return "POPTYPE_RANDOM_AMBIENT";
		case fx::sync::POPTYPE_PERMANENT:
			return "POPTYPE_PERMANENT";
		case fx::sync::POPTYPE_MISSION:
			return "POPTYPE_MISSION";
		case fx::sync::POPTYPE_REPLAY:
			return "POPTYPE_REPLAY";
		case fx::sync::POPTYPE_CACHE:
			return "POPTYPE_CACHE";
		case fx::sync::POPTYPE_TOOL:
			return "POPTYPE_TOOL";
		default:
			return "";	
	}
}

static const char* TypeToString(fx::sync::NetObjEntityType type)
{
	using namespace fx;

	switch (type)
	{
		case sync::NetObjEntityType::Automobile:
			return "Automobile";
		case sync::NetObjEntityType::Bike:
			return "Bike";
		case sync::NetObjEntityType::Boat:
			return "Boat";
		case sync::NetObjEntityType::Door:
			return "Door";
		case sync::NetObjEntityType::Heli:
			return "Heli";
		case sync::NetObjEntityType::Object:
			return "Object";
		case sync::NetObjEntityType::Ped:
			return "Ped";
		case sync::NetObjEntityType::Pickup:
			return "Pickup";
		case sync::NetObjEntityType::PickupPlacement:
			return "PickupPlacement";
		case sync::NetObjEntityType::Plane:
			return "Plane";
		case sync::NetObjEntityType::Submarine:
			return "Submarine";
		case sync::NetObjEntityType::Player:
			return "Player";
		case sync::NetObjEntityType::Trailer:
			return "Trailer";
		case sync::NetObjEntityType::Train:
			return "Train";
	}
}

static SvGuiModule netobjviewer("Network Object Viewer", "netobjviewer", 0, [](fx::ServerInstanceBase* instance)
{
	const auto& entityData = CollectEntities(instance);
	static std::tuple<uint16_t, EntityMetaData> curObj;

	for (const auto& [owner, data] : entityData)
	{
		if (ImGui::TreeNode(owner.c_str()))
		{
			std::map<fx::sync::ePopType, std::set<uint16_t>> popTypeMap;

			for (const auto& [id, instData] : data)
			{
				popTypeMap[instData.popType].insert(id);
			}

			for (const auto& [popType, ids] : popTypeMap)
			{
				if (ImGui::TreeNode(PopTypeToString(popType)))
				{
					for (auto id : ids)
					{
						const auto& instData = data.find(id)->second;

						if (ImGui::TreeNodeEx(fmt::sprintf("%d - %s (%08x)", id, TypeToString(instData.entityType), instData.model).c_str(), ImGuiTreeNodeFlags_Leaf | ((id == std::get<0>(curObj)) ? ImGuiTreeNodeFlags_Selected : 0)))
						{
							if (ImGui::IsItemClicked())
							{
								curObj = { id, instData };
							}

							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}
	}

	if (std::get<0>(curObj))
	{
		const auto& instData = std::get<1>(curObj);
		ImGui::Text("(%f, %f, %f)", instData.position.x, instData.position.y, instData.position.z);
	}
});
