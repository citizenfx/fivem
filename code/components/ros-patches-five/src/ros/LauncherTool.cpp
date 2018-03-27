/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ToolComponentHelpers.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <botan/auto_rng.h>
#include <botan/rsa.h>
#include <botan/sha160.h>
#include <botan/pubkey.h>

#include <shlobj.h>

#include <Error.h>

#include <LaunchMode.h>

#include "Hooking.h"

bool CanSafelySkipLauncher()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\launcher_skip").c_str(), L"rb");

	if (f)
	{
		fclose(f);

		return true;
	}

	return false;
}

void SetCanSafelySkipLauncher(bool value)
{
	if (value)
	{
		FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\launcher_skip").c_str(), L"wb");

		if (f)
		{
			fclose(f);
		}
	}
	else
	{
		_wunlink(MakeRelativeCitPath(L"cache\\launcher_skip").c_str());
	}
}

static void Launcher_HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()
		("parent_pid", boost::program_options::value<int>()->default_value(-1), "")
		("cake", boost::program_options::value<std::vector<std::string>>()->required(), "");

	boost::program_options::positional_options_description positional;
	positional.add("cake", -1);

	parser.options(desc).
		   positional(positional).
		   allow_unregistered();

	cb();
}

extern std::wstring g_origProcess;

extern int g_rosParentPid;

static FILE* (__cdecl* wfsopenOrig)(const wchar_t*, const wchar_t*, int);

static FILE* wfsopenCustom(const wchar_t* fileName, const wchar_t* mode, int shFlag)
{
	char fileNameNarrow[256];
	wcstombs(fileNameNarrow, fileName, sizeof(fileNameNarrow));

	FILE* f = wfsopenOrig(fileName, mode, shFlag);

	return f;
}

static int ReturnFalseStuff()
{
	return 0;
}

#pragma comment(lib, "version.lib")

BOOL WINAPI GetFileVersionInfoAStub(_In_ LPCSTR lptstrFilename, _Reserved_ DWORD dwHandle, _In_ DWORD dwLen, _Out_writes_bytes_(dwLen) LPVOID lpData)
{
	return GetFileVersionInfoA(lptstrFilename, dwHandle, dwLen, lpData);
}

static HICON hIcon;

static InitFunction iconFunction([] ()
{
	if (!CfxIsSinglePlayer())
	{
		hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(1));
	}
	else
	{
		hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(202));
	}
});

static HICON WINAPI LoadIconStub(HINSTANCE, LPCSTR)
{
    return hIcon;
}

void VerifyOwnership(int parentPid);

static void Legit_Run(const boost::program_options::variables_map& map)
{
    auto args = map["cake"].as<std::vector<std::string>>();
    g_rosParentPid = map["parent_pid"].as<int>();

    VerifyOwnership(g_rosParentPid);

    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"CitizenFX_GTA5_ClearedForLaunch");

    if (hEvent != INVALID_HANDLE_VALUE)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}

#include <wincrypt.h>

static DWORD WINAPI CertGetNameStringStub(_In_ PCCERT_CONTEXT pCertContext, _In_ DWORD dwType, _In_ DWORD dwFlags, _In_opt_ void *pvTypePara, _Out_writes_to_opt_(cchNameString, return) LPWSTR pszNameString, _In_ DWORD cchNameString)
{
	DWORD origSize = CertGetNameStringW(pCertContext, dwType, dwFlags, pvTypePara, nullptr, 0);
	std::vector<wchar_t> data(origSize);

	CertGetNameStringW(pCertContext, dwType, dwFlags, pvTypePara, data.data(), origSize);

	// get which name to replace
	const wchar_t* newName = nullptr;

	// add any names here

	// return if no such name
	if (newName == nullptr)
	{
		return CertGetNameStringW(pCertContext, dwType, dwFlags, pvTypePara, pszNameString, cchNameString);
	}
	
	if (pszNameString)
	{
		wcsncpy(pszNameString, newName, cchNameString);
	}

	return wcslen(newName) + 1;
}

static HWND g_launcherWindow;

static HWND WINAPI CreateWindowExWStub(_In_     DWORD     dwExStyle,
	_In_opt_ LPCWSTR   lpClassName,
	_In_opt_ LPCWSTR   lpWindowName,
	_In_     DWORD     dwStyle,
	_In_     int       x,
	_In_     int       y,
	_In_     int       nWidth,
	_In_     int       nHeight,
	_In_opt_ HWND      hWndParent,
	_In_opt_ HMENU     hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID    lpParam)
{
	auto hWnd = CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	if (lpClassName && wcscmp(lpClassName, L"LauncherWindowClass") == 0)
	{
		g_launcherWindow = hWnd;
	}

	return hWnd;
}

BOOL WINAPI AnimateWindowStub(
	_In_ HWND  hwnd,
	_In_ DWORD dwTime,
	_In_ DWORD dwFlags
)
{
	if (hwnd == g_launcherWindow)
	{
		return TRUE;
	}

	return AnimateWindow(hwnd, dwTime, dwFlags);
}

#include "RSAKey.h"

static void Launcher_Run(const boost::program_options::variables_map& map)
{
	auto args = map["cake"].as<std::vector<std::string>>();
	g_rosParentPid = map["parent_pid"].as<int>();

	boost::filesystem::path programPath(args[0]);

	auto parentPath = programPath.parent_path();
	SetCurrentDirectory(parentPath.wstring().c_str());

	trace("launcher! %s\n", GetCommandLineA());

	g_origProcess = programPath.wstring();
	ToolMode_SetPostLaunchRoutine([] ()
	{
		assert(LoadLibrary(L"C:\\program files\\rockstar games\\social club\\socialclub.dll") != nullptr);

		// rosdll
		HMODULE rosDll = LoadLibrary(L"ros.dll");

		if (rosDll != nullptr)
		{
			((void(*)(const wchar_t*))GetProcAddress(rosDll, "run"))(MakeRelativeCitPath(L"").c_str());
		}

		// wfsopen debug hook
		void* call = hook::pattern("49 8B 94 DE ? ? ? ? 44 8B C6 48 8B CD E8").count(1).get(0).get<void>(14);

		hook::set_call(&wfsopenOrig, call);
		hook::call(call, wfsopenCustom);

		hook::iat("version.dll", GetFileVersionInfoAStub, "GetFileVersionInfoA");

        hook::iat("user32.dll", LoadIconStub, "LoadIconA");
        hook::iat("user32.dll", LoadIconStub, "LoadIconW");

		if (CanSafelySkipLauncher())
		{
			hook::iat("user32.dll", CreateWindowExWStub, "CreateWindowExW");
			hook::iat("user32.dll", AnimateWindowStub, "AnimateWindow");
		}

		hook::iat("crypt32.dll", CertGetNameStringStub, "CertGetNameStringW");
	});

	// delete in- files (these being present will trigger safe mode, and the function can't be hooked due to hook checks)
	{
		PWSTR localAppData = nullptr;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppData);

		boost::filesystem::path appDataPath(localAppData);
		appDataPath += L"\\Rockstar Games\\GTA V\\";

		boost::system::error_code ec;

		if (boost::filesystem::exists(appDataPath, ec))
		{
			for (boost::filesystem::directory_iterator it(appDataPath); it != boost::filesystem::directory_iterator(); it++)
			{
				auto path = it->path();

				if (path.filename().string().find("in-", 0) == 0)
				{
					boost::filesystem::remove(path, ec);
				}
			}

			boost::filesystem::remove(appDataPath.append("launcher.telem"), ec);
		}

		CoTaskMemFree(localAppData);
	}

	ToolMode_LaunchGame(programPath.wstring().c_str());
}

static FxToolCommand rosSubprocess("ros:launcher", Launcher_HandleArguments, Launcher_Run);
static FxToolCommand rosSubprocess2("ros:legit", Launcher_HandleArguments, Legit_Run);

void RunLauncher(const wchar_t* toolName, bool instantWait);

#include "ResumeComponent.h"

void WaitForLauncher();
bool LoadOwnershipTicket();

static InitFunction initFunction([] ()
{
	if (getenv("CitizenFX_ToolMode") == nullptr || getenv("CitizenFX_ToolMode")[0] == 0)
	{
		if (OpenMutex(SYNCHRONIZE, FALSE, L"CitizenFX_LauncherMutex") == nullptr)
		{
			// create the mutex
			CreateMutex(nullptr, TRUE, L"CitizenFX_LauncherMutex");

            if (!LoadOwnershipTicket())
            {
                RunLauncher(L"ros:legit", true);
            }

			if (!LoadOwnershipTicket())
			{
				TerminateProcess(GetCurrentProcess(), 0x8000DEAD);
			}

			RunLauncher(L"ros:launcher", false);
		}

		if (!LoadOwnershipTicket())
		{
			TerminateProcess(GetCurrentProcess(), 0x8000DEAF);
		}

		OnResumeGame.Connect([] ()
		{
			if (!CanSafelySkipLauncher())
			{
				WaitForLauncher();

				SetCanSafelySkipLauncher(true);
			}
		});
	}
});

static HookFunction hookFunction([] ()
{
	if (!IsWindows7SP1OrGreater())
	{
		FatalError("Windows 7 SP1 or higher is required to run the FiveM ros:five component.");
	}

    hook::iat("user32.dll", LoadIconStub, "LoadIconA");
    hook::iat("user32.dll", LoadIconStub, "LoadIconW");

	// bypass the check routine for sky init
	void* skyInit = hook::pattern("48 8D A8 D8 FE FF FF 48 81 EC 10 02 00 00 41 BE").count(1).get(0).get<void>(-0x14);
	char* skyInitLoc = hook::pattern("EB 13 48 8D 0D ? ? ? ? 83 FB 08 74 07").count(2).get(0).get<char>(0x71);

	hook::call(skyInitLoc, skyInit);

	// same for distantlights
	void* distantLightInit = hook::pattern("48 8D 68 A1 48 81 EC F0 00 00 00 BE 01 00").count(1).get(0).get<void>(-0x10);

	hook::call(skyInitLoc + 0x30B, distantLightInit);
});
