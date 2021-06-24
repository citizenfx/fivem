#include <StdInc.h>

#include <ServerInstanceBaseRef.h>
#include <GameServer.h>
#include <state/ServerGameStatePublic.h>

#include <ServerGui.h>
#include <imgui.h>

struct EntityMetaData
{
	std::string entityType;
	uint32_t model;
	glm::vec3 position;
	std::string popType;
};

static const auto& CollectEntities(fx::ServerInstanceBase* instance)
{
	auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();

	auto curTime = msec();
	static auto lastCollect = msec();

	static std::map<std::string, std::map<uint16_t, EntityMetaData>> entities;

	if ((curTime - lastCollect) > 500ms)
	{
		entities.clear();

		// collect entities
		{
			sgs->ForAllEntities([](fx::sync::Entity* entity)
			{
				auto owner = entity->GetOwner();
				auto ownerName = (owner) ? fmt::sprintf("[%d] %s", owner->GetNetId(), owner->GetName()) : "[-1] Server";

				auto objectId = entity->GetId();
				uint32_t model = entity->GetModel();
				auto position = entity->GetPosition();
				auto popType = entity->GetPopType();
				auto type = entity->GetType();

				entities[ownerName][objectId] = {
					type,
					model,
					position,
					popType
				};
			});
		}

		lastCollect = curTime;
	}

	return entities;
}

static SvGuiModule netobjviewer("Network Object Viewer", "netobjviewer", 0, [](fx::ServerInstanceBase* instance)
{
	const auto& entityData = CollectEntities(instance);
	static std::tuple<uint16_t, EntityMetaData> curObj;

	for (const auto& [owner, data] : entityData)
	{
		if (ImGui::TreeNode(owner.c_str()))
		{
			std::map<std::string, std::set<uint16_t>> popTypeMap;

			for (const auto& [id, instData] : data)
			{
				popTypeMap[instData.popType].insert(id);
			}

			for (const auto& [popType, ids] : popTypeMap)
			{
				if (ImGui::TreeNode(popType.c_str()))
				{
					for (auto id : ids)
					{
						const auto& instData = data.find(id)->second;

						if (ImGui::TreeNodeEx(fmt::sprintf("%d - %s (%08x)", id, instData.entityType, instData.model).c_str(), ImGuiTreeNodeFlags_Leaf | ((id == std::get<0>(curObj)) ? ImGuiTreeNodeFlags_Selected : 0)))
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
