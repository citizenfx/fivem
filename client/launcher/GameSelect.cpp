/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ShlObj.h>

#include <wrl.h>

namespace WRL = Microsoft::WRL;

struct ScopedCoInitialize
{
	template<typename... TArg>
	ScopedCoInitialize(const TArg&&... args) : m_hr(CoInitializeEx(nullptr, args...))
	{
	}

	~ScopedCoInitialize()
	{
		if (SUCCEEDED(m_hr))
		{
			CoUninitialize();
		}
	}

	inline operator bool()
	{
		return (SUCCEEDED(m_hr));
	}

	inline HRESULT GetResult()
	{
		return m_hr;
	}

private:
	HRESULT m_hr;
};

void EnsureGamePath()
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	const wchar_t* pathKey = L"IVPath";

	if (wcsstr(GetCommandLine(), L"cl2"))
	{
		pathKey = L"PathCL2";
	}

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		wchar_t path[256];

		GetPrivateProfileString(L"Game", pathKey, L"", path, _countof(path), fpath.c_str());

		if (path[0] != L'\0')
		{
			return;
		}
	}

	ScopedCoInitialize coInit(COINIT_APARTMENTTHREADED);

	if (!coInit)
	{
		MessageBox(nullptr, va(L"CoInitializeEx failed. HRESULT = 0x%08x.", coInit.GetResult()), L"Error", MB_OK | MB_ICONERROR);

		ExitProcess(coInit.GetResult());
	}

	WRL::ComPtr<IFileDialog> fileDialog;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileDialog, (void**)fileDialog.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBox(nullptr, va(L"CoCreateInstance(IFileDialog) failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);

		ExitProcess(hr);
	}

	FILEOPENDIALOGOPTIONS opts;
	fileDialog->GetOptions(&opts);

	opts |= FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS;

	fileDialog->SetOptions(opts);

#ifndef GTA_FIVE
	fileDialog->SetTitle(L"Select the folder containing " GAME_EXECUTABLE);
#else
	fileDialog->SetTitle(L"Select the folder containing Grand Theft Auto V");

	// set the default folder, if we can find one
	{
		wchar_t gameRoot[1024];
		DWORD gameRootLength = sizeof(gameRoot);

		if (RegGetValue(HKEY_LOCAL_MACHINE,
			L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Grand Theft Auto V", L"InstallFolder",
			RRF_RT_REG_SZ, nullptr, gameRoot, &gameRootLength) == ERROR_SUCCESS)
		{
			WRL::ComPtr<IShellItem> item;

			if (SUCCEEDED(SHCreateItemFromParsingName(gameRoot, nullptr, IID_IShellItem, (void**)item.GetAddressOf())))
			{
				fileDialog->SetFolder(item.Get());
			}
		}
	}
#endif

	hr = fileDialog->Show(nullptr);

	if (FAILED(hr))
	{
		if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
		{
			MessageBox(nullptr, va(L"Could not show game folder selection window: IFileDialog::Show failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		}

		ExitProcess(0);
	}

	WRL::ComPtr<IShellItem> result;
	hr = fileDialog->GetResult(result.GetAddressOf());

	if (!result)
	{
		MessageBox(nullptr, va(L"You did not select a game folder: IFileDialog::GetResult failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	PWSTR resultPath;

	if (FAILED(hr = result->GetDisplayName(SIGDN_FILESYSPATH, &resultPath)))
	{
		MessageBox(nullptr, va(L"Could not get game directory: IShellItem::GetDisplayName failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	// check if there's a game EXE in the path
	std::wstring gamePath = std::wstring(resultPath) + L"\\" GAME_EXECUTABLE;

	if (GetFileAttributes(gamePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
#if defined(GTA_NY)
		std::wstring eflcPath = std::wstring(resultPath) + L"\\EFLC.exe";

		if (GetFileAttributes(eflcPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(nullptr, L"The selected path does not contain a GTAIV.exe file. As this is an EFLC installation, placing a GTAIV.exe (version 1.0.7.0) from any source will work as well.", PRODUCT_NAME, MB_OK | MB_ICONWARNING);
		}
		else
#endif
		{
			MessageBox(nullptr, L"The selected path does not contain a " GAME_EXECUTABLE L" file.", PRODUCT_NAME, MB_OK | MB_ICONWARNING);
		}

		ExitProcess(0);
	}
	
	WritePrivateProfileString(L"Game", pathKey, resultPath, fpath.c_str());

	CoTaskMemFree(resultPath);

	CoUninitialize();
}