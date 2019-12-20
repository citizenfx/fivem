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
#include <shellapi.h>

#include <Error.h>

#include <LaunchMode.h>
#include <MinHook.h>

#include "Hooking.h"

bool CanSafelySkipLauncher()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\launcher_skip_mtl2").c_str(), L"rb");

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
		FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\launcher_skip_mtl2").c_str(), L"wb");

		if (f)
		{
			fclose(f);
		}
	}
	else
	{
		_wunlink(MakeRelativeCitPath(L"cache\\launcher_skip_mtl").c_str());
	}
}

static void Launcher_HandleArguments(boost::program_options::wcommand_line_parser& parser, std::function<void()> cb)
{
	boost::program_options::options_description desc;

	desc.add_options()
		("parent_pid", boost::program_options::value<int>()->default_value(-1), "")
		("cake", boost::program_options::wvalue<std::vector<std::wstring>>()->required(), "");

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
    auto args = map["cake"].as<std::vector<std::wstring>>();
    g_rosParentPid = map["parent_pid"].as<int>();

    VerifyOwnership(g_rosParentPid);

    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"CitizenFX_GTA5_ClearedForLaunch");

    if (hEvent != INVALID_HANDLE_VALUE)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}

void ValidateSteam(int parentPid);

static void Steam_Run(const boost::program_options::variables_map& map)
{
	auto args = map["cake"].as<std::vector<std::wstring>>();
	g_rosParentPid = map["parent_pid"].as<int>();

	ValidateSteam(g_rosParentPid);

	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"CitizenFX_GTA5_ClearedForLaunch");

	if (hEvent != INVALID_HANDLE_VALUE)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}

	TerminateProcess(GetCurrentProcess(), 0);
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

static bool IsLauncher(HWND hWnd)
{
	if (hWnd == g_launcherWindow)
	{
		return true;
	}

	wchar_t className[512];
	GetClassNameW(hWnd, className, std::size(className));

	return wcscmp(className, L"Rockstar Games Launcher") == 0;
}

static decltype(&CreateWindowExW) g_origCreateWindowExW;

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
	bool isThing = lpClassName && !IS_INTRESOURCE(lpClassName) && (wcscmp(lpClassName, L"LauncherWindowClass") == 0 || wcscmp(lpClassName, L"WindowWrapper") == 0 || wcscmp(lpClassName, L"Rockstar Games Launcher") == 0);

	if (isThing && CanSafelySkipLauncher())
	{
		dwStyle &= ~WS_VISIBLE;
	}

	if (CanSafelySkipLauncher())
	{
		dwStyle |= WS_DISABLED;
		dwExStyle |= WS_EX_NOACTIVATE;
	}

	auto hWnd = g_origCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	 
	if (isThing)
	{
		// set up a lazy wait for closing the window
		std::thread([hWnd]()
		{
			HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, L"CitizenFX_GTA5_ClearedForLaunch");
			WaitForSingleObject(hEvent, INFINITE);

			ShowWindow(g_launcherWindow, SW_HIDE);
		}).detach();

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
	if (IsLauncher(hwnd))
	{
		return TRUE;
	}

	return AnimateWindow(hwnd, dwTime, dwFlags);
}

BOOL WINAPI ShowWindowStub(_In_ HWND hWnd, _In_ int nCmdShow)
{
	if (IsLauncher(hWnd))
	{
		return TRUE;
	}

	return ShowWindow(hWnd, nCmdShow);
}

BOOL WINAPI SetWindowPosStub(_In_ HWND hWnd, _In_opt_ HWND hWndInsertAfter, _In_ int X, _In_ int Y, _In_ int cx, _In_ int cy, _In_ UINT uFlags)
{
	if (IsLauncher(hWnd))
	{
		return TRUE;
	}

	return SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

BOOL WINAPI Shell_NotifyIconWStub(
	DWORD            dwMessage,
	PNOTIFYICONDATAW lpData
)
{
	return TRUE;
}

#include "RSAKey.h"

FARPROC GetProcAddressStub(HMODULE hModule, LPCSTR name);

DWORD WINAPI GetModuleFileNameWStub(HMODULE hModule, LPWSTR lpFilename, DWORD nSize)
{
	if (hModule == GetModuleHandle(NULL) || !hModule)
	{
		wcscpy(lpFilename, L"C:\\Program Files\\Rockstar Games\\Launcher\\Launcher.exe");
		return wcslen(lpFilename);
	}

	return GetModuleFileNameW(hModule, lpFilename, nSize);
}

static void LogStuff(void*, const char* format, ...)
{
	char buf[1024];

	va_list ap;
	va_start(ap, format);
	_vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	trace("launcher say %s\n", buf);
}

static BOOL WINAPI SetForegroundWindowStub(_In_ HWND hWnd)
{
	return TRUE;
}

void DoLauncherUiSkip()
{
	MH_Initialize();
	MH_CreateHookApi(L"user32.dll", "CreateWindowExW", CreateWindowExWStub, (void**)&g_origCreateWindowExW);
	MH_CreateHookApi(L"user32.dll", "SetForegroundWindow", SetForegroundWindowStub, (void**)NULL);
	MH_CreateHookApi(L"user32.dll", "AllowSetForegroundWindow", SetForegroundWindowStub, (void**)NULL);
	MH_CreateHookApi(L"user32.dll", "SetActiveWindow", SetForegroundWindowStub, (void**)NULL);
	MH_CreateHookApi(L"user32.dll", "BringWindowToTop", SetForegroundWindowStub, (void**)NULL);

	if (CanSafelySkipLauncher())
	{
		hook::iat("user32.dll", AnimateWindowStub, "AnimateWindow");
		hook::iat("user32.dll", ShowWindowStub, "ShowWindow");
		hook::iat("user32.dll", SetWindowPosStub, "SetWindowPos");
		hook::iat("shell32.dll", Shell_NotifyIconWStub, "Shell_NotifyIconW");
	}

	MH_EnableHook(MH_ALL_HOOKS);
}

#include <minhook.h>
static void* (*opf)(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7);

static void* pf(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7)
{
	if (!strcmp((char*)a2, "payload") && *(char**)a3)
	{
		//*(int*)0 = 0xDEAD;
	}

	return opf(a1, a2, a3, a4, a5, a6, a7);
}

static HANDLE CreateMutexWStub(_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes, _In_ BOOL bInitialOwner, _In_opt_ LPCWSTR lpName)
{
	if (lpName && wcscmp(lpName, L"{EADA79DC-1FAF-4354-AB3E-23F30CBB7B8F}") == 0)
	{
		lpName = L"{EADA79DC-1FAF-4354-AB3E-23F30CBB7B8F}_fakeMTL";
	}

	return CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
}

static LONG WinVerifyTrustStub(HWND hwnd, GUID* pgActionID, LPVOID pWVTData)
{
	return 0;
}

static int ReturnFalse()
{
	return 0;
}

static void Launcher_Run(const boost::program_options::variables_map& map)
{
	// make firstrun.dat so the launcher won't whine/crash
	{
		CreateDirectoryW(MakeRelativeCitPath(L"cache\\game\\ros_launcher_appdata2").c_str(), NULL);
		FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\game\\ros_launcher_appdata2\\firstrun.dat").c_str(), L"wb");

		if (f)
		{
			fclose(f);
		}
	}

	auto args = map["cake"].as<std::vector<std::wstring>>();
	g_rosParentPid = map["parent_pid"].as<int>();

	boost::filesystem::path programPath(args[0]);

	auto parentPath = programPath.parent_path();
	SetCurrentDirectory(MakeRelativeCitPath(L"cache\\game\\launcher").c_str());

	trace("launcher! %s\n", GetCommandLineA());

	g_origProcess = programPath.wstring();
	ToolMode_SetPostLaunchRoutine([] ()
	{
		HMODULE scDll = LoadLibrary(L"C:\\Program Files\\Rockstar Games\\Social Club\\socialclub.dll");

		if (!scDll)
		{
			FatalError("Couldn't load SC SDK: Windows error code %d", GetLastError());
		}

		// rosdll
		HMODULE rosDll = LoadLibrary(L"ros.dll");

		if (rosDll != nullptr)
		{
			((void(*)(const wchar_t*))GetProcAddress(rosDll, "run"))(MakeRelativeCitPath(L"").c_str());
		}

		// wfsopen debug hook
		/*void* call = hook::pattern("49 8B 94 DE ? ? ? ? 44 8B C6 48 8B CD E8").count(1).get(0).get<void>(14);

		hook::set_call(&wfsopenOrig, call);
		hook::call(call, wfsopenCustom);*/

		auto pref = hook::get_pattern("8B C3 4D 85 C0  74 11 48 83 C8 FF 48 FF", -0xb);

		MH_Initialize();
		MH_CreateHook(pref, pf, (void**)&opf);
		MH_EnableHook(MH_ALL_HOOKS);

		hook::jump(hook::get_pattern("4C 89 44 24 18 4C 89 4C 24 20 C3"), LogStuff);

		hook::iat("version.dll", GetFileVersionInfoAStub, "GetFileVersionInfoA");

        hook::iat("user32.dll", LoadIconStub, "LoadIconA");
        hook::iat("user32.dll", LoadIconStub, "LoadIconW");

		hook::iat("kernel32.dll", CreateMutexWStub, "CreateMutexW");

		DoLauncherUiSkip();

		hook::iat("crypt32.dll", CertGetNameStringStub, "CertGetNameStringW");
		hook::iat("wintrust.dll", WinVerifyTrustStub, "WinVerifyTrust");

		hook::iat("kernel32.dll", GetProcAddressStub, "GetProcAddress");
		hook::iat("kernel32.dll", GetModuleFileNameWStub, "GetModuleFileNameW");

		HMODULE hSteam = LoadLibrary(L"C:\\Program Files\\Rockstar Games\\Launcher\\steam_api64.dll");

		if (hSteam)
		{
			MH_CreateHook(GetProcAddress(hSteam, "SteamAPI_Init"), ReturnFalse, NULL);
			MH_EnableHook(MH_ALL_HOOKS);
		}
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
static FxToolCommand rosSubprocess3("ros:steam", Launcher_HandleArguments, Steam_Run);

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

#include <winsock2.h>
#include <iphlpapi.h>

DWORD NotifyIpInterfaceChangeFake(_In_ ADDRESS_FAMILY Family, _In_ void* Callback, _In_opt_ PVOID CallerContext, _In_ BOOLEAN InitialNotification, _Inout_ HANDLE* NotificationHandle)
{
	*NotificationHandle = NULL;
	return NO_ERROR;
}

static InitFunction initFunctionF([]()
{
	// #TODORDR: this hangs on pre-20H1 Windows in a chain from WinVerifyTrust leading to an infinite wait??
	MH_Initialize();
	MH_CreateHookApi(L"iphlpapi.dll", "NotifyIpInterfaceChange", NotifyIpInterfaceChangeFake, NULL);
	MH_EnableHook(MH_ALL_HOOKS);
});

static HookFunction hookFunction([] ()
{
	if (!IsWindows7SP1OrGreater())
	{
		FatalError("Windows 7 SP1 or higher is required to run the FiveM ros:five component.");
	}

    hook::iat("user32.dll", LoadIconStub, "LoadIconA");
    hook::iat("user32.dll", LoadIconStub, "LoadIconW");

#ifdef GTA_FIVE
	// bypass the check routine for sky init
	void* skyInit = hook::pattern("48 8D A8 D8 FE FF FF 48 81 EC 10 02 00 00 41 BE").count(1).get(0).get<void>(-0x14);
	char* skyInitLoc = hook::pattern("EB 13 48 8D 0D ? ? ? ? 83 FB 08 74 07").count(2).get(0).get<char>(0x71);

	hook::call(skyInitLoc, skyInit);

	// same for distantlights
	void* distantLightInit = hook::pattern("48 8D 68 A1 48 81 EC F0 00 00 00 BE 01 00").count(1).get(0).get<void>(-0x10);

	hook::call(skyInitLoc + 0x30B, distantLightInit);
#endif
});
