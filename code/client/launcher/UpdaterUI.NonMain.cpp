#include "StdInc.h"

#ifndef LAUNCHER_PERSONALITY_MAIN
#include <optional>

void UI_DoCreation(bool safeMode)
{
}
void UI_DoDestruction()
{
}
void UI_UpdateText(int textControl, const wchar_t* text)
{
}
void UI_UpdateProgress(double percentage)
{
}
bool UI_IsCanceled()
{
	return true;
}
HWND UI_GetWindowHandle()
{
	return NULL;
}
std::unique_ptr<TenUIBase> UI_InitTen()
{
	return {};
}

#ifndef _M_IX86
extern "C" DLL_EXPORT HRESULT __stdcall DllCanUnloadNow()
{
	return S_OK;
}
#endif

void UI_DestroyTen()
{
}

std::optional<int> EnsureGamePath()
{
	return {};
}

#pragma comment(lib, "delayimp.lib")

//
#endif
