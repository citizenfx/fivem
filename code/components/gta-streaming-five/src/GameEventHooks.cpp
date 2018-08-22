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

	return (*TFunc)(group, event);
}

#include <sstream>

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
