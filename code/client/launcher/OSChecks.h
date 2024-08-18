#pragma once

#include <commctrl.h>
#include <shellapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "version.lib")

inline bool EnsureCompatibleOSVersion()
{
	if (IsWindows10OrGreater())
	{
		return true;
	}

	const wchar_t* title = L"Your operating system is out of support.";

	const wchar_t* content = L"In light of Microsoft’s end of support for Windows 7 and Windows 8 operating systems,"
							 L"\nRockstar Games is no longer officially supporting these operating systems since January 30, 2024."
							 L"\n\nPlease upgrade to a supported operating system."
							 L"\n\nFor more information, please read our "
							 L"<A HREF=\"https://support.rockstargames.com/articles/21494223343379\">support article</A>.";

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
