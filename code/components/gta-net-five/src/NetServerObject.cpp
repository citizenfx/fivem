/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>
#include <MinHook.h>
#include <netObject.h>
#include <ICoreGameInit.h>
#include <NetLibrary.h>
#include <EntitySystem.h>
#include <CoreConsole.h>

static ICoreGameInit* icgi;

static bool g_protectServerEntity = false;

namespace sync
{
	extern std::unordered_map<int, uint32_t> g_objectIdToCreationToken;
}

static bool CanEntityBeDeleted(rage::netObject* netObject)
{
	if (!g_protectServerEntity)
	{
		return true;
	}

	uint16_t creationToken = sync::g_objectIdToCreationToken[netObject->GetObjectId()];
	
	if (creationToken != 0)
	{
		return netObject->syncData.wantsToDelete && !netObject->syncData.shouldNotBeDeleted;
	}

	return true;
}

static void (*g_origCVehicleFactoryDestroy)(void*, CVehicle*, bool);
static void CVehicleFactory_Destroy(void* self, CVehicle* vehicle, bool unk)
{
	if (!vehicle)
	{
		return;
	}

	if (!icgi->OneSyncEnabled || !vehicle->GetNetObject() || CanEntityBeDeleted((rage::netObject*)vehicle->GetNetObject()))
	{
		g_origCVehicleFactoryDestroy(self, vehicle, unk);
	}
}

static bool (*g_origCPedFactoryDestroy)(void*, fwEntity*, bool);
static bool CPedFactory_Destroy(void* self, fwEntity* ped, bool unk)
{
	if (!ped)
	{
		return false;
	}

	if (!icgi->OneSyncEnabled || !ped->GetNetObject() || CanEntityBeDeleted((rage::netObject*)ped->GetNetObject()))
	{
		return g_origCPedFactoryDestroy(self, ped, unk);
	}

	return false;
}

static void (*g_destroyObject)(fwEntity*, int);
static void DestroyObject(fwEntity* object, int unk)
{
	if (!object)
	{
		return;
	}

	if (!icgi->OneSyncEnabled || !object->GetNetObject() || CanEntityBeDeleted((rage::netObject*)object->GetNetObject()))
	{
		g_destroyObject(object, unk);
	}
}

static HookFunction hookFunction([]()
{
	static ConVar<bool> protectServerEntities("sv_protectServerEntities", ConVar_Replicated, false, &g_protectServerEntity);

	// Block the client from being able to delete server-setter entities if "sv_protectServerEntities" is set. 
	// This prevents the deleted object from "flickering" as it gets instantly re-created by the server
	{
        MH_Initialize();
		MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? FF 05"), CVehicleFactory_Destroy, (void**)&g_origCVehicleFactoryDestroy);
		MH_CreateHook(hook::get_pattern("48 8B F1 48 85 D2 74 ? 48 8B 8A", -0x15), CPedFactory_Destroy, (void**)&g_origCPedFactoryDestroy);
		MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 85 FF 74 ? F7 87")), DestroyObject, (void**)&g_destroyObject);
		MH_EnableHook(MH_ALL_HOOKS);
	}
});

static InitFunction initFunctionSv([]()
{
	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
	   icgi = Instance<ICoreGameInit>::Get();
	});
});
