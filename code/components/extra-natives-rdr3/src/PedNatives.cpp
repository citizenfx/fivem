#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>
#include <EntitySystem.h>
#include <DirectXMath.h>
#include <scrEngine.h>

using Matrix4x4 = DirectX::XMFLOAT4X4;

static hook::cdecl_stub<void(void*, uint32_t, Matrix4x4* readMatrix, char)> GetPedBoneTransform([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 0F C2 F7 00 0F 50 C6 83 E0"));
});

static hook::cdecl_stub<void*(rage::fwEntity*)> GetPedCreatureComponent([]()
{
	return hook::get_call(hook::get_pattern("D1 0F 14 FA E8 ? ? ? ? 48 8B C8 4C 8D 44", 4));
});

static hook::cdecl_stub<float(uintptr_t)> GetPedScale([]()
{
	return hook::get_pattern("0F 57 C0 0F 2F 81 ? ? ? ? 0F 82 ? ? ? ? 8B 81");
});

static hook::cdecl_stub<bool(rage::fwEntity*)> IsMetaPed([]()
{
	return hook::get_pattern("40 53 48 83 EC ? 8B 15 ? ? ? ? 48 8B D9 48 83 C1 ? E8 ? ? ? ? 33 D2");
});

struct PedComponentsContainer
{
	void* m_animal;
	void* m_animalAudio;
	void* m_animalEars;
	void* m_animalTail;
	void* m_animation;
	void* m_attribute;
	void* m_audio;
	void* m_avoidance;
	void* m_breathe;
	void* m_cloth;
	void* m_core;
	void* m_creature;
	void* m_distraction;
	void* m_driving;
	void* m_dummy;
	void* m_event;
	void* m_facial;
	void* m_footstep;
	void* m_gameplay;
	void* m_graphics;
	void* m_health;
	void* m_horse;
	void* m_humanAudio;
	void* m_intelligence;
	void* m_inventory;
	void* m_lookAt;
	void* m_motion;
	void* m_motivation;
	void* m_physics;
	void* m_projDecal;
	void* m_player;
	void* m_ragdoll;
	void* m_scriptData;
	void* m_stamina;
	void* m_targetting;
	void* m_threatResponse;
	void* m_transport;
	void* m_transportUser;
	void* m_vfxComponent;
	void* m_visibility;
	void* m_weapon;
};

static HookFunction hookFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_PED_BONE_MATRIX", [](fx::ScriptContext& context)
	{
		rage::fwEntity* ped = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
		uint32_t boneId = context.GetArgument<int>(1);

		if (ped && boneId)
		{

			void* g_pedCreatureComponent = GetPedCreatureComponent(ped);

			if (g_pedCreatureComponent)
			{

				Matrix4x4 readMatrix;
				GetPedBoneTransform(g_pedCreatureComponent, boneId, &readMatrix, (char)0);

				scrVector* forwardVector = context.GetArgument<scrVector*>(2);
				scrVector* rightVector = context.GetArgument<scrVector*>(3);
				scrVector* upVector = context.GetArgument<scrVector*>(4);
				scrVector* atVector = context.GetArgument<scrVector*>(5);

				auto copyVector = [](const float* in, scrVector* out)
				{
					out->x = in[0];
					out->y = in[1];
					out->z = in[2];
				};

				copyVector(readMatrix.m[0], forwardVector);
				copyVector(readMatrix.m[1], rightVector);
				copyVector(readMatrix.m[2], upVector);
				copyVector(readMatrix.m[3], atVector);
			}
		}
	});

	{
		static PedComponentsContainer** g_pedComponentContainer = hook::get_address<PedComponentsContainer**>(hook::get_pattern("48 8B 05 ? ? ? ? 4C 8B 44 01 ? 48 8B 94 01", 3));

		static int32_t* g_pedComponentIndexOffset = hook::get_address<int32_t*>(hook::get_pattern("2B 0D ? ? ? ? 84 C0 8B C1 74 ? 48 69 C8 ? ? ? ? 0F 28 CE", 2));


		fx::ScriptEngine::RegisterNativeHandler("GET_PED_SCALE", [](fx::ScriptContext& context)
		{
			rage::fwEntity* ped = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0));
			if (!ped)
			{
				context.SetResult<float>(0.0f);
				return;
			}

			uint32_t entityComponents = *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(ped) + 0x9C);
			int32_t componentIndex = (entityComponents & 0x1FFFF) - *g_pedComponentIndexOffset;

			if (!g_pedComponentContainer || componentIndex < 0)
			{
				context.SetResult<float>(0.0f);
				return;
			}

			auto& pedComponent = (*g_pedComponentContainer)[componentIndex];

			float scale = 0.0f;
			if (!IsMetaPed(ped))
			{
				uintptr_t corePtr = reinterpret_cast<uintptr_t>(pedComponent.m_core);
				corePtr = (corePtr != 0) ? (corePtr & 0xFFFFFFFFFFFFFFFE) : 0;
				if (corePtr)
				{
					scale = *reinterpret_cast<float*>(corePtr + 0x48);
				}
			}
			else
			{
				uintptr_t physicsPtr = reinterpret_cast<uintptr_t>(pedComponent.m_physics);
				physicsPtr = (physicsPtr != 0) ? (physicsPtr & 0xFFFFFFFFFFFFFFFE) : 0;
				if (physicsPtr)
				{
					scale = GetPedScale(physicsPtr);
				}
			}

			context.SetResult<float>(scale);
		});
	}
	
});
