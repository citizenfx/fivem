#include <StdInc.h>
#include <Hooking.h>

#include "CrossBuildRuntime.h"

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

class CPedWeaponManager;
class CWeaponComponent;

// TODO: refactor and move to EntitySystem in the future.
class CWeapon
{
public:
	int GetNumComponents();
};

class CObject
{
public:
	CWeapon* GetWeapon();
};

static uint32_t g_objectWeaponPointerOffset;
static uint32_t g_weaponNumComponentsOffset;

static hook::cdecl_stub<void(rage::fwEntity*)> _deleteEntitySkeleton([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 33 D2 48 8B CB E8 ? ? ? ? 81 A3"));
});

inline CWeapon* CObject::GetWeapon()
{
	return *reinterpret_cast<CWeapon**>(reinterpret_cast<char*>(this) + g_objectWeaponPointerOffset);
}

inline int CWeapon::GetNumComponents()
{
	return *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + g_weaponNumComponentsOffset);
}

static CObject*(*g_origPedWeaponManagerDropWeapon)(CPedWeaponManager*, uint32_t, bool, bool, bool, bool);
static CObject* PedWeaponManagerDropWeapon(CPedWeaponManager* self, uint32_t weaponHash, bool flag1, bool flag2, bool flag3, bool flag4)
{
	auto object = g_origPedWeaponManagerDropWeapon(self, weaponHash, flag1, flag2, flag3, flag4);

	if (object)
	{
		auto removeSkeleton = true;

		// Don't remove the skeleton if we're dropping a "weapon" with attached components.
		if (const auto weapon = object->GetWeapon())
		{
			if (weapon->GetNumComponents() > 0)
			{
				removeSkeleton = false;
			}
		}

		if (removeSkeleton)
		{
			_deleteEntitySkeleton(reinterpret_cast<rage::fwEntity*>(object));
		}
	}

	return object;
}

static HookFunction hookFunction([]
{
	{
		auto location = hook::get_pattern("44 0F 29 6C 24 60 E8 ? ? ? ? 48", 6); 
		hook::set_call(&g_origPedWeaponManagerDropWeapon, location);
		hook::call(location, PedWeaponManagerDropWeapon);	
	}

	{
		auto location = hook::get_pattern("74 18 B8 80 00 00 00 48 8B CB", -7);
		g_objectWeaponPointerOffset = *reinterpret_cast<uint32_t*>(location);
	}

	{
		auto location = hook::get_pattern("4D 85 C9 7E 20 48 8D B9 ? ? ? ? 48 8B C7", -6);
		g_weaponNumComponentsOffset = *reinterpret_cast<uint32_t*>(location);
	}

#if _DEBUG
	// Valid for 1604..3095 game builds.
	assert(g_objectWeaponPointerOffset == (xbr::IsGameBuildOrGreater<2802>() ? 0x320 : 0x340));
	assert(g_weaponNumComponentsOffset == (xbr::IsGameBuildOrGreater<2802>() ? 0x128 : 0x120));
#endif
});
