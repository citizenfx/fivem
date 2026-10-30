#include "StdInc.h"
#include "Hooking.h"
#include <EntitySystem.h>
#include <ScriptEngine.h>
#include <Pool.h>
#include <ScriptSerialization.h>
#include "ClientConfig.h"
#include "Hooking.Stubs.h"

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

static hook::cdecl_stub<hook::FlexStruct* (void*, uint16_t, bool)> getNetworkObject([]()
{
	return hook::get_pattern("66 89 54 24 ? 56", -0xA);
});

static void** g_objectMgr;
static uint32_t attachmentObjectIdOffset;

static bool (*g_origCanProcessPendingAttachment)(hook::FlexStruct*, void*, int*);
static bool CanProcessPendingAttachment(hook::FlexStruct* self, void* currentAttachmentEntity, int* failReason)
{
	if (IsClientConfigEnabled(ClientConfigFlag::DisableRemoteAttachments))
	{
		// Get the entity that the attachment will be attached to
		hook::FlexStruct* targetEntity = getNetworkObject(g_objectMgr, self->At<uint16_t>(attachmentObjectIdOffset), false);
		if (targetEntity != nullptr)
		{
			int8_t targetEntityOwner = targetEntity->At<int8_t>(0x45);
			int8_t ownerId = self->At<int8_t>(0x45);

			if (targetEntityOwner != ownerId)
			{
				// Clear the attachment object ID to prevent reattachment when the object owner changes
				self->Set<uint16_t>(attachmentObjectIdOffset, 0);
				
				*failReason = 2;
				return false;
			}
		}
	}
	
	return g_origCanProcessPendingAttachment(self, currentAttachmentEntity, failReason);
}

static HookFunction hookFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_MATRIX", [](fx::ScriptContext& context)
	{
		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		if (!entity)
		{
			return;
		}

		const float forwardX = context.GetArgument<float>(1);
		const float forwardY = context.GetArgument<float>(2);
		const float forwardZ = context.GetArgument<float>(3);
		const float rightX = context.GetArgument<float>(4);
		const float rightY = context.GetArgument<float>(5);
		const float rightZ = context.GetArgument<float>(6);
		const float upX = context.GetArgument<float>(7);
		const float upY = context.GetArgument<float>(8);
		const float upZ = context.GetArgument<float>(9);
		const float atX = context.GetArgument<float>(10);
		const float atY = context.GetArgument<float>(11);
		const float atZ = context.GetArgument<float>(12);

		DirectX::XMMATRIX matrix(
			DirectX::XMVectorSet(rightX, rightY, rightZ, 0.0f),
			DirectX::XMVectorSet(forwardX, forwardY, forwardZ, 0.0f),
			DirectX::XMVectorSet(upX, upY, upZ, 0.0f),
			DirectX::XMVectorSet(atX, atY, atZ, 1.0f)
		);

		entity->SetMatrix(matrix, true);
	});

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

	// Allow to disable remote attachments
	g_origCanProcessPendingAttachment = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 8A D8 84 C0 0F 84 ? ? ? ? 48 85 ED")), CanProcessPendingAttachment);
	g_objectMgr = hook::get_address<void**>(hook::get_pattern("45 0F 57 C0 48 8B 35 ? ? ? ? 0F 57 FF", 0x7));
	attachmentObjectIdOffset = *hook::get_pattern<uint32_t>("0F B7 96 ? ? ? ? 66 85 D2 0F 84", 0x3);
});
