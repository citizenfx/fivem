#include <StdInc.h>

#include <Pool.h>
#include "Hooking.h"
#include "Hooking.Stubs.h"
#include <Error.h>

// 
// When the game encounters some errors, it fatally errors with a vague rage error (e.g. 0x496AC5DF:961, 0x9952DB5E:212).
// This provides the server owner/end user no actionable information on the next steps to resolve the crash. This patch resolves this by removing some of these vague errors.
// Instead replacing them with an more descriptive error message that provides actionable information.
//
static std::unordered_map<void*, std::string> g_entityComponentstorage;

//
// These pools are unique in how they behave, how they are initalized and the layout of their struct.
// They can't be trivially fetched and most lack identifiable information (such as pool name/hash).
// So we manually grab any of these pools that tries to fetch its poolSize through gameconfig.xml
//
static std::unordered_map<void*, std::string> g_specialArchetypePools;

static void*(*g_initPool)(void*, int64_t);

template<uint32_t poolHash>
void* InitPool(void* self, int64_t poolSize)
{
	g_initPool(self, poolSize);
	g_specialArchetypePools.insert({ self, rage::GetPoolName(poolHash) });

	return self;
}

static void OnEntityComponentRegisterFail(void* componentStoragePtr)
{
	auto poolData = g_entityComponentstorage.find(componentStoragePtr);
	if (poolData != g_entityComponentstorage.end())
	{
		auto pool = rage::GetPoolBase(poolData->second.c_str());
		
		if (!pool)
		{
			FatalError("Failed to create '%s' component during entity creation.\n", poolData->second);
			return;
		}

		FatalError("Failed to create '%s' component during creating entity.\nCurrent Pool Size: %i/%i", poolData->second, pool->GetCount(), pool->GetSize());
	}

	FatalError("Failed to create an unknown component during entity creation");
}
	
static void OnArchetypePoolFull(hook::FlexStruct* pool)
{
	std::string poolName = "<<unknown pool>>";
	auto poolData = g_specialArchetypePools.find(pool);
	if (poolData != g_specialArchetypePools.end())
	{
		poolName = poolData->second;
	}

	AddCrashometry("pool_error", "%s (%d)", poolName, pool->At<int32_t>(0x10));
	FatalErrorNoExcept("%s Pool Full, Size == %d", poolName, pool->At<int32_t>(0x10));
}

static HookFunction hookFunction([]()
{
	// Due to the structure of entity component storages. Its not trivial to automatically fetch the associated pool names for each components.
	{
		g_entityComponentstorage = {
			// Ped Components
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 0F BA B6", 3)),
				"CPedPlayerComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8D 0D ? ? ? ? 48 8D 05 ? ? ? ? 48 89 0D", 3)),
				"CPedBreatheComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? FF 50 ? 48 8B 05 ? ? ? ? BA ? ? ? ? 48 8B CB FF 50 ? 48 8B 05 ? ? ? ? BA", 3)),
				"CPedAnimalComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 8B 06 41 23 C6", 3)),
				"CPedClothComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 1D ? ? ? ? 48 8D 05 ? ? ? ? 48 89 1D ? ? ? ? 48 89 05 ? ? ? ? 48 8D 35", 3)),
				"CPedAnimalAudioComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 1D ? ? ? ? 48 89 05 ? ? ? ? 48 8D 35", 3)), 
				"CPedCoreComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? BA ? ? ? ? 48 8B CE", 3)),
				"CPedAnimalEarsComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 8B D3 FF 50 ? 48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? 8B D6", 3)),
				"CPedCreatureComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? BE ? ? ? ? 8B D6 48 8B CD", 3)),
				"CPedAnimalTailComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 40 F6 C7 ? 74 ? 8B 93 ? ? ? ? 8B C2 41 23 C7", 3)),
				"CPedDistractionComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 8D 5E"), 3, 7),
				"CPedAnimationComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 8B EF", 3)),
				"CPedDrivingComponent" 
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 8B D6 49 8B CF", 3)),
				"CPedAttributeComponent"
			},
			{ 
				hook::get_address<void*>((intptr_t)hook::get_call(hook::get_pattern("E9 ? ? ? ? 33 C0 C3 CC BD")) + 0x7A, 3, 7),
				"CPedDummyComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 8B D6 49 8B CC", 3)),
				"CPedAudioComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 0F BA E7 ? 73 ? 8B 93 ? ? ? ? 8B C2 41 23 C7", 3)),
				"CPedEventComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 8B D6 49 8B CD", 3)),
				"CPedAvoidanceComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 ? C3 90 40 53 48 83 EC ? 8B 81", 3)),
				"CPedFacialComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 8B 93 ? ? ? ? 4C 8B 15", 3)),
				"CPedFootstepComponent"
			},
			{ 
				hook::get_address<void*>((intptr_t)hook::get_call(hook::get_pattern("E8 ? ? ? ? 4C 8B C7 4C 89 7D ? 48 8D 55 ? 48 8B CE E8 ? ? ? ? 8B 06")) + 0x79, 3, 7),
				"CPedGameplayComponent",
			},
			{ 
				hook::get_address<void*>((intptr_t)hook::get_call((intptr_t)hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B D3 48 8B CF E8 ? ? ? ? 48 8B 0D")) + 0x19) + 0x79, 3, 7),
				"CPedHealthComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 ? C3 CC 48 83 EC ? 8B 81", 3)),
				"CPedHorseComponent" 
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 7C 24 ? 8B F0 85 F6 75 ? BA ? ? ? ? 41 B8 ? ? ? ? 83 C9 ? E8 ? ? ? ? 83 FE ? 48 8B 74 24 ? 48 0F 44 DF 48 8B C3 48 8B 5C 24 ? 48 83 C4 ? 5F C3 CC CC CC", 3)),
				"CPedHumanAudioComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 4D 8B 03 FF 50 ? B8 ? ? ? ? 48 83 C4 ? C3 CC B7", 3)),
				"CPedIntelligenceComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 ? 5B C3 48 89 5C 24 ? 57 48 83 EC ? 48 8B 41", 3)),
				"CPedInventoryComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 ? C3 CC 48 83 EC ? 8B 91", 3)),
				"CPedLookAtComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 8B EF", 3)),
				"CPedMotionComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 85 F6 0F 84 ? ? ? ? 48 8B CB", 3)),
				"CPedMotivationComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? 48 83 C4 ? 5B E9 ? ? ? ? 90 48 83 EC ? 8B 81", 3)),
				"CPedPhysicsComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 4C 8B C7 E8 ? ? ? ? 8B D8 85 DB 75 ? BA ? ? ? ? 41 B8 ? ? ? ? 83 C9 ? E8 ? ? ? ? 83 FB ? 48 8B 5C 24 ? 0F 94 C0 48 83 C4 ? 5F C3 CC AE", 3)),
				"CPedProjDecalComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 0F BA B6", 3)),
				"CPedPlayerComponent"
			},
			{ 
				hook::get_address<void*>((intptr_t)hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B D8 48 8B 46 ? 48 83 E0")) + 0x125, 3, 7),
				"CPedRagdollComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 7C 24 ? 8B F0 85 F6 75 ? BA ? ? ? ? 41 B8 ? ? ? ? 83 C9 ? E8 ? ? ? ? 83 FE ? 48 8B 74 24 ? 48 0F 44 DF 48 8B C3 48 8B 5C 24 ? 48 83 C4 ? 5F C3 CC B8", 3)),
				"CPedScriptDataComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E9 ? ? ? ? C6 41 ? ? C3 90 48 89 5C 24", 3)),
				"CPedStaminaComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E9 ? ? ? ? C6 41 ? ? C3 90 8B 82", 3)),
				"CPedTargettingComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 ? C3 90 48 83 EC ? 8B 81", 3)),
				"CPedThreatResponseComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 C4 ? C3 90 EB ? 83 EC ? 80 3D", 3)),
				"CPedTransportComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? EB ? 48 8D B7 ? ? ? ? 41 BE", 3)),
				"CPedTransportUserComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 0F BA E7 ? 73 ? 48 8B CB E8 ? ? ? ? 40 F6 C7", 3)),
				"CPedVfxComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B D3 48 8D 0D ? ? ? ? E8 ? ? ? ? 33 ED", 3)),
				"CPedVisibilityComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? BA", 3)),
				"CPedWeaponComponent"
			},
			// Vehicle Components
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 8B ? ? ? ? E8 ? ? ? ? 48 8D 8B ? ? ? ? E8 ? ? ? ? 48 8B CF", 3)),
				"CVehiclePhysicsComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? FF 50 ? 48 8B 05 ? ? ? ? BA ? ? ? ? 48 8B CB FF 50 ? 48 8B 05 ? ? ? ? BB", 3)),
				"CVehiclePhysicsComponent" // CArtilleryGunPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? BA ? ? ? ? 48 8B CB FF 50 ? 48 8B 05 ? ? ? ? BB", 3)),
				"CVehiclePhysicsComponent" // CAutogyroPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 49 83 C8", 3)),
				"CVehiclePhysicsComponent" // CHeliPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? BB ? ? ? ? 8B D3 48 8B CF", 3)),
				"CVehiclePhysicsComponent" // CAutomobilePhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 8B D3 48 8B CE", 3)),
				"CVehiclePhysicsComponent" // CBalloonPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 8B D3 48 8B CD FF 50 ? 8B D3", 3)),
				"CVehiclePhysicsComponent" // CBikePhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 8D 8B", 3)),
				"CVehiclePhysicsComponent" // CPlanePhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 49 8B CE FF 50 ? 48 8B 05", 3)),
				"CVehiclePhysicsComponent" // CBlimpPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_call((uintptr_t)hook::get_pattern("E8 ? ? ? ? 83 ? ? ? 00 00 04 48 8D 05 ? ? ? ? 48 89 03 75", 34)) + 0x7A, 3, 7),
				"CVehiclePhysicsComponent" // CQuadBikePhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_call((uintptr_t)hook::get_pattern("E8 ? ? ? ? 83 ? ? ? 00 00 0D 48 8D 05 ? ? ? ? 48 89 03 75", 34)) + 0x7A, 3, 7),
				"CVehiclePhysicsComponent" // CBmxPhysicsComponent, uses the same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 80 BB ? ? ? ? ? 74 ? 8B 83", 3)),
				"CVehiclePhysicsComponent" // CBoatPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_call((uintptr_t)hook::get_pattern("E8 ? ? ? ? 83 ? ? ? 00 00 10 48 8D 05 ? ? ? ? 48 89 03 75", 34)) + 0x7D, 3, 7),
				"CVehiclePhysicsComponent" // CRowingBoatPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_call((uintptr_t)hook::get_pattern("E8 ? ? ? ? 83 ? ? ? 00 00 0F 48 8D 05 ? ? ? ? 48 89 03 75", 34)) + 0x7A, 3, 7),
				"CVehiclePhysicsComponent" // CCanoePhysicsComponent, uses the same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 8B ? ? ? ? E8 ? ? ? ? 48 8B CB", 3)),
				"CVehiclePhysicsComponent" // CSubmarineCarPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? BF ? ? ? ? 48 8D B3", 3)),
				"CVehiclePhysicsComponent" // CSubmarinePhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 8B CB 48 89 83", 3)),
				"CVehiclePhysicsComponent" // CTrailerPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 8B D3 FF 50 ? 48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? 8B D3 FF 50 ? 48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? 8B D3 FF 50 ? 48 8B 05 ? ? ? ? 8D 53", 3)),
				"CVehiclePhysicsComponent" // CTrainCartPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D AB", 3)),
				"CVehiclePhysicsComponent" // CTrainCartPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 57 ? 48 8D 4C 24", 3)),
				"CVehiclePhysicsComponent" // CTrainPhysicsComponent, uses same pool as CVehiclePhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? FF 50 ? 8B D3", 3)),
				"CVehicleAnimationComponent"
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? FF 50 ? 48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? 8B D3", 3)),
				"CVehicleCoreComponent"
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? BD ? ? ? ? 39 AB", 3)),
				"CVehicleDrivingComponent"
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 8B 93 ? ? ? ? 4C 8B 05", 3)),
				"CVehicleIntelligenceComponent"
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 89 05 ? ? ? ? 48 8B 05 ? ? ? ? FF 50", 3)),
				"CVehicleWeaponsComponent"
			},

			// Object Components
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8B 8B ? ? ? ? 48 85 C9 74 ? 48 8B 01 41 8B D6", 3)),
				"CObjectIntelligenceComponent"
			},
			{  
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 4C 8D 25 ? ? ? ? 4C 8D 2D", 3)),
				"CObjectPhysicsComponent" // CDoorPhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 8B D3 FF 50 ? 48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? 8B D3 FF 50 ? 8B D3", 3)),
				"CObjectNetworkComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? BA ? ? ? ? 48 8B CB FF 50 ? 48 8B 05 ? ? ? ? BA ? ? ? ? 48 8B CF", 3)),
				"CObjectPhysicsComponent" // CDraftVehicleWheelObjectPhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 8B D3 FF 50 ? 8B D3", 3)),
				"CObjectPhysicsComponent"
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? BA ? ? ? ? 48 8B CF FF 50 ? 48 8B 05", 3)),
				"CObjectAnimationComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? FF 50 ? 48 8B 05 ? ? ? ? 48 8D 0D ? ? ? ? 8B D3", 3)),
				"CObjectRiverProbeSubmissionComponent"
			}, 
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? BB ? ? ? ? 8B D3 48 8B CE", 3)),
				"CObjectAutoStartAnimComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 0F BA B3 ? ? ? ? ? 41 BE", 3)),
				"CObjectWeaponsComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 8B D3 48 8B CD FF 50 ? 48 8B 05", 3)),
				"CObjectBreakableGlassComponent"
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? 8B D3 FF 50 ? 8B D3", 3)),
				"CObjectPhysicsComponent" // CPickupPhysicsComponent
			},
			{
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? F6 83 ? ? ? ? ? 0F 85", 3)),
				"CObjectBuoyancyModeComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 8D BB ? ? ? ? 48 8B CF", 3)),
				"CObjectPhysicsComponent" // CProjectilePhysicsComponent
			},
			{ 
				hook::get_address<void*>(hook::get_pattern("4C 8D 3D ? ? ? ? 48 8B 05 ? ? ? ? 4C 8D 25", 3)),
				"CObjectCollisionDetectedComponent"
			},
			{ 
				hook::get_address<void*>(hook::get_pattern(" 4C 8D 25 ? ? ? ? 4C 8D 2D ? ? ? ? 4C 89 3D", 3)),
				"CObjectDoorComponent"
			}
		};


		// 0x496AC5DF, 0x3C1, 0F
		auto patterns = hook::pattern("75 ? ? DF C5 6A 49 ? B8 C1 03 00 00 ? C9 FF");
		int count = 0;
		for (size_t i = 0; i < patterns.size(); i++)
		{
			auto pattern = patterns.get(i).get<void>(2);
			hook::nop(pattern, 19);

			auto patchCall = [&](uintptr_t address, intptr_t offset)
			{
				uint8_t* leaInstr = (uint8_t*)(address - offset);
				// 48 8D 0D = lea rcx, [rip + disp32]
				if (leaInstr[0] == 0x48 && leaInstr[1] == 0x8D && leaInstr[2] == 0x0D)
				{
					uintptr_t target = ((uintptr_t)leaInstr + 7) + *(int32_t*)(leaInstr + 3);
					intptr_t newDisp = (intptr_t)target - (address + 7);
					assert(newDisp >= INT32_MIN && newDisp <= INT32_MAX);

					// lea rcx, [rip + newDisp]
					hook::put<uint8_t>(address + 0, 0x48);
					hook::put<uint8_t>(address + 1, 0x8D);
					hook::put<uint8_t>(address + 2, 0x0D);
					hook::put<int32_t>(address + 3, newDisp);
					count++;
				}
			};

			// All variations of the function calling 0x496AC5DF:961 are one of the following
			// 1:
			// 48 8D 0D ? ? ? ? lea     rcx, qword_*
			// E8 ?  ?  ?  ?    call    sub_*
			// 48 8B 7C 24 30   mov     rdi, [rsp+30h]
			// 8B F0            mov     esi, eax
			// 2:
			// 48 8D 0D ? ? ? ? lea     rcx, qword_*
			// 4C 8B C7         mov     r8, rdi
			// E8 ?  ?  ?  ?    call    sub_*
			// 8B D8            mov     ebx, eax

			patchCall((uintptr_t)pattern, 23);
			patchCall((uintptr_t)pattern, 21);
			hook::call((uintptr_t)pattern + 7, OnEntityComponentRegisterFail);
		}

#ifdef _DEBUG
		// In case future updates breaks the functionality above.
		assert(count == patterns.size());
#endif
	}

	// Replace 0x9952DB5E:212 error code with the pool that ran out.
	{
		auto location = hook::get_pattern("BA ? ? ? ? 41 B8 ? ? ? ? 83 C9 ? E8 ? ? ? ? 48 3B 7B");
		hook::nop(location, 19);
		// mov rcx, rbx
		hook::put<uint32_t>(location, 0xCB8B48);
		hook::call((uintptr_t)location + 3, OnArchetypePoolFull);
	}

	// RDR3 has some pools that are handled uniquely, most of which lack identifiable information (such as a pool hash/name)
	// However, for the few that do provide atleast a pool hash, we want to make this available for providing some crashes with more information.
	{		
		void* fwDynamicArchetypeComponent = hook::get_pattern("48 8B 0D ? ? ? ? BA 2B 20 10 E0 41 B8 ? ? ? ? E8", 72);
		hook::set_call(&g_initPool, fwDynamicArchetypeComponent);
		hook::call(fwDynamicArchetypeComponent, &InitPool<HashString("fwDynamicArchetypeComponent")>);

		void* knownRefs = hook::get_pattern("48 8B 0D ? ? ? ? BA 32 01 D8 D8 41 B8 ? ? ? ? E8", 72);
		hook::call(knownRefs, InitPool<HashString("known refs")>);

		void* maxVisibleClothCount = hook::get_pattern("E8 ? ? ? ? D1 EF");
		hook::call(maxVisibleClothCount, InitPool<HashString("MaxVisibleClothCount")>);
	}
});
