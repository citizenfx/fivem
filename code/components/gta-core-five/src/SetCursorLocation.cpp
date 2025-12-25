#include "StdInc.h"

#include "scrEngine.h"
#include "InputHook.h"

enum NativeIdentifiers : uint64_t
{
	_SET_CURSOR_LOCATION = 0xFC695459D4D0E219,
};

rage::scrEngine::NativeHandler m_origSetCursorLocation;
static HookFunction hookFunction([]()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		m_origSetCursorLocation = rage::scrEngine::GetNativeHandler(_SET_CURSOR_LOCATION);
		rage::scrEngine::RegisterNativeHandler(_SET_CURSOR_LOCATION, [](rage::scrNativeCallContext* context)
		{
			InputHook::EnableSetCursorPos(true);
			(*m_origSetCursorLocation)(context);
			InputHook::EnableSetCursorPos(false);
		});
	});

});
