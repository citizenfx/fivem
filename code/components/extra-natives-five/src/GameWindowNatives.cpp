#include <StdInc.h>
#include <windowsx.h>
#include <ScriptEngine.h>
#include <CrossBuildRuntime.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("IS_GAME_WINDOW_FOCUSED", [](fx::ScriptContext& context)
	{
		bool result = false;

		auto gameWindow = CoreGetGameWindow();
		if (gameWindow)
		{
			result = gameWindow == GetForegroundWindow();
		}

		context.SetResult<bool>(result);
	});
});
