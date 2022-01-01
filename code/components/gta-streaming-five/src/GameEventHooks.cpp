#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

#include <MinHook.h>

namespace rage
{
	class fwEvent
	{
	public:
		virtual ~fwEvent() = 0;

		virtual void m_8() = 0;

		virtual bool Equals(rage::fwEvent* other) = 0;

		virtual int32_t GetId() = 0;

		virtual int m_20() = 0;

		virtual int m_28() = 0;

		virtual bool GetArguments(void* buffer, size_t len) = 0;

		virtual bool m_38() = 0;

		virtual bool m_40(rage::fwEvent* other) = 0;
	};
}

void*(*g_eventCall1)(void* group, void* event);
void*(*g_eventCall2)(void* group, void* event);
void*(*g_eventCall3)(void* group, void* event);

template<decltype(g_eventCall1)* TFunc>
void* HandleEventWrap(void* group, rage::fwEvent* event)
{
	if (event)
	{
		try
		{
			const char* eventName = typeid(*event).name();

			GameEventMetaData data = { 0 };
			strcpy(data.name, &eventName[6]);
			data.numArguments = 0;

			// brute-force the argument count
			// since these functions should early exit, most cost here will be VMT dispatch
			for (int i = 0; i < _countof(data.arguments); i++)
			{
				if (event->GetArguments(data.arguments, i * sizeof(uintptr_t)))
				{
					data.numArguments = i;
					break;
				}
			}

			OnTriggerGameEvent(data);
		}
		catch (std::exception& e)
		{
		
		}
	}

	return (*TFunc)(group, event);
}

#include <sstream>

namespace rage
{
class netObject;
}

typedef void (*OnEntityTakeDmgFn)(rage::netObject*, void*, uint8_t);
static OnEntityTakeDmgFn origOnEntityTakeDmg;

static void OnEntityTakeDmg(rage::netObject* thisptr, void* dmgInfo, uint8_t unk)
{
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		// Hack: 2060+ does not set the damageSource for fall damage. This is checked != 0 before sending the event
		if (*((DWORD*)dmgInfo + 8) == 0)
		{
			*((DWORD*)dmgInfo + 8) = 0xCDC174B0; // damageSource = WORLD (hash)
		}
	}
	return origOnEntityTakeDmg(thisptr, dmgInfo, unk);
}

#if 0
struct Damage
{
	rage::fwEntity* culprit;
	float baseDamage;
	uint32_t weapon;

	// more fields follow
};

static bool (*origDamageProcess)(Damage* damage, rage::fwEntity* victim, uintptr_t a3, uintptr_t a4, uintptr_t a5, uintptr_t a6);

bool DamageProcess(Damage* damage, rage::fwEntity* victim, uintptr_t a3, uintptr_t a4, uintptr_t a5, uintptr_t a6)
{
	bool rv = origDamageProcess(damage, victim, a3, a4, a5, a6);

	DamageEventMetaData md;
	md.baseDamage = damage->baseDamage;
	md.victim = victim;
	md.culprit = damage->culprit;
	md.weapon = damage->weapon;

	OnEntityDamaged(md);

	return rv;
}
#endif

static void (*origEntityLogDamage)(void* victim, void* culprit, uint32_t weapon, uint32_t time, bool a5);

static void EntityLogDamage(rage::fwEntity* victim, rage::fwEntity* culprit, uint32_t weapon, uint32_t time, bool a5)
{
	origEntityLogDamage(victim, culprit, weapon, time, a5);

	DamageEventMetaData md;
	md.baseDamage = 0.0f;
	md.victim = victim;
	md.culprit = culprit;
	md.weapon = weapon;

	OnEntityDamaged(md);
}

static HookFunction hookFunction([]()
{
	MH_Initialize();

	// 8-bit event pools

	// these are for ped events, we don't handle getting entities/positions from aiEvent instances yet
	/*{
		auto matches = hook::pattern("83 BF ? ? 00 00 ? 75 ? 48 8B CF E8 ? ? ? ? 83 BF").count(2);

		MH_CreateHook(matches.get(0).get<void>(-0x36), HandleEventWrap<&g_eventCall1>, (void**)&g_eventCall1);
		MH_CreateHook(matches.get(1).get<void>(-0x36), HandleEventWrap<&g_eventCall2>, (void**)&g_eventCall2);
	}*/

	{
		MH_CreateHook(hook::get_pattern("81 BF ? ? 00 00 ? ?  00 00 75 ? 48 8B CF E8", -0x36), HandleEventWrap<&g_eventCall3>, (void**)&g_eventCall3);
	}


	// fix for invalid damage sources in events
	uintptr_t* cNetObjPhys_vtable = hook::get_address<uintptr_t*>(hook::get_pattern<unsigned char>("88 44 24 20 E8 ? ? ? ? 33 C9 48 8D 05", 14));
	if (xbr::IsGameBuildOrGreater<2189>())
	{
		MH_CreateHook((LPVOID)cNetObjPhys_vtable[128], OnEntityTakeDmg, (void**)&origOnEntityTakeDmg);
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		MH_CreateHook((LPVOID)cNetObjPhys_vtable[127], OnEntityTakeDmg, (void**)&origOnEntityTakeDmg);
	}

	MH_CreateHook(hook::get_pattern("21 4D D8 21 4D DC 41 8B D8", -0x1F), EntityLogDamage, (void**)&origEntityLogDamage);

	/* // ped damage specific
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		MH_CreateHook(hook::get_pattern("41 8A 40 08 84 C1", -0x56), DamageProcess, (void**)&origDamageProcess);
	}
	else
	{
		MH_CreateHook(hook::get_pattern("41 80 60 09 FC 24 40", -0x5D), DamageProcess, (void**)&origDamageProcess);
	}
	*/

	MH_EnableHook(MH_ALL_HOOKS);

	/*
	OnTriggerGameEvent.Connect([](const GameEventMetaData& data)
	{
		std::stringstream argStr;

		argStr << "(" << data.numArguments << ") ";

		for (int i = 0; i < data.numArguments; i++)
		{
			argStr << " " << data.arguments[i];
		}

		trace("game event %s, args %s\n", data.name, argStr.str());
	});
	*/
});

fwEvent<const GameEventMetaData&> OnTriggerGameEvent;
fwEvent<const DamageEventMetaData&> OnEntityDamaged;
