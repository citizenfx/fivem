/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ShlObj.h>

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

	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	IFileDialog* fileDialog;
	CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog));

	FILEOPENDIALOGOPTIONS opts;
	fileDialog->GetOptions(&opts);

	opts |= FOS_FORCEFILESYSTEM | FOS_PICKFOLDERS;

	fileDialog->SetOptions(opts);

	fileDialog->SetTitle(L"Select the folder containing " GAME_EXECUTABLE);

	HRESULT hr = fileDialog->Show(nullptr);

	if (FAILED(hr))
	{
		MessageBox(nullptr, va(L"IFileDialog::Show failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	IShellItem* result = nullptr;
	hr = fileDialog->GetResult(&result);

	if (!result)
	{
		MessageBox(nullptr, va(L"IFileDialog::GetResult failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	PWSTR resultPath;

	if (FAILED(hr = result->GetDisplayName(SIGDN_FILESYSPATH, &resultPath)))
	{
		MessageBox(nullptr, va(L"IShellItem::GetDisplayName failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		ExitProcess(0);
	}

	fileDialog->Release();

	result->Release();

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
}