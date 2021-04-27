#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>
#include <ICoreGameInit.h>

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
	
	// Flashlight can be destroyed when climbing over terrain, etc, it will save this flag when recreated
	if (thisptr->flashlightFlags & FLASHLIGHT_TURN_ON_NEXT_AIM)
	{
		thisptr->flashlightFlags |= FLASHLIGHT_ON;
	}

	*g_flashlightAndByte = 0xFF;
	origFlashlightProcess(thisptr, ped);
	*g_flashlightAndByte = 0xFE;
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
	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_SET_FLASH_LIGHT_KEEP_ON_WHILE_MOVING = false;
	});

	uintptr_t* flashlightVtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("83 CD FF 48 8D 05 ? ? ? ? 33 DB", 6));
	origFlashlightProcess = (flashlightProcessFn)flashlightVtable[3];
	flashlightVtable[3] = (uintptr_t)Flashlight_Process;
});
