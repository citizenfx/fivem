#include <StdInc.h>
#include <CoreConsole.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>

#include <EntitySystem.h>
#include <netObject.h>
#include <rageVectors.h>

class CTaskAimGunOnFoot
{
public:
	char m_Padding[0x10];
	fwEntity* m_Ped;
	char m_Padding2[0x1B8];

	rage::Vector3 m_FiringStartOffset;
	rage::Vector3 m_FiringEndOffset;
	float m_FiringTargetDistance;
};

class CClonedAimGunOnFootInfo : public hook::FlexStruct
{
public:
	char m_Padding[0x40];

	rage::Vector3 m_FiringStartOffset;
	rage::Vector3 m_FiringEndOffset;
	float m_FiringTargetDistance;
};

class CSyncDataSerialiser : public hook::FlexStruct
{
public:
	inline void SerialisePackedFloat(float& value, float range, int bits)
	{
		CallVirtual<void>(0x80, &value, range, bits, nullptr);
	}

	inline void SerialiseVector(rage::Vector3& value, float range, int bits)
	{
		CallVirtual<void>(0xA8, &value, range, bits, nullptr);
	}
};

static bool g_AccurateBullet;

static CTaskAimGunOnFoot* (*g_CTaskAimGunOnFoot_Ctor)(CTaskAimGunOnFoot*, int, void*, void*, void*, void*, uint32_t);

static CTaskAimGunOnFoot* CTaskAimGunOnFoot_Ctor(CTaskAimGunOnFoot* task, int weaponControllerType, void* flags, void* fireFlags, void* target, void* ikInfo, uint32_t blockFiringTime)
{
	task = g_CTaskAimGunOnFoot_Ctor(task, weaponControllerType, flags, fireFlags, target, ikInfo, blockFiringTime);

	task->m_FiringStartOffset = {};
	task->m_FiringEndOffset = {};
	task->m_FiringTargetDistance = -1.0f;

	return task;
}

static bool (*g_CTaskAimGun_CalculateFiringVector)(CTaskAimGunOnFoot*, fwEntity*, rage::Vector3*, rage::Vector3*, void*, float*, bool);

static bool CTaskAimGun_CalculateFiringVector(CTaskAimGunOnFoot* task, fwEntity* ped, rage::Vector3* start, rage::Vector3* end, void* cameraEntity, float* targetDistance, bool aimFromCamera)
{
	bool result = g_CTaskAimGun_CalculateFiringVector(task, ped, start, end, cameraEntity, targetDistance, aimFromCamera);

	if (!g_AccurateBullet)
	{
		return result;
	}

	auto object = static_cast<rage::netObject*>(ped->GetNetObject());

	if (!object)
	{
		return result;
	}

	auto pedPos = ped->GetPosition();
	rage::Vector3 origin(pedPos.x, pedPos.y, pedPos.z);

	if (!object->IsClone())
	{
		task->m_FiringStartOffset = *start - origin;
		task->m_FiringEndOffset = *end - origin;
		task->m_FiringTargetDistance = targetDistance ? *targetDistance : -1.0f;
	}
	else
	{
		*start = task->m_FiringStartOffset + origin;
		*end = task->m_FiringEndOffset + origin;

		if (targetDistance)
		{
			*targetDistance = task->m_FiringTargetDistance;
		}
	}

	return result;
}

static CClonedAimGunOnFootInfo* (*g_CTaskAimGunOnFoot_CreateQueriableState)(CTaskAimGunOnFoot*);

static CClonedAimGunOnFootInfo* CTaskAimGunOnFoot_CreateQueriableState(CTaskAimGunOnFoot* task)
{
	auto info = g_CTaskAimGunOnFoot_CreateQueriableState(task);

	if (g_AccurateBullet)
	{
		info->m_FiringStartOffset = task->m_FiringStartOffset;
		info->m_FiringEndOffset = task->m_FiringEndOffset;
		info->m_FiringTargetDistance = task->m_FiringTargetDistance;
	}

	return info;
}

static void (*g_CClonedAimGunOnFootInfo_Serialise)(CClonedAimGunOnFootInfo*, CSyncDataSerialiser*);

static void CClonedAimGunOnFootInfo_Serialise(CClonedAimGunOnFootInfo* info, CSyncDataSerialiser* serialiser)
{
	g_CClonedAimGunOnFootInfo_Serialise(info, serialiser);

	if (g_AccurateBullet)
	{
		serialiser->SerialiseVector(info->m_FiringStartOffset, 8.0f, 12);

		// NOTE: the game itself uses 16000.0f (WORLDLIMITS_XMAX) in many places, so i think it should be fine here too
		serialiser->SerialiseVector(info->m_FiringEndOffset, 16000.0f, 32);
		serialiser->SerialisePackedFloat(info->m_FiringTargetDistance, 16000.0f, 32);
	}
}

static void (*g_CTaskAimGunOnFoot_ReadQueriableState)(CTaskAimGunOnFoot*, CClonedAimGunOnFootInfo*);

static void CTaskAimGunOnFoot_ReadQueriableState(CTaskAimGunOnFoot* task, CClonedAimGunOnFootInfo* info)
{
	g_CTaskAimGunOnFoot_ReadQueriableState(task, info);

	if (g_AccurateBullet)
	{
		task->m_FiringStartOffset = info->m_FiringStartOffset;
		task->m_FiringEndOffset = info->m_FiringEndOffset;
		task->m_FiringTargetDistance = info->m_FiringTargetDistance;
	}
}

static HookFunction hookFunction([]
{
	static ConVar<bool> accurateBullet("game_accurateBullet", ConVar_Replicated, false, &g_AccurateBullet);

	hook::put<uint8_t>(hook::get_pattern<uint8_t>("41 8D 51 ? 45 8D 41 ? E8 ? ? ? ? 4C 8B D0 48 85 C0 75 ? B9 ? ? ? ? E8 ? ? ? ? EB ? 48 8B 45", 3), sizeof(CClonedAimGunOnFootInfo));
	hook::put<uint8_t>(hook::get_pattern<uint8_t>("41 8D 51 ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B C8 E8 ? ? ? ? E9 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 C9 4D 8B C6 BA ? ? ? ? E8 ? ? ? ? 48 85 C0 0F 84 ? ? ? ? 48 8B C8 E8 ? ? ? ? E9 ? ? ? ? 48 8B 0D ? ? ? ? 45 33 C9 4D 8B C6 BA ? ? ? ? E8 ? ? ? ? 48 8B D8 48 85 C0 0F 84 ? ? ? ? 44 21 68 ? 44 09 78 ? 44 89 60", 3), sizeof(CClonedAimGunOnFootInfo));

	hook::put<uint32_t>(hook::get_pattern<uint8_t>("BA D0 01 00 00 41 B8 10 00 00 00 E8 ? ? ? ? 33 DB 4C 8B D0 48 85 C0 75 ? B9 84 EC F4 C6", 1), sizeof(CTaskAimGunOnFoot));
	hook::put<uint32_t>(hook::get_pattern<uint8_t>("BA D0 01 00 00 41 B8 10 00 00 00 E8 ? ? ? ? 33 DB 48 8B C8 48 85 C0 74 ? 8B 84 24 88 00 00 00", 1), sizeof(CTaskAimGunOnFoot));

	g_CTaskAimGunOnFoot_Ctor = hook::trampoline(
		hook::get_pattern("48 89 5C 24 ? 57 48 83 EC 30 48 8B 44 24 ? 48 8B D9 48 89 44 24 ? E8 ? ? ? ? 48 8D 05"), CTaskAimGunOnFoot_Ctor);

	g_CTaskAimGun_CalculateFiringVector = hook::trampoline(
		hook::get_pattern("48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 48 89 48 ? 55 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? B8 30 1A 00 00"), CTaskAimGun_CalculateFiringVector);

	g_CTaskAimGunOnFoot_CreateQueriableState = hook::trampoline(
		hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 57 48 83 EC 70 0F 29 70 ? 48 8B 41 ? 40 B6 01"), CTaskAimGunOnFoot_CreateQueriableState);

	g_CClonedAimGunOnFootInfo_Serialise = hook::trampoline(
		hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 30 48 8B FA 48 8B F1 E8 ? ? ? ? 48 8B 07 45 33 C9"), CClonedAimGunOnFootInfo_Serialise);

	g_CTaskAimGunOnFoot_ReadQueriableState = hook::trampoline(
		hook::get_pattern("48 89 5C 24 ? 57 48 83 EC 20 48 8B FA 48 8B D9 E8 ? ? ? ? 8A 83 ? ? ? ? B1 01"), CTaskAimGunOnFoot_ReadQueriableState);
});
