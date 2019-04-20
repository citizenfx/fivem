/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <CommCtrl.h>
#include <shobjidl.h>

#include <ShellScalingApi.h>

static class DPIScaler
{
public:
	DPIScaler()
	{
		// Default DPI is 96 (100%)
		dpiX = 96;
		dpiY = 96;
	}

	void SetScale(UINT dpiX, UINT dpiY)
	{
		this->dpiX = dpiX;
		this->dpiY = dpiY;
	}

	int ScaleX(int x)
	{
		return MulDiv(x, dpiX, 96);
	}

	int ScaleY(int y)
	{
		return MulDiv(y, dpiY, 96);
	}

private:
	UINT dpiX, dpiY;
} g_dpi;

static struct  
{
	HWND rootWindow;
	HWND topStatic;
	HWND bottomStatic;
	HWND progressBar;
	HWND cancelButton;

	UINT taskbarMsg;

	bool canceled;

	ITaskbarList3* tbList;

	wchar_t topText[512];
	wchar_t bottomText[512];
} g_uui;

HWND UI_GetWindowHandle()
{
	return g_uui.rootWindow;
}

HFONT UI_CreateScaledFont(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic,
	DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision,
	DWORD iQuality, DWORD iPitchAndFamily, LPCWSTR pszFaceName)
{
	LOGFONT logFont;
	
	memset(&logFont, 0, sizeof(LOGFONT));
	logFont.lfHeight = g_dpi.ScaleY(cHeight);
	logFont.lfWidth = cWidth;
	logFont.lfEscapement = cEscapement;
	logFont.lfOrientation = cOrientation;
	logFont.lfWeight = cWeight;
	logFont.lfItalic = bItalic;
	logFont.lfUnderline = bUnderline;
	logFont.lfStrikeOut = bStrikeOut;
	logFont.lfCharSet = iCharSet;
	logFont.lfOutPrecision = 8;
	logFont.lfClipPrecision = iClipPrecision;
	logFont.lfQuality = iQuality;
	logFont.lfPitchAndFamily = iPitchAndFamily;
	wcscpy_s(logFont.lfFaceName, pszFaceName);
	return CreateFontIndirect(&logFont);
}

void UI_CreateWindow()
{
	g_uui.taskbarMsg = RegisterWindowMessage(L"TaskbarButtonCreated");

	HWND rootWindow = CreateWindowEx(0, L"NotSteamAtAll", L"Updating " PRODUCT_NAME, 13238272 /* lol */, 0x80000000, 0, g_dpi.ScaleX(500), g_dpi.ScaleY(129), NULL, NULL, GetModuleHandle(NULL), 0);

	INITCOMMONCONTROLSEX controlSex;
	controlSex.dwSize = sizeof(controlSex);
	controlSex.dwICC = 16416; // lazy bum
	InitCommonControlsEx(&controlSex);

	HFONT font = UI_CreateScaledFont(-12, 0, 0, 0, 0, 0, 0, 0, 1, 8, 0, 5, 2, L"Tahoma");

	// TODO: figure out which static is placed where
	HWND static1 = CreateWindowEx(0x20, L"static", L"static1", 0x50000000, g_dpi.ScaleX(15), g_dpi.ScaleY(15), g_dpi.ScaleX(455), g_dpi.ScaleY(25), rootWindow, 0, GetModuleHandle(NULL) /* what?! */, 0);

	SendMessage(static1, WM_SETFONT, (WPARAM)font, 0);

	HWND cancelButton = CreateWindowEx(0, L"button", L"Cancel", 0x50000000, g_dpi.ScaleX(395), g_dpi.ScaleY(64), g_dpi.ScaleX(75), g_dpi.ScaleY(25), rootWindow, 0, GetModuleHandle(NULL), 0);
	SendMessage(cancelButton, WM_SETFONT, (WPARAM)font, 0);

	HWND progressBar = CreateWindowEx(0, L"msctls_progress32", 0, 0x50000000, g_dpi.ScaleX(15), g_dpi.ScaleY(40), g_dpi.ScaleX(455), g_dpi.ScaleY(15), rootWindow, 0, GetModuleHandle(NULL), 0);
	SendMessage(progressBar, PBM_SETRANGE32, 0, 10000);

	HWND static2 = CreateWindowEx(0x20, L"static", L"static2", 0x50000000, g_dpi.ScaleX(15), g_dpi.ScaleY(68), g_dpi.ScaleX(370), g_dpi.ScaleY(25), rootWindow, 0, GetModuleHandle(NULL) /* what?! */, 0);
	SendMessage(static2, WM_SETFONT, (WPARAM)font, 0);

	g_uui.cancelButton = cancelButton;
	g_uui.progressBar = progressBar;
	g_uui.topStatic = static1;
	g_uui.bottomStatic = static2;
	g_uui.rootWindow = rootWindow;

	RECT wndRect;
	wndRect.left = 0;
	wndRect.top = 0;
	wndRect.right = g_dpi.ScaleX(500);
	wndRect.bottom = g_dpi.ScaleY(139);

	HWND desktop = GetDesktopWindow();
	HDC dc = GetDC(desktop);
	int width = GetDeviceCaps(dc, 8);
	int height = GetDeviceCaps(dc, 10);

	ReleaseDC(desktop, dc);

	SetTimer(rootWindow, 0, 20, NULL);

	MoveWindow(rootWindow, (width - g_dpi.ScaleX(500)) / 2, (height - g_dpi.ScaleY(139)) / 2, wndRect.right - wndRect.left + 1, wndRect.bottom - wndRect.top + 1, TRUE);

	ShowWindow(rootWindow, TRUE);
}

LRESULT CALLBACK UI_WndProc(HWND hWnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	switch (uMsg)
	{
		case WM_NCCREATE:
			{
				// Only Windows 10+ supports EnableNonClientDpiScaling
				if (IsWindows10OrGreater())
				{
					HMODULE user32 = LoadLibrary(L"user32.dll");

					if (user32)
					{
						auto EnableNonClientDpiScaling = (decltype(&::EnableNonClientDpiScaling))GetProcAddress(user32, "EnableNonClientDpiScaling");

						if (EnableNonClientDpiScaling)
						{
							EnableNonClientDpiScaling(hWnd);
						}

						FreeLibrary(user32);
					}
				}

				return DefWindowProc(hWnd, uMsg, wparam, lparam);
			}
		
		case WM_CTLCOLORSTATIC:
			SetBkMode((HDC)wparam, TRANSPARENT);
			SetTextColor((HDC)wparam, COLORREF(GetSysColor(COLOR_WINDOWTEXT)));

			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		case WM_COMMAND:
			if ((HWND)lparam == g_uui.cancelButton)
			{
				g_uui.canceled = true;
			}

			break;
		case WM_TIMER:
			SetWindowText(g_uui.topStatic, g_uui.topText);
			SetWindowText(g_uui.bottomStatic, g_uui.bottomText);
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC dc = BeginPaint(hWnd, &ps);
			
				EndPaint(hWnd, &ps);
				break;
			}
		case WM_DPICHANGED:
			{
				// Set the new DPI
				g_dpi.SetScale(LOWORD(wparam), HIWORD(wparam));

				// Resize the window
				LPRECT newScale = (LPRECT)lparam;
				SetWindowPos(hWnd, HWND_TOP, newScale->left, newScale->top, newScale->right - newScale->left, newScale->bottom - newScale->top, SWP_NOZORDER | SWP_NOACTIVATE);

				// Recreate the font
				HFONT newFont = UI_CreateScaledFont(-12, 0, 0, 0, 0, 0, 0, 0, 1, 8, 0, 5, 2, L"Tahoma");

				// Resize all components
				SetWindowPos(g_uui.topStatic, HWND_TOP, g_dpi.ScaleX(15), g_dpi.ScaleY(15), g_dpi.ScaleX(455), g_dpi.ScaleY(25), SWP_SHOWWINDOW);
				SendMessage(g_uui.topStatic, WM_SETFONT, (WPARAM)newFont, 0);

				SetWindowPos(g_uui.cancelButton, HWND_TOP, g_dpi.ScaleX(395), g_dpi.ScaleY(64), g_dpi.ScaleX(75), g_dpi.ScaleY(25), SWP_SHOWWINDOW);
				SendMessage(g_uui.cancelButton, WM_SETFONT, (WPARAM)newFont, 0);

				SetWindowPos(g_uui.progressBar, HWND_TOP, g_dpi.ScaleX(15), g_dpi.ScaleY(40), g_dpi.ScaleX(455), g_dpi.ScaleY(15), SWP_SHOWWINDOW);

				SetWindowPos(g_uui.bottomStatic, HWND_TOP, g_dpi.ScaleX(15), g_dpi.ScaleY(68), g_dpi.ScaleX(370), g_dpi.ScaleY(25), SWP_SHOWWINDOW);
				SendMessage(g_uui.bottomStatic, WM_SETFONT, (WPARAM)newFont, 0);
				break;
			}
		default:
			if (uMsg == g_uui.taskbarMsg)
			{
				if (g_uui.tbList)
				{
					g_uui.tbList->SetProgressState(hWnd, TBPF_NORMAL);
					g_uui.tbList->SetProgressValue(hWnd, 0, 100);
				}
			}
			break;
	}

	return DefWindowProc(hWnd, uMsg, wparam, lparam);
}

void UI_RegisterClass()
{
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = 3;
	wndClass.lpfnWndProc = UI_WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	wndClass.hCursor = LoadCursor(NULL, (LPCWSTR)0x7F02);
	wndClass.hbrBackground = (HBRUSH)6;
	wndClass.lpszClassName = L"NotSteamAtAll";
	wndClass.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));

	RegisterClassEx(&wndClass);
}

void UI_DoCreation()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (IsWindows7OrGreater())
	{
		CoCreateInstance(CLSID_TaskbarList, 
			NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_uui.tbList));
	}

	// Only Windows 8.1+ supports per-monitor DPI awareness
	if (IsWindows8Point1OrGreater())
	{
		HMODULE shCore = LoadLibrary(L"shcore.dll");

		if (shCore)
		{
			auto GetDpiForMonitor = (decltype(&::GetDpiForMonitor))GetProcAddress(shCore, "GetDpiForMonitor");

			if (GetDpiForMonitor)
			{
				UINT dpiX, dpiY;

				POINT point;
				point.x = 1;
				point.y = 1;

				// Get DPI for the main monitor
				HMONITOR monitor = MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST);
				GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
				g_dpi.SetScale(dpiX, dpiY);
			}

			FreeLibrary(shCore);
		}
	}

	UI_RegisterClass();
	UI_CreateWindow();
}

void UI_DoDestruction()
{
	DestroyWindow(g_uui.rootWindow);
}

void UI_UpdateText(int textControl, const wchar_t* text)
{
	if (textControl == 0)
	{
		wcscpy(g_uui.topText, text);
	}
	else
	{
		wcscpy(g_uui.bottomText, text);
	}
}

void UI_UpdateProgress(double percentage)
{
	SendMessage(g_uui.progressBar, PBM_SETPOS, (int)(percentage * 100), 0);

	if (g_uui.tbList)
	{
		g_uui.tbList->SetProgressValue(g_uui.rootWindow, (int)percentage, 100);

		if (percentage == 100)
		{
			g_uui.tbList->SetProgressState(g_uui.rootWindow, TBPF_NOPROGRESS);
		}
	}
}

bool UI_IsCanceled()
{
	return g_uui.canceled;
}
