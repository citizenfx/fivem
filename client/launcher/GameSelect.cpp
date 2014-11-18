#include "StdInc.h"
#include <ShlObj.h>

void EnsureGamePath()
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		wchar_t path[256];

		GetPrivateProfileString(L"Game", L"IVPath", L"", path, _countof(path), fpath.c_str());

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

	fileDialog->SetTitle(L"Select your GTA IV folder");

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
	result->GetDisplayName(SIGDN_FILESYSPATH, &resultPath);

	fileDialog->Release();

	result->Release();

	// check if there's an IV EXE in the path
	std::wstring ivPath = std::wstring(resultPath) + L"\\GTAIV.exe";

	if (GetFileAttributes(ivPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		std::wstring eflcPath = std::wstring(resultPath) + L"\\EFLC.exe";

		if (GetFileAttributes(eflcPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(nullptr, L"The selected path does not contain a GTAIV.exe file. As this is an EFLC installation, placing a GTAIV.exe (version 1.0.7.0) from any source will work as well.", L"CitizenFX:IV", MB_OK | MB_ICONWARNING);
		}
		else
		{
			MessageBox(nullptr, L"The selected path does not contain a GTAIV.exe file.", L"CitizenFX:IV", MB_OK | MB_ICONWARNING);
		}

		ExitProcess(0);
	}
	
	WritePrivateProfileString(L"Game", L"IVPath", resultPath, fpath.c_str());

	CoTaskMemFree(resultPath);
}