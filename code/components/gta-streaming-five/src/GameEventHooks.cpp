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

	class aiEvent : public fwEvent 
	{
	public:
		fwEntity* GetSource() 
		{
			const auto vtable = *reinterpret_cast<uintptr_t*>(this);
			return (*reinterpret_cast<fwEntity*(**)(rage::aiEvent*)>(vtable + 27 * sizeof(uintptr_t)))(this);
		}
	};
}

class CEventGroup {};

class CEventGroupPed : public CEventGroup 
{
public:
	fwEntity* GetPed() 
	{
		static const auto pedOffset = *hook::pattern("48 8B C6 48 89 BE ? ? ? ?").count(1).get(0).get<uint32_t>(6);
		return *reinterpret_cast<fwEntity**>(this + pedOffset);
	}
};

static hook::cdecl_stub<uint32_t(void*)> getScriptGuidForEntity([]() 
{
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
});

void*(*g_eventCall1)(void* group, void* event);
void*(*g_eventCall2)(void* group, void* event);
void*(*g_eventCall3)(void* group, void* event);

template<decltype(g_eventCall1)* TFunc, bool aiEvent = false>
void* HandleEventWrap(CEventGroup* group, rage::fwEvent* event)
{
	if (event)
	{
		try
		{
			const char* eventName = typeid(*event).name();

			GameEventMetaData data = { 0 };
			strcpy(data.name, &eventName[6]);
			data.numArguments = 0;

			if (aiEvent) 
			{
				const auto ped = static_cast<CEventGroupPed*>(group)->GetPed();

				if (ped != nullptr) 
				{
					data.arguments[data.numArguments++] = getScriptGuidForEntity(ped);

					// The source entity involved in this AI event, if one exists,
					// *should* be accessible by vtable lookup.
					const auto sourceEntity = static_cast<rage::aiEvent*>(event)->GetSource();

					if (sourceEntity != nullptr) 
					{
						data.arguments[data.numArguments++] = getScriptGuidForEntity(sourceEntity);
					}
				}
			} 
			else 
			{
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

static HookFunction hookFunction([]()
{
	MH_Initialize();

	// 8-bit event pools

	{
		auto matches = hook::pattern("83 BF ? ? 00 00 ? 75 ? 48 8B CF E8 ? ? ? ? 83 BF").count(2);

		MH_CreateHook(matches.get(0).get<void>(-0x36), HandleEventWrap<&g_eventCall1, true>, (void**)&g_eventCall1);

		// we can't read out any data from this event call yet
		// MH_CreateHook(matches.get(1).get<void>(-0x36), HandleEventWrap<&g_eventCall2, true>, (void**)&g_eventCall2);
	}

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