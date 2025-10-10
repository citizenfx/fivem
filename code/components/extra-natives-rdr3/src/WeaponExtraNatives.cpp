#include "StdInc.h"

#include "fxScripting.h"
#include "Hooking.h"
#include "Hooking.Stubs.h"
#include <ICoreGameInit.h>
#include <ScriptEngine.h>

#include "Resource.h"
#include "om/OMPtr.h"

static std::atomic_bool g_SET_WEAPONS_NO_AUTORELOAD = false;

static hook::cdecl_stub<void*()> getLocalPlayerPed([]()
{
	static const auto addr = hook::get_pattern("E8 ? ? ? ? 48 85 C0 74 ? 48 8B D3 48 8B C8 E8 ? ? ? ? 84 C0 74 ? F3 0F 10 05");
	return hook::get_call( addr );
});

struct CWeapon
{
	char   gap_0[0x10];          // 0x00 - 0x0F
	__int16 pad_10;              // 0x10
	char    gap_12[0x4E];        // 0x12 - 0x5F

	void*   m_pWeaponInfo;       // 0x60
	uint32_t m_weaponHash;       // 0x68
	__int16 pad_6C;              // 0x6C
	char    gap_6E[0x2A];        // 0x6E - 0x97

	unsigned int m_uGlobalTime;  // 0x98
	unsigned int m_uTimer;       // 0x9C
	char    gap_A0[0x6];         // 0xA0 - 0xA5

	__int16 m_iAmmoTotal;        // 0xA6
	char    gap_A8[0x40];        // 0xA8 - 0xE7

	void*   m_pDrawableEntity;   // 0xE8
	void*   m_pObserver;         // 0xF0
	char    gap_F8[0x8];         // 0xF8 - 0xFF
};

static bool ( *origGetNeedsToReload )( CWeapon*, bool );
static bool GetNeedsToReload( CWeapon* thisptr, bool reloadWhenZero)
{
	using GetOwnerFn = const void* (__fastcall*)(void* self);
	bool localPlayer = thisptr->m_pObserver == nullptr;

	if(thisptr->m_pObserver)
	{
		const void* owner = reinterpret_cast<GetOwnerFn>(
			(*reinterpret_cast<void***>(thisptr->m_pObserver))[0x58 / 8]
		)(thisptr->m_pObserver);

		localPlayer = owner == getLocalPlayerPed();
	}
	if( !g_SET_WEAPONS_NO_AUTORELOAD || !localPlayer)
	{
		return origGetNeedsToReload( thisptr, reloadWhenZero );
	}

	return false;
}

static HookFunction hookFunction([]()
{
	// Disable auto reload
	{
		origGetNeedsToReload = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 ? 48 85 F6 0F 84 ? ? ? ? 41 8A D4")), GetNeedsToReload);
	}

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
			});
		}
	});
});