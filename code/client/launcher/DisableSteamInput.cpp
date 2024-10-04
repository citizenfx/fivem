#include <StdInc.h>

#include <psapi.h>
#include <shellapi.h>
#include <shlwapi.h>

#ifdef LAUNCHER_PERSONALITY_MAIN
uint32_t GetSteamProcessId()
{
	DWORD pid;
	DWORD pidSize = sizeof(pid);

	if (RegGetValue(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", L"pid", RRF_RT_REG_DWORD, nullptr, &pid, &pidSize) == ERROR_SUCCESS)
	{
		return pid;
	}

	return 0;
}

bool IsSteamRunning()
{
	bool retval = false;
	uint32_t pid = GetSteamProcessId();

	if (pid != 0)
	{
		HANDLE steamProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

		if (steamProcess != INVALID_HANDLE_VALUE)
		{
			wchar_t imageName[512] = { 0 };
			GetProcessImageFileNameW(steamProcess, imageName, std::size(imageName));

			if (StrStrIW(imageName, L"steam"))
			{
				retval = true;
			}
			
			CloseHandle(steamProcess);
		}
	}

	return retval;
}

static void ForceInputAppId(int appId)
{
	SHELLEXECUTEINFOW shinfo = { 0 };
	shinfo.cbSize = sizeof(SHELLEXECUTEINFO);
	shinfo.fMask = SEE_MASK_ASYNCOK;
	shinfo.hwnd = NULL;
	shinfo.lpVerb = NULL;
	shinfo.lpFile = va(L"steam://forceinputappid/%d", appId);
	shinfo.lpParameters = NULL;
	shinfo.lpDirectory = NULL;
	shinfo.nShow = SW_HIDE;
	shinfo.hInstApp = NULL;

	ShellExecuteEx(&shinfo);
}

void SteamInput_Initialize()
{
	if (IsSteamRunning())
	{
		ForceInputAppId(
#ifdef GTA_FIVE
			271590
#elif defined(IS_RDR3)
			1174180
#else
			0
#endif
		);
	}
}

void SteamInput_Reset()
{
	if (IsSteamRunning())
	{
		ForceInputAppId(0);
	}
}
#endif
