#pragma once

#include <windows.h>
#include <string>

#include <CfxState.h>
#include <HostSharedData.h>

#include <Utils.h>

inline const wchar_t* MakeCfxSubProcess(const std::wstring& processType, const std::wstring& origin = L"")
{
	// get the current EXE name
	wchar_t fxApplicationName[MAX_PATH];
	static HostSharedData<CfxState> initState("CfxInitState");

	if (initState->gameExePath[0])
	{
		wcscpy_s(fxApplicationName, initState->gameExePath);
	}
	else
	{
		GetModuleFileName(GetModuleHandle(nullptr), fxApplicationName, _countof(fxApplicationName));
	}

	if (!origin.empty())
	{
		auto sub = MakeRelativeCitPath(fmt::sprintf(L"CitizenFX_SubProcess_%s.bin", origin));

		if (GetFileAttributesW(sub.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			wcscpy_s(fxApplicationName, sub.c_str());
		}
	}

	// make the out directory
	std::wstring outPath = MakeRelativeCitPath(L"data\\");
	CreateDirectory(outPath.c_str(), nullptr);

	outPath += L"cache\\";
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
#elif defined(GTA_NY)
	productName = L"LibertyM_";
#else
#error No subprocess name!
#endif

#ifndef IS_FXSERVER
	if (wcsstr(GetCommandLine(), L"cl2") != nullptr)
	{
		productName += L"cl2_";
	}

	if (wcsstr(GetCommandLine(), L"fxdk") != nullptr)
	{
		productName += L"fxdk_";
	}
	
#ifdef GTA_FIVE
	if (wcsstr(GetCommandLine(), L"b2372") != nullptr)
	{
		productName += L"b2372_";
	}

	if (wcsstr(GetCommandLine(), L"b2189") != nullptr)
	{
		productName += L"b2189_";
	}

	if (wcsstr(GetCommandLine(), L"b2060") != nullptr)
	{
		productName += L"b2060_";
	}

	if (wcsstr(GetCommandLine(), L"b372") != nullptr)
	{
		productName += L"b372_";
	}
#elif IS_RDR3
	if (wcsstr(GetCommandLine(), L"b1355") != nullptr)
	{
		productName += L"b1355_";
	}
#endif
#endif

	outPath += productName + processType;

	bool changed = false;

	WIN32_FILE_ATTRIBUTE_DATA outData = { 0 };
	if (!GetFileAttributesExW(outPath.c_str(), GetFileExInfoStandard, &outData))
	{
		changed = true;
	}
	else
	{
		WIN32_FILE_ATTRIBUTE_DATA inData = { 0 };
		if (!GetFileAttributesExW(fxApplicationName, GetFileExInfoStandard, &inData))
		{
			changed = true;
		}
		else
		{
			auto getFileTime = [](const FILETIME& time)
			{
				ULARGE_INTEGER uli = { 0 };
				uli.LowPart = time.dwLowDateTime;
				uli.HighPart = time.dwHighDateTime;

				return uli.QuadPart;
			};

			if (getFileTime(inData.ftLastWriteTime) > getFileTime(outData.ftLastWriteTime))
			{
				changed = true;
			}

			if (inData.nFileSizeLow != outData.nFileSizeLow || inData.nFileSizeHigh != outData.nFileSizeHigh)
			{
				changed = true;
			}
		}
	}

	if (changed)
	{
		if (!DeleteFile(outPath.c_str()))
		{
			auto oldPath = outPath + L".old";
			DeleteFile(oldPath.c_str());
			MoveFile(outPath.c_str(), oldPath.c_str());
		}

		CopyFile(fxApplicationName, outPath.c_str(), FALSE);
	}

	if (GetFileAttributes(outPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return va(L"%s", fxApplicationName);
	}

	// return the out path
	return va(L"%s", outPath);
}
