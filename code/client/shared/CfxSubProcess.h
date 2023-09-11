#pragma once

#include <windows.h>
#include <string>

#include <CfxState.h>
#include <HostSharedData.h>

#include <CrossBuildRuntime.h>

#include <Utils.h>

inline const wchar_t* MakeCfxSubProcess(const std::wstring& processType, const std::wstring& origin = L"")
{
	// get the current EXE name
	wchar_t fxApplicationName[MAX_PATH];
	auto initState = CfxState::Get();

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
	productName = L"DevotedStuios_";
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
	
	if ((origin.find(L"game") == 0 && origin != L"game_mtl") || processType == L"DumpServer")
	{
#if defined(GTA_FIVE) || defined(IS_RDR3)
		auto buildNumber = xbr::GetGameBuild();

		if (buildNumber != 1604 && buildNumber != 1311)
		{
			productName += fmt::sprintf(L"b%d_", buildNumber);
		}
#endif
#endif
	}

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
		if (processType != L"DumpServer")
		{
			__debugbreak();
		}

		return va(L"%s", fxApplicationName);
	}

	// return the out path
	return va(L"%s", outPath);
}
