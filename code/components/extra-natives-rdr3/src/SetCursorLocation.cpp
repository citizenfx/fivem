#include "StdInc.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <InputHook.h>

static hook::cdecl_stub<bool(float, float)> _setCursorLocation([]()
{
	return hook::get_pattern("0F 28 F8 75 ? 66 0F 6E 15 ? ? ? ? 66", -24);
});

static HookFunction hookFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_CURSOR_LOCATION", [](fx::ScriptContext& context)
	{
		auto posX = context.GetArgument<float>(0);
		auto posY = context.GetArgument<float>(1);

		InputHook::EnableSetCursorPos(true);
		auto result = _setCursorLocation(posX, posY);
		InputHook::EnableSetCursorPos(false);

		context.SetResult<bool>(result);
	});
});
