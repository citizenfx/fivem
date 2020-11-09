#include <StdInc.h>
#include <ScriptEngine.h>
#include <Hooking.h>

#include "NativeWrappers.h"
#include "Local.h"

class STREAMING_EXPORT CPhysical : public fwEntity {
};

static HookFunction initFunction([]()
{
	auto concealed = hook::pattern("48 83 EC 28 B2 01 E8 ? ? ? ? 48 85 C0 74 1A").count(1).get(0);
	static uint32_t netObjOffset = *concealed.get<uint32_t>(19); // 1604: 0xD0
	static uint32_t concealedOffset = *concealed.get<uint32_t>(38) / 8; // 1604: 0x468

	auto makeEntityFunction = [](auto fn, uintptr_t defaultValue = 0)
	{
		return [=](fx::ScriptContext& context)
		{
			if (fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(context.GetArgument<int>(0)))
			{
				context.SetResult(fn(context, entity));
			}
			else
			{
				context.SetResult(defaultValue);
			}
		};
	};

	fx::ScriptEngine::RegisterNativeHandler("NETWORK_IS_OBJECT_CONCEALED", makeEntityFunction([](fx::ScriptContext& context, fwEntity* entity)
	{
		if (entity->IsOfType<CPhysical>())
		{
			if (uintptr_t netobj = *(uintptr_t*)((char*)entity + netObjOffset))
			{
				auto vtbl = *(uintptr_t**)netobj;
				return ((bool(*)(uintptr_t))vtbl[concealedOffset])(netobj);
			}
		}
		return false;
	}, false));
});
