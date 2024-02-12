#include <StdInc.h>
#include <Hooking.h>

//
// When the game creates a weapon object for the "CWeapon" instance, it creates a skeleton
// and has many checks to ensure it exists when necessary. However, when the game needs
// to "drop" a weapon from a ped to the world, it never removes the weapon object's skeleton.
// We're only patching a specific place that is known to cause "leaks".
//

namespace rage
{
class fwEntity;
}

class CObject;
class CPedWeaponManager;

static hook::cdecl_stub<void(rage::fwEntity*)> _deleteEntitySkeleton([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 33 D2 48 8B CB E8 ? ? ? ? 81 A3"));
});

static CObject*(*g_origPedWeaponManagerDropWeapon)(CPedWeaponManager*, uint32_t, bool, bool, bool, bool);
static CObject* PedWeaponManagerDropWeapon(CPedWeaponManager* self, uint32_t weaponHash, bool flag1, bool flag2, bool flag3, bool flag4)
{
	auto obj = g_origPedWeaponManagerDropWeapon(self, weaponHash, flag1, flag2, flag3, flag4);

	if (obj)
	{
		_deleteEntitySkeleton(reinterpret_cast<rage::fwEntity*>(obj));
	}

	return obj;
}

static HookFunction hookFunction([]
{
	auto location = hook::get_pattern("44 0F 29 6C 24 60 E8 ? ? ? ? 48", 6); 
	hook::set_call(&g_origPedWeaponManagerDropWeapon, location);
	hook::call(location, PedWeaponManagerDropWeapon);
});
