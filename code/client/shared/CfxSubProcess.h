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
	outPath += L"FiveM_" + processType;

	DeleteFile(outPath.c_str());
	CopyFile(fxApplicationName, outPath.c_str(), FALSE);

	if (GetFileAttributes(outPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return va(L"%s", fxApplicationName);
	}

	// return the out path
	return va(L"%s", outPath);
}