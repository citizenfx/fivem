#include "StdInc.h"

class CommandLineOption;

static CommandLineOption** g_curOption; //= *(CommandLineOption**)0x184A244;

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
		: name(name), description(description), value(0), next(nullptr)
	{
		if (IsRunningTests())
		{
			return;
		}

		next = *g_curOption;
		*g_curOption = this;
	}
};

static HMONITOR GetPrimaryMonitorHandle()
{
	const POINT ptZero = { 0, 0 };
	return MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
}

static CommandLineOption* g_noborder;

static HWND WINAPI CreateWindowExWCustom(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!_wcsicmp(lpClassName, L"grcWindow"))
	{
		if (!g_noborder->value)
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

static HWND WINAPI CreateWindowExACustom(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if (!strcmp(lpClassName, "grcWindow"))
	{
		if (!g_noborder->value)
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

	return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

static HookFunction windowInit([] ()
{
	hook::iat("user32.dll", CreateWindowExACustom, "CreateWindowExA");

	// 1.2.0.30: "8A 44 24 0C 88 41 10", 8
	g_curOption = *hook::get_pattern<CommandLineOption**>("8B 44 24 10 89 41 04 A1", 8);

	g_noborder = new CommandLineOption("border", "[GRAPHICS] Force disable borderless mode (needs -windowed)");

	// default to non-fullscreen mode
	//hook::put<uint8_t>(0x796BEB, 1);

	// don't unset width, height, fullscreen, refreshrate values during initialization
	hook::nop(hook::get_pattern("0F 45 C8 89 4C 24 40 68", 17), 10 * 5);
});
