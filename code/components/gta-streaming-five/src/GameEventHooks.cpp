#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

#include <MinHook.h>

#include <sstream>
#include <msgpack.hpp>

#include <CrossBuildRuntime.h>
#include "XBRVirtual.h"
#include <GameEventNames.h>

namespace rage
{
	class netObject;

	class fwEvent : XBR_VIRTUAL_BASE_2802(0)
	{
	public:
		XBR_VIRTUAL_DTOR(fwEvent)

		XBR_VIRTUAL_METHOD(void, m_8, ())

		XBR_VIRTUAL_METHOD(bool, Equals, (rage::fwEvent* other))

		XBR_VIRTUAL_METHOD(int32_t, GetId, ())

		XBR_VIRTUAL_METHOD(int, m_20, ())

		XBR_VIRTUAL_METHOD(int, m_28, ())

		XBR_VIRTUAL_METHOD(bool, GetArguments, (void* buffer, size_t len))

		XBR_VIRTUAL_METHOD(bool, m_38, ())

		XBR_VIRTUAL_METHOD(bool, m_40, (rage::fwEvent* other))

		XBR_VIRTUAL_METHOD(bool, m_0x9, ())
		XBR_VIRTUAL_METHOD(bool, m_0xA, ())
		XBR_VIRTUAL_METHOD(bool, m_0xB, ())
		XBR_VIRTUAL_METHOD(bool, m_0xC, ())
		XBR_VIRTUAL_METHOD(bool, m_0xD, ())
		XBR_VIRTUAL_METHOD(bool, m_0xE, ())
		XBR_VIRTUAL_METHOD(void, m_0xF, ())
		XBR_VIRTUAL_METHOD(bool, m_0x10, ())
		XBR_VIRTUAL_METHOD(bool, m_0x11, ())
		XBR_VIRTUAL_METHOD(bool, m_0x12, ())
		XBR_VIRTUAL_METHOD(bool, m_0x13, ())
		XBR_VIRTUAL_METHOD(bool, m_0x14, ())
		XBR_VIRTUAL_METHOD(uint32_t, m_0x15, ())
		XBR_VIRTUAL_METHOD(uint32_t, m_0x16, ())
		XBR_VIRTUAL_METHOD(uint32_t, m_0x17, ())
		XBR_VIRTUAL_METHOD(bool, m_0x18, (void*, void*))

		// vtable[25]: retrieve the entity associated with this event object
		XBR_VIRTUAL_METHOD(fwEntity*, GetEntity, ())
	};

	class fwEventGroup
	{
	public:		
		size_t unk; // somehow always ended up to be 0x0000000000000000

		// it looked like it was filling 30-ish 64 bit values.
		// until now I got a total of 13 events at one time.
		fwEvent* events[30];
				
		uint32_t unk2;
		uint32_t unk3; // it's like this one is used to store how much of these event wrappers are processed.
		uint32_t eventsCount; // Stores the amount of events, there's an exception case for a count of 32.
		uint32_t unk4;

		size_t unk5;

		virtual ~fwEventGroup() = 0;
	};

	class fwEventGroup2
	{
	public:
		size_t unk0; // somehow always ended up to be 0x0000000000000000

		// a known case where it was filled with 1 was when I was punching,
		// it contained a ptr to the same entity stored in this object's entity members.
		fwEvent* events[15];

		size_t eventsCount_1; // stores the size of events, difference with eventsCount_2 is unknown.
		size_t eventsCount_2; // stores the size of events, difference with eventsCount_1 is unknown.

		fwEntity* entity; // the entity reacting
		fwEntity* entityDuplicate; // stores same as entity
				
		size_t unk1[8]; // more allocated data filled with 0x0, size is unknown

		virtual ~fwEventGroup2() = 0;
	};
}

static const char* GetEventName(rage::fwEvent* ev)
{
	int eventId = ev->GetId();

	if (xbr::IsGameBuildOrGreater<2612>())
	{
		return (eventId < _countof(g_eventNames2612)) ? g_eventNames2612[eventId] : nullptr;
	}
	else if (xbr::IsGameBuildOrGreater<2545>())
	{
		return (eventId < _countof(g_eventNames2545)) ? g_eventNames2545[eventId] : nullptr;
	}
	else if (xbr::IsGameBuildOrGreater<2189>())
	{
		return (eventId < _countof(g_eventNames2189)) ? g_eventNames2189[eventId] : nullptr;
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		return (eventId < _countof(g_eventNames2060)) ? g_eventNames2060[eventId] : nullptr;
	}
	
	return (eventId < _countof(g_eventNames1604)) ? g_eventNames1604[eventId] : nullptr;
}

namespace
{
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
}

void*(*g_eventCall1)(void* group, void* event);
void*(*g_eventCall2)(void* group, void* event);
void*(*g_eventCall3)(void* group, void* event);

template<decltype(g_eventCall3)* TFunc>
void* HandleEventWrap(rage::fwEventGroup* group, rage::fwEvent* event)
{
	if (event)
	{
		try
		{
			if (auto eventName = GetEventName(event))
			{
				GameEventMetaData data{ eventName, 0 };

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
		}
		catch (std::exception& e)
		{
		}
	}

	return (*TFunc)(group, event);
}

template<decltype(g_eventCall1)* TFunc, int stream>
void* HandleEventWrapExt(rage::fwEventGroup* group, rage::fwEvent* event)
{
	if (event)
	{
		/*
		 * event:
		 * These events are stored with different member variables and with different layouts per class/struct,
		 * to get more data from them we'll have to investigate them case by case.
		 */

		try
		{
			if (auto eventName = GetEventName(event))
			{
				GameEventData data{ eventName };

				msgpack::sbuffer buf;
				msgpack::packer packer(buf);

				packer.pack_array(3); // we'll offer 3 parameters

				if constexpr (stream == 1)
				{
					// retrieve all entities this event is being sent to
					size_t size = group->eventsCount;
					packer.pack_array(size);

					for (size_t i = 0; i < size; ++i)
					{
						packer.pack(GetGuidFromBaseSafe(group->events[i]->GetEntity()));
					}
				}
				else
				{
					// retrieve entity who is triggering this
					// this group has a different layout
					msgpack_array(packer, GetGuidFromBaseSafe(reinterpret_cast<rage::fwEventGroup2*>(group)->entity));
				}

				// retrieve entity related to the event
				msgpack_pack(packer, GetGuidFromBaseSafe(event->GetEntity()));

				// retrieve extra event data
				const char* eventSubName = data.name + 6; // get part after "CEvent"

				if (memcmp(eventSubName, "Shoc", 4) == 0) // Shocking
				{
					msgpack_array(packer, (float(&)[3])event[8]);
				}
				else
				{
					packer.pack_array(0); // empty array
				}

				data.argsData = std::string_view(buf.data(), buf.size());
				OnTriggerGameEventExt(data);
			}
		}
		catch (std::exception&)
		{
			// an std::bad_alloc could be thrown when packing (msgpack);
			// should we do anything else except for not sending this to scripts?
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

		MH_CreateHook(matches.get(0).get<void>(-0x36), HandleEventWrapExt<&g_eventCall1, 0>, (void**)&g_eventCall1);
		MH_CreateHook(matches.get(1).get<void>(-0x36), HandleEventWrapExt<&g_eventCall2, 1>, (void**)&g_eventCall2);
	}

	{
		MH_CreateHook(hook::get_pattern("81 BF ? ? 00 00 ? ?  00 00 75 ? 48 8B CF E8", -0x36), HandleEventWrap<&g_eventCall3>, (void**)&g_eventCall3);
	}


	// fix for invalid damage sources in events
	if (xbr::IsGameBuildOrGreater<2060>())
	{
		void** cNetObjPhys_vtable = hook::get_address<void**>(hook::get_pattern<unsigned char>("88 44 24 20 E8 ? ? ? ? 33 C9 48 8D 05", 14));
		int vtableIdx = 0;

		if (xbr::IsGameBuildOrGreater<2802>())
		{
			vtableIdx = 136;
		}
		else if (xbr::IsGameBuildOrGreater<2545>())
		{
			vtableIdx = 130;
		}
		else if (xbr::IsGameBuildOrGreater<2189>())
		{
			vtableIdx = 128;
		}
		else if (xbr::IsGameBuildOrGreater<2060>())
		{
			vtableIdx = 127;
		}

		MH_CreateHook(cNetObjPhys_vtable[vtableIdx], OnEntityTakeDmg, (void**)&origOnEntityTakeDmg);
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
fwEvent<const GameEventData&> OnTriggerGameEventExt;
fwEvent<const DamageEventMetaData&> OnEntityDamaged;
