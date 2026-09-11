#include <StdInc.h>
#include <CoreConsole.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>

#include <rageVectors.h>

namespace rage
{
	struct alignas(16) Vec3V : Vector3 { };
}

class CPedBlenderData
{
public:
	char m_Padding[0x14];
	uint32_t m_BlendVelSmoothTime;

	char m_Padding2[0x7C];
	float m_MaxVelDiffFromTarget;
};

class CNetBlenderPed : public hook::FlexStruct
{
public:
	char m_Padding[0x70];
	rage::Vector3 m_Velocity;

	char m_Padding2[0x15];
	bool m_IsRamping;

	inline CPedBlenderData* GetBlenderData()
	{
		return CallVirtual<CPedBlenderData*>(0x18);
	}

	inline void GetCurrentPredictedPosition(rage::Vec3V& result)
	{
		CallVirtual<void>(0x88, &result);
	}

	inline void GetPositionFromObject(rage::Vec3V& result)
	{
		CallVirtual<void>(0xE0, &result);
	}

	inline void GetVelocityFromObject(rage::Vec3V& result)
	{
		CallVirtual<void>(0xE8, &result);
	}

	inline void SetVelocityOnObject(const rage::Vec3V& velocity)
	{
		CallVirtual<void>(0x108, &velocity);
	}
};

static bool g_UsePedNetworkVelocity = true;

static void (*g_CNetBlenderPed_ProcessPrePhysics)(CNetBlenderPed*);

static void CNetBlenderPed_ProcessPrePhysics(CNetBlenderPed* thisPtr)
{
	if (!g_UsePedNetworkVelocity)
		return g_CNetBlenderPed_ProcessPrePhysics(thisPtr);

	thisPtr->m_IsRamping = false;

	g_CNetBlenderPed_ProcessPrePhysics(thisPtr);

	if (!thisPtr->m_IsRamping)
		return;

	CPedBlenderData* blenderData = thisPtr->GetBlenderData();

	const float maxSpeedDiff = blenderData->m_MaxVelDiffFromTarget;
	const float timeToBlendOver = blenderData->m_BlendVelSmoothTime / 1000.0f;

	rage::Vec3V predictedCurrentPosition, objectPosition, currentVelocity;
	thisPtr->GetCurrentPredictedPosition(predictedCurrentPosition);
	thisPtr->GetPositionFromObject(objectPosition);
	thisPtr->GetVelocityFromObject(currentVelocity);

	const rage::Vector3& targetVelocity = thisPtr->m_Velocity;
	rage::Vector3 positionDelta = predictedCurrentPosition - objectPosition;

	rage::Vector3 correctionVector = positionDelta * (1.0f / timeToBlendOver);
	correctionVector.z = 0.0f;

	const float correctionLength = correctionVector.Length();

	if (correctionLength > maxSpeedDiff)
		correctionVector *= maxSpeedDiff / correctionLength;

	rage::Vec3V newVelocity = { targetVelocity + correctionVector };
	newVelocity.z = currentVelocity.z;

	thisPtr->SetVelocityOnObject(newVelocity);
}

static HookFunction hookFunction([]
{
	static ConVar<bool> usePedNetworkVelocity("game_usePedNetworkVelocity", ConVar_Replicated, false, &g_UsePedNetworkVelocity);

	g_CNetBlenderPed_ProcessPrePhysics = hook::trampoline(
		hook::get_pattern("48 8B C4 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 0F 29 70 ? 0F 29 78 ? 44 0F 29 40 ? 48 8B F9 48 8B 89 10 01 00 00"), CNetBlenderPed_ProcessPrePhysics);
});
