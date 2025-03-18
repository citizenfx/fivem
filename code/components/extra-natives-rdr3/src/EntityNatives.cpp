#include "StdInc.h"
#include "Hooking.h"
#include <EntitySystem.h>
#include <ScriptEngine.h>
#include <Pool.h>
#include <ScriptSerialization.h>

static hook::cdecl_stub<uint32_t(fwEntity*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("32 DB E8 ? ? ? ? 48 85 C0 75 ? 8A 05", -35);
});

constexpr const char* GetPoolNameForEntityType(int entityType)
{
	constexpr std::array<const char*, 4> poolNames = {
		"Invalid", // 0
		"Peds", // 1
		"CVehicle", // 2
		"Object" // 3
	};
	return (entityType >= 1 && entityType <= 3)
		   ? poolNames[entityType]
		   : "Object";
}

static HookFunction hookFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_ENTITIES_IN_RADIUS", [](fx::ScriptContext& context)
	{
		float checkX = context.GetArgument<float>(0);
		float checkY = context.GetArgument<float>(1);
		float checkZ = context.GetArgument<float>(2);
		float radius = context.GetArgument<float>(3);
		int entityType = context.GetArgument<int>(4);
		bool sortOutput = context.GetArgument<bool>(5);
		fx::scrObject models = context.GetArgument<fx::scrObject>(6);

		std::vector<std::pair<float, int>> entities;

		std::vector<int> modelList = fx::DeserializeObject<std::vector<int>>(models);
		std::unordered_set<int> modelSet(modelList.begin(), modelList.end());

		float squaredMaxDistance = radius * radius;

		auto objectPool = rage::GetPool<fwEntity>(GetPoolNameForEntityType(entityType));
		for (int i = 0; i < objectPool->GetSize(); i++)
		{
			fwEntity* entity = objectPool->GetAt(i);
			if (!entity)
				continue;

			auto position = entity->GetPosition();

			float dx = position.x - checkX;
			float dy = position.y - checkY;
			float dz = position.z - checkZ;
			float distSq = dx * dx + dy * dy + dz * dz;

			if (distSq >= squaredMaxDistance)
				continue;

			auto modelHash = entity->GetArchetype()->hash;

			if (modelSet.empty() || modelSet.find(modelHash) != modelSet.end())
			{
				entities.push_back({ distSq, getScriptGuidForEntity(entity) });
			}
		}

		if (sortOutput)
		{
			std::sort(entities.begin(), entities.end(), [](const auto& a, const auto& b)
			{
				return a.first < b.first;
			});
		}

		std::vector<int> entityList;
		entityList.reserve(entities.size());
		for (auto& entry : entities)
		{
			entityList.push_back(entry.second);
		}

		context.SetResult(fx::SerializeObject(entityList));
	});
});
