#pragma once

#include <windows.h>
#include <string>

#include <Utils.h>

inline const wchar_t* MakeCfxSubProcess(const std::wstring& processType)
{
	// get the current EXE name
	wchar_t fxApplicationName[MAX_PATH];
	GetModuleFileName(GetModuleHandle(nullptr), fxApplicationName, _countof(fxApplicationName));

	// make the out directory
	std::wstring outPath = MakeRelativeCitPath(L"cache\\");
	CreateDirectory(outPath.c_str(), nullptr);

	outPath += L"subprocess\\";
	CreateDirectory(outPath.c_str(), nullptr);

	// copy the executable to the out directory
	std::wstring productName;

#ifdef IS_LAUNCHER
	productName = L"CGL_";
#elif defined(GTA_FIVE)
	productName = L"FiveM_";
#elif defined(IS_FXSERVER)
	productName = L"FXS_";
#elif defined(IS_RDR3)
	productName = L"RedM_";
#else
#error No subprocess name!
#endif

#ifndef IS_FXSERVER
	if (wcsstr(GetCommandLine(), L"cl2") != nullptr)
	{
		productName += L"cl2_";
	}
	
	if (wcsstr(GetCommandLine(), L"b1868") != nullptr)
	{
		productName += L"b1868_";
	}
#endif

	outPath += productName + processType;

	DeleteFile(outPath.c_str());
	CopyFile(fxApplicationName, outPath.c_str(), FALSE);

	if (GetFileAttributes(outPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return va(L"%s", fxApplicationName);
	}

	// return the out path
	return va(L"%s", outPath);
}
