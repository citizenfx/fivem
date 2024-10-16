#include <StdInc.h>
#include <windowsx.h>
#include <ScriptEngine.h>
#include <CrossBuildRuntime.h>

bool IsWindowFullscreen(HWND hwnd) {
	RECT windowRect;
	GetWindowRect(hwnd, &windowRect); 

	HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);

	return (windowRect.left == monitorInfo.rcMonitor.left &&
			windowRect.top == monitorInfo.rcMonitor.top &&
			windowRect.right == monitorInfo.rcMonitor.right &&
			windowRect.bottom == monitorInfo.rcMonitor.bottom);
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("IS_FULLSCREEN", [](fx::ScriptContext& context)
	{
		bool result = false;
		auto hwnd = CoreGetGameWindow();
		bool isInFullScreen = IsWindowFullscreen(hwnd);
		
		if (hwnd)
		{
			if (isInFullScreen)
			{
				result = true;
			}
			else
			{
				result = false;
			}
		}
		
		context.SetResult<bool>(result);
	});
});
