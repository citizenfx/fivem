#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

#include <MinHook.h>

#include <sstream>
#include <msgpack.hpp>

namespace rage
{
	class netObject;

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

	class fwEventGroup
	{
	public:
		// somehow always ended up to be 0x0000000000000000
		size_t unk;

		// How much would there be, it looked like it was filling it with 30-ish 64 bit values
		// or maybe we should check until 17 (16?) like the HandleEventWrapReact version?
		// Until now I got a total of 13 entities at once
		fwEntity* entities[24];

		virtual ~fwEventGroup() = 0;
	};
}

template<typename... TArg>
inline void msgpack_array(msgpack::packer<msgpack::sbuffer>& p, const TArg&... args)
{
	p.pack_array(sizeof...(args));
	(p.pack(args), ...);
}

template<typename... TArg>
inline void msgpack_pack(msgpack::packer<msgpack::sbuffer>& p, const TArg&... args)
{
	(p.pack(args), ...);
}

inline uint32_t GetGuidFromBaseSafe(rage::fwEntity* entity)
{
	return entity ? rage::fwScriptGuid::GetGuidFromBase(entity) : 0;
}

void* (*g_eventCall1)(void* group, void* event);
void* (*g_eventCall2)(void* group, void* event);
void* (*g_eventCall3)(void* group, void* event);

template<decltype(g_eventCall3)* TFunc>
void* HandleEventWrap(rage::fwEventGroup* group, rage::fwEvent* event)
{
	if (event)
	{
		try
		{
			GameEventMetaData data{ typeid(*event).name() + 6, 0 };

			// brute-force the argument count
			// since these functions should early exit, most cost here will be VMT dispatch
			// ^ as these functions have a hardcoded size check, though not all...
			for (int i = 0; i < _countof(data.arguments); i++)
			{
				if (event->GetArguments(data.arguments, i * sizeof(size_t)))
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

template<decltype(g_eventCall1)* TFunc>
void* HandleEventWrapReact(rage::fwEventGroup* group, rage::fwEvent* event)
{
	if (event)
	{
		/*
		 * event:
		 * These events are stored with different member variables and with different layouts per class/struct,
		 * so we have to investigate them case by case.
		 * 
		 * group:
		 * In this reacting version the entity that triggers this event is stored in group->entities[17]
		 * as well as in group->entities[18].
		 */

		try
		{
			msgpack::sbuffer buf;
			msgpack::packer packer(buf);

			const char* eventName = typeid(*event).name() + 6;
			rage::fwEntity* entity = group->entities[17];

			packer.pack_array(4); // we'll offer 4 parameters
			msgpack_pack(packer, eventName, event->GetId(), GetGuidFromBaseSafe(entity));

			// Quick 8 bytes check, i.e.: no strcmp
			// this will turn the first 8 bytes after "CEvent" into an uint64 and check against that value
			switch ((uint64_t&)(eventName[6]))
			{
				case 0x676e696b636f6853u: // "Shocking"
				{
					msgpack_array(packer, GetGuidFromBaseSafe((rage::fwEntity*&)event[13]), (float(&)[3])event[8]);
					break;
				}
				case 0x6169746e65746f50u: // "Potentia" little endian
				{
					// TODO: check how we will do this without a strcmp
					if (strcmp(eventName + 15, "WalkIntoVehicle") == 0)
					{
						msgpack_array(packer, GetGuidFromBaseSafe((rage::fwEntity*&)event[8]), (float(&)[3])event[5]);
					}
					else if (strcmp(eventName + 15, "GetRunOver") == 0)
					{
						msgpack_array(packer, GetGuidFromBaseSafe((rage::fwEntity*&)(event[6])));
						// doesn't seem there is a vector3, might have other info, speed, vector2?
					}
					else
					{
						packer.pack_array(0); // empty array
					}
					break;
				}
				default:
				{
					packer.pack_array(0); // empty array
					break;
				}
			}

			OnTriggerGameEventReact({ buf.data(), buf.size() });
		}
		catch (std::exception& e)
		{
		}
	}

	return (*TFunc)(group, event);
}

template<decltype(g_eventCall2)* TFunc>
void* HandleEventWrapEmit(rage::fwEventGroup* group, rage::fwEvent* event)
{
	if (event)
	{
		/*
		 * event:
		 * These events are stored with different member variables and with different layouts per class/struct,
		 * so we have to investigate them case by case.
		 * 
		 * group:
		 * In this emitting version they store them from group->entities[0] to group->entities[n] 
		 * where it ends with 0x0 or a repeating entity ptr.
		 */

		try
		{
			msgpack::sbuffer buf;
			msgpack::packer packer(buf);

			const char* eventName = typeid(*event).name() + 6;

			packer.pack_array(4); // we'll offer 4 parameters
			msgpack_pack(packer, eventName, event->GetId());

			// retrieve all entities
			rage::fwEntity* ent2 = nullptr; // to ignore duplicates on the end
			rage::fwEntity** ent1 = group->entities; // to ignore duplicates on the end
			
			// iterate ptrs until we find a nullptr, a duplicate, or if we reached the end of the buffer size
			size_t size = 0;
			for (; *ent1 && *ent1 != ent2 && ++size < _countof(group->entities); ent2 = *ent1++);
			packer.pack_array(size);

			for (size_t i = 0; i < size; ++i)
			{
				packer.pack(rage::fwScriptGuid::GetGuidFromBase(group->entities[i]));
			}

			// Quick 8 bytes check, i.e.: no strcmp
			// this will turn the first 8 bytes after "CEvent" into an uint64 and check against that value
			switch ((uint64_t&)(eventName[6]))
			{
				case 0x676e696b636f6853u: // "Shocking"
				{
					msgpack_array(packer, GetGuidFromBaseSafe((rage::fwEntity*&)event[13]), (float(&)[3])event[8]);
					break;
				}
				default:
				{
					packer.pack_array(0); // empty array
					break;
				}
			}

			OnTriggerGameEventEmit({ buf.data(), buf.size() });
		}
		catch (std::exception& e)
		{
		}
	}

	return (*TFunc)(group, event);
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
	{
		auto matches = hook::pattern("83 BF ? ? 00 00 ? 75 ? 48 8B CF E8 ? ? ? ? 83 BF").count(2);

		MH_CreateHook(matches.get(0).get<void>(-0x36), HandleEventWrapReact<&g_eventCall1>, (void**)&g_eventCall1);
		MH_CreateHook(matches.get(1).get<void>(-0x36), HandleEventWrapEmit<&g_eventCall2>, (void**)&g_eventCall2);
	}

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
fwEvent<std::string_view> OnTriggerGameEventEmit;
fwEvent<std::string_view> OnTriggerGameEventReact;
fwEvent<const DamageEventMetaData&> OnEntityDamaged;
