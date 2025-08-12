#include "StdInc.h"

#include "scrEngine.h"
#include "InputHook.h"
#include <ConsoleHost.h>


rage::scrEngine::NativeHandler m_origSetCursorLocation;
static HookFunction hookFunction([]()
{
	//_SET_CURSOR_LOCATION
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		m_origSetCursorLocation = rage::scrEngine::GetNativeHandler(0xFC695459D4D0E219);
		rage::scrEngine::RegisterNativeHandler(0xFC695459D4D0E219, [](rage::scrNativeCallContext* context)
		{
			HWND gameWindow = CoreGetGameWindow();
			bool consoleEnabled = ConHost::IsConsoleOpen();
			
			if (GetForegroundWindow() == gameWindow && !consoleEnabled)
			{
				InputHook::EnableSetCursorPos(true);
				(*m_origSetCursorLocation)(context);
				InputHook::EnableSetCursorPos(false);
			}
		});
	});

});
