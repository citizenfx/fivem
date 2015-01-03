/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "SteamLoader.h"

void SteamLoader::Initialize()
{
	if (IsSteamRunning())
	{
		std::wstring steamDllPath = GetSteamDllPath();
		std::wstring steamDirectory = steamDllPath.substr(0, steamDllPath.rfind(L'\\'));

		// add Steam to the process path
		static wchar_t pathBuffer[65536];
		GetEnvironmentVariable(L"PATH", pathBuffer, sizeof(pathBuffer));

		wcscat(pathBuffer, L";");
		wcscat(pathBuffer, steamDirectory.c_str());

		SetEnvironmentVariable(L"PATH", pathBuffer);

		// load steamclient*.dll
		m_hSteamClient = LoadLibrary(steamDllPath.c_str());

		LoadGameOverlayRenderer(steamDllPath);
	}
}

CreateInterface_t SteamLoader::GetCreateInterfaceFunc()
{
	CreateInterface_t func = nullptr;

	if (m_hSteamClient)
	{
		func = (CreateInterface_t)GetProcAddress(m_hSteamClient, "CreateInterface");
	}

	return func;
}

bool SteamLoader::IsSteamRunning()
{
	bool retval = false;
	uint32_t pid = GetSteamProcessId();

	if (pid != 0)
	{
		HANDLE steamProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

		if (steamProcess != INVALID_HANDLE_VALUE)
		{
			CloseHandle(steamProcess);

			retval = true;
		}
	}

	return retval;
}

uint32_t SteamLoader::GetSteamProcessId()
{
	DWORD pid;
	DWORD pidSize = sizeof(pid);

	if (RegGetValue(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", L"pid", RRF_RT_REG_DWORD, nullptr, &pid, &pidSize) == ERROR_SUCCESS)
	{
		return pid;
	}

	return 0;
}

#ifndef _M_AMD64
#define STEAMCLIENT_DLL L"SteamClientDll"
#else
#define STEAMCLIENT_DLL L"SteamClientDll64"
#endif

std::wstring SteamLoader::GetSteamDllPath()
{
	wchar_t pathString[512];
	DWORD pathStringSize = sizeof(pathString);

	if (RegGetValue(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", STEAMCLIENT_DLL, RRF_RT_REG_SZ, nullptr, pathString, &pathStringSize) == ERROR_SUCCESS)
	{
		return pathString;
	}

	return L"";
}

#ifndef _M_AMD64
#define OVERLAYRENDERER_DLL L"GameOverlayRenderer.dll"
#else
#define OVERLAYRENDERER_DLL L"GameOverlayRenderer64.dll"
#endif

void SteamLoader::LoadGameOverlayRenderer(const std::wstring& baseDllPath)
{
	std::wstring overlayDllPath = baseDllPath.substr(0, baseDllPath.rfind(L'\\')) + L"\\" OVERLAYRENDERER_DLL;

	LoadLibrary(overlayDllPath.c_str());
}