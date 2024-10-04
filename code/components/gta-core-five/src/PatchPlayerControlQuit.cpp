#include <StdInc.h>
#include <Hooking.h>

#include <MinHook.h>

//
// This patches a function used to (among other things) instantly halt player vehicles
// if a warning screen, SET_PLAYER_CONTROL, or some other causes are triggering the player
// to be uncontrollable.
// 
// With the patch applied, if there's a warning screen active, and the title is EXIT_SURE or
// EXIT_SURE_2, we'll pretend that the player is controllable anyway, so that pressing Alt-F4
// while in a moving vehicle will not instantly halt the player's vehicle.
// 
// This, however, has the side effect that the player will also not be marked as uncontrollable here
// if they have been made uncontrollable for a reason other than warning screens, and they open this
// warning screen.
//

static hook::cdecl_stub<uint32_t()> _getWarningMessageTitle([]()
{
	return hook::get_pattern("74 0F E8 ? ? ? ? 84 C0 74 06 8B 1D", -33);
});

static bool (*g_origIsUncontrollable)(void*);

static bool CControl__IsUncontrollableStub(void* self)
{
	bool isUncontrollable = g_origIsUncontrollable(self);

	if (isUncontrollable)
	{
		auto title = _getWarningMessageTitle();

		if (title && (title == HashString("EXIT_SURE") || title == HashString("EXIT_SURE_2")))
		{
			isUncontrollable = false;
		}
	}

	return isUncontrollable;
}

static HookFunction hookFunction([]
{
	MH_Initialize();

	{
		auto location = hook::get_pattern("74 09 80 3D ? ? ? ? 00 75 10 48 8D", -0x1B);
		MH_CreateHook(location, CControl__IsUncontrollableStub, (void**)&g_origIsUncontrollable);
		MH_EnableHook(location);
	}
});
