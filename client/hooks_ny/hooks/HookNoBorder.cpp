#include "StdInc.h"

class CommandLineOption;

static CommandLineOption*& g_curOption = *(CommandLineOption**)0x184A244;

class CommandLineOption
{
public:
	const char* name;
	const char* description;
	union
	{
		uint64_t value;
	};
	CommandLineOption* next;

public:
	CommandLineOption(const char* name, const char* description)
		: name(name), description(description)
	{
		if (IsRunningTests())
		{
			return;
		}

		next = g_curOption;
		g_curOption = this;
	}
};

static HMONITOR GetPrimaryMonitorHandle()
{
	const POINT ptZero = { 0, 0 };
	return MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
}

static CommandLineOption g_noborder("border", "[GRAPHICS] Force disable borderless mode (needs -windowed)");

static HWND WINAPI CreateWindowExWCustom(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!_wcsicmp(lpClassName, L"grcWindow"))
	{
		if (!g_noborder.value)
		{
			MONITORINFO info;
			info.cbSize = sizeof(MONITORINFO);

			GetMonitorInfo(GetPrimaryMonitorHandle(), &info);

			dwStyle = WS_VISIBLE | WS_POPUP;

			X = 0;
			Y = 0;
			nWidth = info.rcMonitor.right;
			nHeight = info.rcMonitor.bottom;

			dwExStyle = 0;
		}
	}

	return CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

static HookFunction windowInit([] ()
{
	*(DWORD*)0xD4D3CC = (DWORD)CreateWindowExWCustom;

	// default to non-fullscreen mode
	//hook::put<uint8_t>(0x796BEB, 1);
});