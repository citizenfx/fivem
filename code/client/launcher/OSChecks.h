#pragma once

#include <commctrl.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "version.lib")

inline bool EnsureCompatibleOSVersion()
{
	if (IsWindows8OrGreater())
	{
		return true;
	}

	const wchar_t* title = L"Your Windows 7 PC is out of support";

	const wchar_t* content = L"As of January 14, 2020, support for Windows 7 has come to an end."
							 L"\n\nYour PC is more vulnerable to viruses and malware due to:"
							 L"\n- No security updates"
							 L"\n- No software updates"
							 L"\n- No tech support"
							 L"\n\nPlease upgrade to Windows 8.1 or higher as soon as possible."
							 // L"\n\nSee the <A HREF=\"https://fivem.net\">help article</A> for more info."
							 L"\n\nThe game will close now.";

	TASKDIALOGCONFIG taskDialogConfig = { 0 };
	taskDialogConfig.cbSize = sizeof(taskDialogConfig);
	taskDialogConfig.hInstance = GetModuleHandle(nullptr);
	taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS;
	taskDialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
	taskDialogConfig.pszWindowTitle = title;
	taskDialogConfig.pszMainIcon = TD_ERROR_ICON;
	taskDialogConfig.pszMainInstruction = title;
	taskDialogConfig.pszContent = content;

	taskDialogConfig.pfCallback = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LONG_PTR lpRefData) -> HRESULT
	{
		if (msg == TDN_HYPERLINK_CLICKED)
		{
			ShellExecuteW(NULL, L"open", (const wchar_t*)lParam, NULL, NULL, SW_SHOWNORMAL);

			DestroyWindow(hwnd);
		}

		return S_OK;
	};

	TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);

	return false;
}
