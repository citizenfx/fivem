#include "StdInc.h"

#include <ICoreGameInit.h>
#include <GameInit.h>
#include <CoreConsole.h>

#include <Hooking.h>

#include <ScriptEngine.h>
#include <scrEngine.h>

#include <CrossBuildRuntime.h>
#include <EntitySystem.h>
#include <DirectXMath.h>

#include "GameValueStub.h"

static GameValueStub<float> g_vfxNitrousOverride;
static bool g_scriptExhaustBones;

namespace rage
{
	using Mat34V = DirectX::XMMATRIX;
	using Vec4V = DirectX::XMVECTOR;
}

static hook::cdecl_stub<void(CVehicle*, int32_t, rage::Mat34V*, int32_t*)> _getExhaustMatrix([]()
{
	return hook::get_pattern("4D 8B E1 49 8B F0 48 8B D9 44 0F 29 48", -0x33);
});

static hook::cdecl_stub<bool(CVehicle*, rage::Mat34V*, int32_t)> _getModdedExhaustMatrixBone([]()
{
	return hook::get_pattern("48 83 EC 28 41 8B C0 41 83 F8 FF 75 04");
});

static hook::cdecl_stub<bool(CVehicle*, int, rage::Vec4V*, rage::Vec4V*)> _getLocalBonePosition([]()
{
	return hook::get_pattern("48 8D 50 98 48 8D 48 A8 0F 29 70 E8 C6 40 88 00", -0x1B);
});

// b3095 updated GET_ENTITY_BONE_POSITION with a specific carve-out for modified
// exhaust bone matrices (0x140A7E4E6/b3095).
static void UpdateGetEntityBonePosition()
{
	constexpr uint64_t GET_ENTITY_BONE_POSITION = 0x46F8696933A63C9B;
	const auto originalHandler = fx::ScriptEngine::GetNativeHandler(GET_ENTITY_BONE_POSITION);
	if (!originalHandler)
	{
		return;
	}

	const auto handler = *originalHandler;
	fx::ScriptEngine::RegisterNativeHandler(GET_ENTITY_BONE_POSITION, [handler](fx::ScriptContext& context)
	{
		if (!g_scriptExhaustBones)
		{
			handler(context);
			return;
		}

		uint32_t entityIndex = context.GetArgument<uint32_t>(0);
		int32_t boneIndex = context.GetArgument<int32_t>(1);

		auto entity = rage::fwScriptGuid::GetBaseFromGuid(entityIndex);
		if (entity && entity->IsOfType<CVehicle>())
		{
			rage::Mat34V exhaustMat = DirectX::XMMatrixIdentity();
			if (_getModdedExhaustMatrixBone(reinterpret_cast<CVehicle*>(entity), &exhaustMat, boneIndex))
			{
				scrVector result = {};
				result.x = DirectX::XMVectorGetX(exhaustMat.r[3]);
				result.y = DirectX::XMVectorGetY(exhaustMat.r[3]);
				result.z = DirectX::XMVectorGetZ(exhaustMat.r[3]);
				context.SetResult<scrVector>(result);
				return;
			}
		}

		handler(context);
	});
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern("0F 28 82 ? ? ? ? 48 8D 4D C7 0F 29 45 C7", 0x14 + 0x3);
		g_vfxNitrousOverride.Init(*hook::get_address<float*>(location));
		g_vfxNitrousOverride.SetLocation(location);
	}

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_vfxNitrousOverride.Reset();
	});
});

static InitFunction initFunction([]()
{
	// GH-2003: Increase the maximum range for displaying veh_nitrous PTFX
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_NITRO_PTFX_RANGE", [](fx::ScriptContext& context)
	{
		constexpr float kMaxPtfxSqrDist = 212.0f * 212.0f; // Half of OneSync maxRange

		float dist = context.GetArgument<float>(0);
		float sqrDist = dist * dist;
		if (std::isfinite(sqrDist))
		{
			float minSqrDist = g_vfxNitrousOverride.GetDefault();
			g_vfxNitrousOverride.Set(std::clamp(sqrDist, minSqrDist, kMaxPtfxSqrDist));
		}
	});

	// GH-2003: Backport b3095 vehicle exhaust bone getters to earlier builds
	if (!xbr::IsGameBuildOrGreater<3095>())
	{
		static constexpr int32_t kVehicleBoneLimit = 32; // b3095
		static constexpr int32_t kExhaustModOffset = 56; // Never changed
		static ConVar<bool> enableExhaustBones("game_enableScriptExhaustBones", ConVar_Replicated, false, &g_scriptExhaustBones);

		OnKillNetworkDone.Connect([]()
		{
			se::ScopedPrincipal principalScopeInternal(se::Principal{ "system.internal" });
			enableExhaustBones.GetHelper()->SetValue("false");
		});

		rage::scrEngine::OnScriptInit.Connect([]()
		{
			UpdateGetEntityBonePosition();
		});

		fx::ScriptEngine::RegisterNativeHandler(0x3EE18B00CD86C54F, [](fx::ScriptContext& context)
		{
			context.SetResult<int32_t>(kVehicleBoneLimit);
		});

		fx::ScriptEngine::RegisterNativeHandler(0xE728F090D538CB18, [](fx::ScriptContext& context)
		{
			auto entityIndex = context.GetArgument<uint32_t>(0);
			auto exhaustIndex = context.GetArgument<int32_t>(1);
			auto outBone = context.CheckArgument<int32_t*>(2);
			auto outInvAxis = context.CheckArgument<bool*>(3);

			bool result = false;
			auto entity = rage::fwScriptGuid::GetBaseFromGuid(entityIndex);
			if (entity && entity->IsOfType<CVehicle>() && exhaustIndex >= 0 && exhaustIndex < kVehicleBoneLimit)
			{
				CVehicle* vehicle = reinterpret_cast<CVehicle*>(entity);
				rage::Mat34V exhaustMat = DirectX::XMMatrixIdentity();

				// GET_VEHICLE_EXHAUST_BONE translates the bone matrix back into
				// local space and checks the sign of the X.
				_getExhaustMatrix(vehicle, exhaustIndex + kExhaustModOffset, &exhaustMat, outBone);
				if (!DirectX::XMVector4Equal(exhaustMat.r[0], DirectX::XMVectorZero()))
				{
					rage::Vec4V exhaustPos = exhaustMat.r[3];
					rage::Vec4V localPos = DirectX::XMVectorZero();
					if (_getLocalBonePosition(vehicle, -1, &exhaustPos, &localPos))
					{
						result = true;
						*outInvAxis = DirectX::XMVectorGetX(localPos) > 0.0f;
					}
				}
			}

			context.SetResult<bool>(result);
		});
	}
});
