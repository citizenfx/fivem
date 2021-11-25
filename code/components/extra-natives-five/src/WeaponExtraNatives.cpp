#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>
#include <ICoreGameInit.h>
#include <rageVectors.h>
#include <MinHook.h>

static int WeaponDamageModifierOffset;

static uint16_t* g_weaponCount;
static uint64_t** g_weaponList;

enum flashlightFlags_t : uint8_t
{
	FLASHLIGHT_ON				= (1 << 0),
	FLASHLIGHT_UNK				= (1 << 1), // always seems to be set
	FLASHLIGHT_TURN_ON_NEXT_AIM = (1 << 2), // set when gun is lowered while flashlight was On. Used to remember state for the next time gun is aimed.
	FLASHLIGHT_UNK2				= (1 << 3),
	FLASHLIGHT_UNK3				= (1 << 4), // Spammed On/Off, unsure what it's for.
};

class CWeaponComponentFlashlight
{
public:
	virtual void m_0() = 0;
	virtual void m_1() = 0;
	virtual void New(void *memory, bool option) = 0;
	virtual bool Process(CPed* ped) = 0;
	virtual bool PostPreRender(CPed* ped) = 0;
	virtual void m_5() = 0;
	virtual void m_6() = 0;
	virtual void m_7() = 0;

	void* pFlashlightInfo; //0x0008
	class CWeapon* pParentWeapon; //0x0010
	void* pObject; //0x0018 CObject
	char pad_0020[20]; //0x0020
	uint32_t N000001A8; //0x0034 used in code, uint32, -1 = invalid
	char pad_0038[8]; //0x0038
	float aimFraction; //0x0040 -4 when standing, 500 while aiming. Used in code - if( x<=0 )
	float N000001A4; //0x0044
	char pad_0048[1]; //0x0048
	uint8_t flashlightFlags; //0x0049
};
class WeaponInfo
{
public:
	char pad_0000[16]; //0x0000
	uint32_t hash; //0x0010
	char pad_0014[76]; //0x0014
	void* ammoInfo; //0x0060
	char pad_0068[8]; //0x0068
	uint32_t maxClipSize; //0x0070
	//...
};
struct CPedInventory
{
	virtual ~CPedInventory() = default;
	void* unk;
	CPed* ped;
	//...
};
struct CWeapon
{
	virtual ~CWeapon() = default;
	void* extensions;
	rage::Vector4 muzzleOffset;
	rage::Vector4 worldPosition;
	char pad0[0x10]; //0x30-0x40
	WeaponInfo* info; //0x40-0x48
	int activeTime; //0x48-0x4C
	int lastFiredTime; //0x4C-0x50
	int activeTime2; //0x50-0x54
	uint16_t ammoTotal;
	uint16_t ammoInClip; //0x56-0x58
	__int64 pWeaponObject;
	CPedInventory* pInventoryPed; //0x60-=0x68
	CWeapon* pUnkWeapon; //self? //0x68-0x70
	char pad2[0xB0]; //0x70-0x120
	void* clipComponent; //0x120-0x128
	char pad3[0x30];
	void* pUnk; //0x158-0x160
	char pad4[0x60];
	char ammoState; //0x1C0
	DWORD weaponStateFlags; // 0x80 = silenced
};

// stolen from devtools-five
static hook::cdecl_stub<CPed*()> getLocalPlayerPed([]()
{
	static const auto addr = hook::get_pattern("E8 ? ? ? ? 48 85 C0 0F 84 C3 00 00 00 0F");
	return hook::get_call( addr );
});

typedef void (*flashlightProcessFn)(CWeaponComponentFlashlight*, CPed*);
static flashlightProcessFn origFlashlightProcess;

static std::atomic_bool g_SET_FLASH_LIGHT_KEEP_ON_WHILE_MOVING = false;

static void Flashlight_Process(CWeaponComponentFlashlight* thisptr, CPed* ped)
{
	// Copy every flag except for FLASHLIGHT_ON which is 1
	// 80 63 49 FE          and     byte ptr [rbx+49h], 0FEh
	// change to 0xFF so it copies the ON flag as well
	// (This byte is located in the original Flashlight::Process() function.)
	static unsigned char* g_flashlightAndByte = hook::get_pattern<unsigned char>("80 63 49 FE EB", 3);

	if (!g_SET_FLASH_LIGHT_KEEP_ON_WHILE_MOVING)
	{
		return origFlashlightProcess(thisptr, ped);
	}
	if (getLocalPlayerPed() != ped)
	{
		return origFlashlightProcess(thisptr, ped);
	}
	
	// spoof this float so it thinks we're aimed in. ( This needed is for the `weapon_flashlight` )
	thisptr->aimFraction = 500.0f;

	// Flashlight can be destroyed when climbing over terrain, etc, it will save this flag when recreated
	if (thisptr->flashlightFlags & FLASHLIGHT_TURN_ON_NEXT_AIM)
	{
		thisptr->flashlightFlags |= FLASHLIGHT_ON;
	}

	*g_flashlightAndByte = 0xFF;
	origFlashlightProcess(thisptr, ped);
	*g_flashlightAndByte = 0xFE;
}


static std::unordered_map<uint32_t /*Weapon Hash*/, short /*Ammo In Clip*/> g_LocalWeaponClipAmounts;
static std::atomic_bool g_SET_WEAPONS_NO_AUTORELOAD = false;
static std::atomic_bool g_SET_WEAPONS_NO_AUTOSWAP = false;

static bool (*origCPedWeapMgr_AutoSwap)(unsigned char*, bool, bool);
static std::atomic_bool attemptedSwap = false;
static bool __fastcall CPedWeaponManager_AutoSwap(unsigned char* thisptr, bool opt1, bool opt2)
{
	if (!g_SET_WEAPONS_NO_AUTOSWAP || *(CPed**)(thisptr + 16) != getLocalPlayerPed())
	{
		return origCPedWeapMgr_AutoSwap(thisptr, opt1, opt2);
	}

	attemptedSwap = true;
	return false;
}

static bool ( *origWantsReload )( CWeapon*, bool );
static bool WantsReload( CWeapon* thisptr, bool reloadWhenZero /*normally 1 - 0 means reload at 75% magazine capacity*/ )
{
	if( !g_SET_WEAPONS_NO_AUTORELOAD || !thisptr->pInventoryPed || thisptr->pInventoryPed->ped != getLocalPlayerPed() )
	{
		return origWantsReload( thisptr, reloadWhenZero );
	}

	return false;
}

typedef void* ( *Weapon_DtorFn )( void*, bool );
static Weapon_DtorFn origWeaponDtor;
static void* Weapon_DESTROY( CWeapon* thisptr, bool option )
{
	// This can be turned off while a weapon is still active.
	if( g_SET_WEAPONS_NO_AUTORELOAD )
	{
		g_LocalWeaponClipAmounts[thisptr->info->hash] = thisptr->ammoInClip;
	}

	return origWeaponDtor( thisptr, option );
}

typedef void* ( *CPed_DtorFn )( void*, bool );
static CPed_DtorFn origCPedDtor;
static void* CPed_DESTROY( CPed* thisptr, bool option )
{
	g_LocalWeaponClipAmounts.clear();
	return origCPedDtor( thisptr, option );
}

struct CWeapon_Vtable_Hook
{
	void* vfuncs[2];
} g_CWeapon_Vtable;

static bool ( *origSetupAsWeapon )( unsigned char*, WeaponInfo*, int, bool, CPed*, int64_t, bool, bool );
static bool __fastcall CPedEquippedWeapon_SetupAsWeapon( unsigned char* thisptr, WeaponInfo* weapInfo, int totalAmmo, bool a4, CPed* targetPed, int64_t weaponAddr, bool a7, bool a8 )
{
	if( !g_SET_WEAPONS_NO_AUTORELOAD || targetPed != getLocalPlayerPed() ) 	{
		return origSetupAsWeapon( thisptr, weapInfo, totalAmmo, a4, targetPed, weaponAddr, a7, a8 );
	}

	if( g_LocalWeaponClipAmounts.find( weapInfo->hash ) != g_LocalWeaponClipAmounts.end() ) 	{
		// Restore the saved ammo count from when the gun was destroyed
		totalAmmo = g_LocalWeaponClipAmounts[weapInfo->hash];
	}

	auto result = origSetupAsWeapon( thisptr, weapInfo, totalAmmo, a4, targetPed, weaponAddr, a7, a8 );

	// Original Function will call CreateWeapon()->CWeapon::CWeapon() if weaponAddr is NULL(which it is most of the time).
	// Afterwards it will store it in this+0x340[ver: 1604-2189]
	// Hook the CWeapon's that we own, when they are destroyed(by one of 100+ different ways), we will remember the ammo count in the clip and restore it.
	CWeapon* pWeap = *( CWeapon** )( thisptr + 0x340 );
	uintptr_t* vtable = ( uintptr_t* )pWeap;
	*vtable = ( uintptr_t )&g_CWeapon_Vtable;

	return result;
}

static void* (*g_TransitionStageFunc)(void*, int);
static void* CTaskGun_Stage1_1_TransitionStage(void* CTask, int newStage)
{
	CPed* ped = *(CPed**)(uintptr_t(CTask) + 16);

	if (ped == getLocalPlayerPed() && attemptedSwap)
	{
		attemptedSwap = false;
		return g_TransitionStageFunc(CTask, 1);
	}
	return g_TransitionStageFunc(CTask, newStage);
}

typedef void* (*CTaskAimGunOnFootProcessStagesFn)(unsigned char*, int, int);
static CTaskAimGunOnFootProcessStagesFn origCTaskAimGunOnFoot_ProcessStages;
static void* CTaskAimGunOnFoot_ProcessStages(unsigned char* thisptr, int stage, int substage)
{
	// Borrow the GetWeapon() and offset into Ped from another vfunc in this class.
	static unsigned char* addr = hook::get_pattern<unsigned char>("0F 84 ? ? ? ? 48 8B 8F ? ? ? ? E8 ? ? ? ? 48 8B F0");
	static uint32_t pedOffsetToWeaponMgr = *(uint32_t*)(addr + 9);
	typedef CWeapon* (*GetWeaponFn)(void*);
	static GetWeaponFn GetWeapon = hook::get_address<GetWeaponFn>((uintptr_t)addr + 13, 1, 5);

	CPed* ped = *(CPed**)(thisptr + 16);

	if (!(g_SET_WEAPONS_NO_AUTOSWAP || g_SET_WEAPONS_NO_AUTORELOAD) || ped != getLocalPlayerPed())
	{
		return origCTaskAimGunOnFoot_ProcessStages(thisptr, stage, substage);
	}

	if (stage == 2 && substage == 1)
	{
		CWeapon* pWeap = GetWeapon(*(void**)(uintptr_t(ped) + pedOffsetToWeaponMgr));
		if (pWeap && !pWeap->ammoInClip)
		{
			// Change state from shooting to idle - this is to fix a spasm when running out of ammo
			g_TransitionStageFunc(thisptr, 5);
		}
	}

	return origCTaskAimGunOnFoot_ProcessStages(thisptr, stage, substage);
}

static HookFunction hookFunction([]()
{
	{
		// _getWeaponInfoByName
		auto location = hook::get_pattern<char>("45 33 D2 85 C0 74 ? 44 0F B7 05");
		g_weaponCount = hook::get_address<uint16_t*>(location + 11);
		g_weaponList = hook::get_address<uint64_t**>(location + 26);
	}

	{
		WeaponDamageModifierOffset = *hook::get_pattern<int>("48 85 C9 74 ? F3 0F 10 81 ? ? ? ? F3 0F 59 81", 9);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_WEAPON_DAMAGE_MODIFIER", [](fx::ScriptContext& context)
	{
		auto weaponHash = context.GetArgument<int>(0);
		float result = 0.0f;

		for (int i = 0; i < *g_weaponCount - 1; i++)
		{
			auto weapon = (*g_weaponList)[i];
			auto hash = *(uint32_t*)(weapon + 16);

			if (hash == weaponHash)
			{
				result = *(float*)(weapon + WeaponDamageModifierOffset);
				break;
			}
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_FLASH_LIGHT_KEEP_ON_WHILE_MOVING", [](fx::ScriptContext& context) 
	{
		bool value = context.GetArgument<bool>(0);
		g_SET_FLASH_LIGHT_KEEP_ON_WHILE_MOVING = value;

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			resource->OnStop.Connect([]()
			{
				g_SET_FLASH_LIGHT_KEEP_ON_WHILE_MOVING = false;
			});
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WEAPONS_NO_AUTOSWAP", [](fx::ScriptContext& context)
	{
		bool value = context.GetArgument<bool>(0);
		g_SET_WEAPONS_NO_AUTOSWAP = value;

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			resource->OnStop.Connect([]()
			{
				g_SET_WEAPONS_NO_AUTOSWAP = false;
			});
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WEAPONS_NO_AUTORELOAD", [](fx::ScriptContext& context) 
	{
		bool value = context.GetArgument<bool>(0);
		g_SET_WEAPONS_NO_AUTORELOAD = value;

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			resource->OnStop.Connect([]()
			{
				g_SET_WEAPONS_NO_AUTORELOAD = false;
				g_LocalWeaponClipAmounts.clear();
			});
		}
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_SET_FLASH_LIGHT_KEEP_ON_WHILE_MOVING = false;
		g_SET_WEAPONS_NO_AUTORELOAD = false;
		g_SET_WEAPONS_NO_AUTOSWAP = false;
		g_LocalWeaponClipAmounts.clear();
	});

	uintptr_t* flashlightVtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("83 CD FF 48 8D 05 ? ? ? ? 33 DB", 6));
	origFlashlightProcess = (flashlightProcessFn)flashlightVtable[3];
	hook::put(&flashlightVtable[3], (uintptr_t)Flashlight_Process);


	// Disable auto-swaps
	{
		void* autoswap;
		if (xbr::IsGameBuildOrGreater<2060>())
		{
			autoswap = hook::pattern("48 8B 8B ? ? 00 00 45 33 C0 33 D2 E8 ? ? ? ? EB").count(3).get(1).get<void>(12);
		}
		else
		{
			autoswap = hook::get_pattern("EB ? 45 33 C0 33 D2 E8 ? ? ? ? EB ? 41", 7);
		}
		hook::set_call(&origCPedWeapMgr_AutoSwap, autoswap);
		hook::call(autoswap, CPedWeaponManager_AutoSwap);

		void* cTaskGun_Stage1_1_TransitionStage = hook::get_pattern("48 8B CF E8 ? ? ? ? E9 ? ? ? ? ? 85 ? 0F 84 ? ? ? ? 8A 83", 3);
		hook::set_call(&g_TransitionStageFunc, cTaskGun_Stage1_1_TransitionStage);
		hook::call(cTaskGun_Stage1_1_TransitionStage, CTaskGun_Stage1_1_TransitionStage);
	}

	// Disable auto-reloads
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("40 53 48 83 EC 20 4C 8B 49 40 33 DB"), WantsReload, (void**)&origWantsReload);
		MH_CreateHook(hook::get_pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 30 48 8B 44 24 78 41 8A"), CPedEquippedWeapon_SetupAsWeapon, (void**)&origSetupAsWeapon);
		MH_EnableHook(MH_ALL_HOOKS);

		// Get the original CWeapon vtable - We will plant a vmt-hook on weapons that we own so we can track their destruction.
		uintptr_t* cWeapon_vtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("45 33 FF 0F 57 C9 48 8D 05 ? ? ? ? 48 89 01", 9));
		origWeaponDtor = (Weapon_DtorFn)cWeapon_vtable[0];
		g_CWeapon_Vtable.vfuncs[0] = Weapon_DESTROY;
		g_CWeapon_Vtable.vfuncs[1] = (void*)cWeapon_vtable[1];

		// Get the original CPed vtable - we'll plant a hook here just on the destructor - this is for clearing the weapon clip history on death
		uintptr_t* cPed_vtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("4D 8B F8 48 8B F9 E8 ? ? ? ? 48 8D 05", 14));
		origCPedDtor = (CPed_DtorFn)cPed_vtable[0];
		hook::put(&cPed_vtable[0], (uintptr_t)CPed_DESTROY);
	}

	// Hook used by auto-reload/auto-swaps to fix a spasm when running out of ammo with the current weapon still held.
	uintptr_t* cTaskAimGun_vtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("48 89 44 24 20 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 8D 8B 20 01", 13));
	origCTaskAimGunOnFoot_ProcessStages = (CTaskAimGunOnFootProcessStagesFn)cTaskAimGun_vtable[14];
	hook::put(&cTaskAimGun_vtable[14], (uintptr_t)CTaskAimGunOnFoot_ProcessStages);
});
