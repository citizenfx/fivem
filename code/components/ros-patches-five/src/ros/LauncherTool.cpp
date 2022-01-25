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

#include <CL2LaunchMode.h>
#include <LaunchMode.h>
#include <MinHook.h>

#include <ROSSuffix.h>
#include <CrossBuildRuntime.h>

#include "Hooking.h"
#include "Hooking.Aux.h"

bool CanSafelySkipLauncher()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\launcher_skip_mtl2").c_str(), L"rb");

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
		FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\launcher_skip_mtl2").c_str(), L"wb");

		if (f)
		{
			fclose(f);
		}
	}
	else
	{
		_wunlink(MakeRelativeCitPath(L"data\\cache\\launcher_skip_mtl2").c_str());
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

	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"CitizenFX_GTA5_ClearedForLaunch_Steam");

	if (hEvent != INVALID_HANDLE_VALUE)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}

	TerminateProcess(GetCurrentProcess(), 0);
}

void ValidateEpic(int parentPid);

static void Epic_Run(const boost::program_options::variables_map& map)
{
	auto args = map["cake"].as<std::vector<std::wstring>>();
	g_rosParentPid = map["parent_pid"].as<int>();

	ValidateEpic(g_rosParentPid);
	TerminateProcess(GetCurrentProcess(), 0);
}

#include <wincrypt.h>

static HLOCAL WINAPI LocalFreeStub(HLOCAL hMem)
{
	if (hMem && strstr((char*)hMem, "Entrust "))
	{
		return NULL;
	}

	return LocalFree(hMem);
}

static DWORD WINAPI CertGetNameStringStubW(_In_ PCCERT_CONTEXT pCertContext, _In_ DWORD dwType, _In_ DWORD dwFlags, _In_opt_ void *pvTypePara, _Out_writes_to_opt_(cchNameString, return) LPWSTR pszNameString, _In_ DWORD cchNameString)
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

static DWORD WINAPI CertGetNameStringStubA(_In_ PCCERT_CONTEXT pCertContext, _In_ DWORD dwType, _In_ DWORD dwFlags, _In_opt_ void* pvTypePara, _Out_writes_to_opt_(cchNameString, return) LPSTR pszNameString, _In_ DWORD cchNameString)
{
	DWORD origSize = CertGetNameStringA(pCertContext, dwType, dwFlags, pvTypePara, nullptr, 0);
	std::vector<char> data(origSize);

	CertGetNameStringA(pCertContext, dwType, dwFlags, pvTypePara, data.data(), origSize);

	// get which name to replace
	const char* newName = nullptr;

	auto certString = std::string{ data.data() };
	if (certString == "DigiCert SHA2 Assured ID Code Signing CA")
	{
		newName = "Entrust Code Signing CA - OVCS1";
	}
	else if (xbr::IsGameBuild<372>() && certString == "Entrust Code Signing CA - OVCS1")
	{
		newName = "Entrust Code Signing Certification Authority - L1D";
	}
	else if (xbr::IsGameBuild<372>() && certString == "Rockstar Games, Inc.")
	{
		newName = "Take-Two Interactive Software, Inc.";
	}

	// return if no such name
	if (newName == nullptr)
	{
		return CertGetNameStringA(pCertContext, dwType, dwFlags, pvTypePara, pszNameString, cchNameString);
	}

	if (pszNameString)
	{
		strncpy(pszNameString, newName, cchNameString);
	}

	return strlen(newName) + 1;
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

FARPROC _stdcall GetProcAddressStub(HMODULE hModule, LPCSTR name);

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

static std::vector<std::tuple<const char*, void*, const char*>>* g_refLauncherHooks;

void DoLauncherUiSkip()
{
	DisableToolHelpScope scope;

	MH_Initialize();
	MH_CreateHookApi(L"user32.dll", "CreateWindowExW", CreateWindowExWStub, (void**)&g_origCreateWindowExW);
	MH_CreateHookApi(L"user32.dll", "SetForegroundWindow", SetForegroundWindowStub, (void**)NULL);
	MH_CreateHookApi(L"user32.dll", "AllowSetForegroundWindow", SetForegroundWindowStub, (void**)NULL);
	MH_CreateHookApi(L"user32.dll", "SetActiveWindow", SetForegroundWindowStub, (void**)NULL);
	MH_CreateHookApi(L"user32.dll", "BringWindowToTop", SetForegroundWindowStub, (void**)NULL);

	if (CanSafelySkipLauncher())
	{
		g_refLauncherHooks->emplace_back("user32.dll", AnimateWindowStub, "AnimateWindow");
		g_refLauncherHooks->emplace_back("user32.dll", ShowWindowStub, "ShowWindow");
		g_refLauncherHooks->emplace_back("user32.dll", SetWindowPosStub, "SetWindowPos");
		g_refLauncherHooks->emplace_back("shell32.dll", Shell_NotifyIconWStub, "Shell_NotifyIconW");
	}

	MH_EnableHook(MH_ALL_HOOKS);
}

#include <minhook.h>

static HANDLE CreateMutexWStub(_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes, _In_ BOOL bInitialOwner, _In_opt_ LPCWSTR lpName)
{
	if (lpName && wcscmp(lpName, L"{EADA79DC-1FAF-4354-AB3E-23F30CBB7B8F}") == 0)
	{
		lpName = va(L"{EADA79DC-1FAF-4354-AB3E-23F30CBB7B8F}_fakeMTL%s", IsCL2() ? L"CL2" : L"");
	}

	return CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
}

LONG __stdcall WinVerifyTrustStub(HWND hwnd, GUID* pgActionID, LPVOID pWVTData)
{
	return 0;
}

static int ReturnFalse()
{
	return 0;
}

static BOOL WINAPI GetExitCodeProcessStub(_In_ HANDLE hProcess, _Out_ LPDWORD lpExitCode)
{
	if (!GetExitCodeProcess(hProcess, lpExitCode))
	{
		*lpExitCode = 0;
	}

	return TRUE;
}

static BOOL WINAPI ShellExecuteExWStub(_Inout_ SHELLEXECUTEINFOW *pExecInfo)
{
	if (pExecInfo->lpFile && wcsstr(pExecInfo->lpFile, L"RockstarService"))
	{
		// setting SEE_MASK_FLAG_NO_UI bypasses some slow stuff
		pExecInfo->fMask |= SEE_MASK_FLAG_NO_UI;

		if (CfxIsWine())
		{
			STARTUPINFOW siw = { sizeof(STARTUPINFOW) };
			PROCESS_INFORMATION pi = { 0 };
			CreateProcessW(NULL, const_cast<wchar_t*>(va(L"\"%s\" %s", pExecInfo->lpFile, pExecInfo->lpParameters)), NULL, NULL, FALSE, 0, NULL, pExecInfo->lpDirectory, &siw, &pi);

			pExecInfo->hProcess = pi.hProcess;
			CloseHandle(pi.hThread);
			
			return TRUE;
		}

		return ShellExecuteExW(pExecInfo);
	}

	if (pExecInfo->lpFile)
	{
		trace("Restricting MTL ShellExecuteExW: %s\n", ToNarrow(pExecInfo->lpFile));
	}

	return TRUE;
}

HINSTANCE WINAPI ShellExecuteWStub(_In_opt_ HWND hwnd, _In_opt_ LPCWSTR lpOperation, _In_ LPCWSTR lpFile, _In_opt_ LPCWSTR lpParameters, _In_opt_ LPCWSTR lpDirectory, _In_ INT nShowCmd)
{
	if (lpFile)
	{
		trace("Restricting MTL ShellExecuteW: %s\n", ToNarrow(lpFile));
	}

	return NULL;
}

static BOOL WINAPI GetComputerNameExWStub(COMPUTER_NAME_FORMAT NameType, LPWSTR lpBuffer, LPDWORD nSize)
{
	return GetComputerNameW(lpBuffer, nSize);
}

#include <shlwapi.h>
#include <tlhelp32.h>

static BOOL WINAPI Process32NextWHook(HANDLE hSnapshot, LPPROCESSENTRY32W lppe)
{
	auto rv = Process32NextW(hSnapshot, lppe);

	if (rv)
	{
		while (rv &&
			(StrStrIW(lppe->szExeFile, L"GTA5.exe") ||
			 StrStrIW(lppe->szExeFile, L"RDR2.exe") ||
			 StrStrIW(lppe->szExeFile, L"FiveM") ||
			 StrStrIW(lppe->szExeFile, L"RedM")))
		{
			rv = Process32NextW(hSnapshot, lppe);
		}
	}

	return rv;
}

static DWORD WINAPI GetLogicalDrivesStub()
{
	wchar_t windir[MAX_PATH];
	GetWindowsDirectoryW(windir, std::size(windir));

	return (1 << DWORD(windir[0] - 'A')) | (1 << DWORD('C' - 'A'));
}

static DWORD WINAPI GetLogicalDriveStringsWStub(DWORD nBufferLength, LPWSTR lpBuffer)
{
	wchar_t windir[MAX_PATH];
	GetWindowsDirectoryW(windir, std::size(windir));

	if (toupper(windir[0]) == 'C')
	{
		if (nBufferLength && lpBuffer)
		{
			lpBuffer[0] = windir[0];
			lpBuffer[1] = windir[1];
			lpBuffer[2] = windir[2];
			lpBuffer[3] = L'\0';
			lpBuffer[4] = L'\0';
		}
		
		return 4;
	}

	if (nBufferLength && lpBuffer)
	{
		lpBuffer[0] = L'C';
		lpBuffer[1] = L':';
		lpBuffer[2] = L'\\';
		lpBuffer[3] = L'\0';
		lpBuffer[4] = windir[0];
		lpBuffer[5] = windir[1];
		lpBuffer[6] = windir[2];
		lpBuffer[7] = L'\0';
		lpBuffer[8] = L'\0';
	}

	return 8;
}

extern HRESULT WINAPI __stdcall CoCreateInstanceStub(_In_ REFCLSID rclsid, _In_opt_ LPUNKNOWN pUnkOuter, _In_ DWORD dwClsContext, _In_ REFIID riid, _COM_Outptr_ _At_(*ppv, _Post_readable_size_(_Inexpressible_(varies))) LPVOID FAR* ppv);
extern BOOL WINAPI __stdcall CreateProcessAStub(_In_opt_ LPCSTR lpApplicationName, _Inout_opt_ LPSTR lpCommandLine, _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes, _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment, _In_opt_ LPCSTR lpCurrentDirectory, _In_ LPSTARTUPINFOA lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation);
extern BOOL WINAPI __stdcall CreateProcessWStub(_In_opt_ LPCWSTR lpApplicationName, _Inout_opt_ LPWSTR lpCommandLine, _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes, _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ BOOL bInheritHandles, _In_ DWORD dwCreationFlags, _In_opt_ LPVOID lpEnvironment, _In_opt_ LPCWSTR lpCurrentDirectory, _In_ LPSTARTUPINFOW lpStartupInfo, _Out_ LPPROCESS_INFORMATION lpProcessInformation);
HANDLE WINAPI __stdcall CreateNamedPipeAHookL(_In_ LPCSTR lpName, _In_ DWORD dwOpenMode, _In_ DWORD dwPipeMode, _In_ DWORD nMaxInstances, _In_ DWORD nOutBufferSize, _In_ DWORD nInBufferSize, _In_ DWORD nDefaultTimeOut, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes);

static std::vector<std::tuple<const char*, void*, const char*>> g_launcherHooks = {
	{ "version.dll", GetFileVersionInfoAStub, "GetFileVersionInfoA" },

	{ "user32.dll", LoadIconStub, "LoadIconA" },
	{ "user32.dll", LoadIconStub, "LoadIconW" },

	{ "kernel32.dll", CreateMutexWStub, "CreateMutexW" },
	{ "kernel32.dll", CreateNamedPipeAHookL, "CreateNamedPipeA" },

	{ "kernel32.dll", Process32NextWHook, "Process32NextW" },

	{ "shell32.dll", ShellExecuteExWStub, "ShellExecuteExW" },
	{ "shell32.dll", ShellExecuteWStub, "ShellExecuteW" },

	{ "kernel32.dll", GetExitCodeProcessStub, "GetExitCodeProcess" },

	{ "crypt32.dll", CertGetNameStringStubW, "CertGetNameStringW" },
	{ "crypt32.dll", CertGetNameStringStubA, "CertGetNameStringA" },
	{ "wintrust.dll", WinVerifyTrustStub, "WinVerifyTrust" },

	{ "kernel32.dll", GetModuleFileNameWStub, "GetModuleFileNameW" },

	{ "ole32.dll", CoCreateInstanceStub, "CoCreateInstance" },
	{ "kernel32.dll", CreateProcessAStub, "CreateProcessA" },
	{ "kernel32.dll", CreateProcessWStub, "CreateProcessW" },
};

static FARPROC GetProcAddressHook(HMODULE hModule, LPCSTR funcName)
{
	if (!IS_INTRESOURCE(funcName))
	{
		for (const auto& h : g_launcherHooks)
		{
			if (strcmp(std::get<2>(h), funcName) == 0)
			{
				return (FARPROC)std::get<1>(h);
			}
		}
	}

	return GetProcAddressStub(hModule, funcName);
}

static void Launcher_Run(const boost::program_options::variables_map& map)
{
	// make firstrun.dat so the launcher won't error out/crash
	{
		CreateDirectoryW(MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_appdata" ROS_SUFFIX_W).c_str(), NULL);
		FILE* f = _wfopen(MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_appdata" ROS_SUFFIX_W L"\\firstrun.dat").c_str(), L"wb");

		if (f)
		{
			fclose(f);
		}
	}

	auto args = map["cake"].as<std::vector<std::wstring>>();
	g_rosParentPid = map["parent_pid"].as<int>();

	boost::filesystem::path programPath(args[0]);

	auto parentPath = programPath.parent_path();
	SetCurrentDirectory(MakeRelativeCitPath(L"data\\game-storage\\launcher").c_str());

	trace("launcher! %s\n", GetCommandLineA());

	g_origProcess = programPath.wstring();
	ToolMode_SetPostLaunchRoutine([] ()
	{
#if _M_IX86
		HMODULE scDll = LoadLibrary(L"C:\\Program Files\\Rockstar Games (x86)\\Social Club\\socialclub.dll");
#else
		HMODULE scDll = LoadLibrary(L"C:\\Program Files\\Rockstar Games\\Social Club\\socialclub.dll");
#endif

		if (!scDll)
		{
			FatalError("Couldn't load SC SDK: Windows error code %d", GetLastError());
		}

#if !GTA_NY
		// rosdll
		HMODULE rosDll = LoadLibrary(L"ros.dll");

		if (rosDll != nullptr)
		{
			((void(*)(const wchar_t*))GetProcAddress(rosDll, "run"))(MakeRelativeCitPath(L"").c_str());
		}

		g_refLauncherHooks = &g_launcherHooks;
		DoLauncherUiSkip();

		hook::iat("kernel32.dll", GetProcAddressHook, "GetProcAddress");

		for (const auto& h : g_launcherHooks)
		{
			hook::iat(std::get<0>(h), std::get<1>(h), std::get<2>(h));
		}

		HMODULE hSteam = LoadLibrary(L"C:\\Program Files\\Rockstar Games\\Launcher\\ThirdParty\\Steam\\steam_api64.dll");

		if (hSteam)
		{
			DisableToolHelpScope scope;
			MH_Initialize();
			MH_CreateHook(GetProcAddress(hSteam, "SteamAPI_Init"), ReturnFalse, NULL);
			MH_EnableHook(MH_ALL_HOOKS);
		}

		{
			DisableToolHelpScope scope;
			MH_Initialize();
			MH_CreateHookApi(L"kernel32.dll", "GetComputerNameExW", GetComputerNameExWStub, NULL);
			MH_CreateHookApi(L"kernel32.dll", "GetLogicalDriveStringsW", GetLogicalDriveStringsWStub, NULL);
			MH_CreateHookApi(L"kernel32.dll", "GetLogicalDrives", GetLogicalDrivesStub, NULL);

#ifdef _DEBUG
			MH_CreateHook(hook::get_pattern("4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 8D"), LogStuff, NULL);
#endif

			MH_EnableHook(MH_ALL_HOOKS);
		}
#endif
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

				if (path.filename().string().find("in-", 0) == 0 || path.filename().string().find("out-", 0) == 0)
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
static FxToolCommand rosSubprocess4("ros:epic", Launcher_HandleArguments, Epic_Run);

void RunLauncher(const wchar_t* toolName, bool instantWait);
void RunLauncherAwait();

#include "ResumeComponent.h"
#include <HostSharedData.h>
#include <CfxState.h>

void WaitForLauncher();
bool LoadOwnershipTicket();

void OnPreInitHook();
void PreInitGameSpec();
void LoadOwnershipEarly();

void Component_RunPreInit()
{
	LoadOwnershipEarly();
	OnPreInitHook();
	PreInitGameSpec();

	if (getenv("CitizenFX_ToolMode") == nullptr || getenv("CitizenFX_ToolMode")[0] == 0)
	{
		OnResumeGame.Connect([]()
		{
			if (!CanSafelySkipLauncher())
			{
				WaitForLauncher();

				SetCanSafelySkipLauncher(true);
			}
		});
	}

	static HostSharedData<CfxState> hostData("CfxInitState");

	if (!hostData->IsMasterProcess())
	{
#ifndef GTA_NY
		if (hostData->IsGameProcess())
		{
			RunLauncherAwait();
		}
#endif

		return;
	}

	if (getenv("CitizenFX_ToolMode") == nullptr || getenv("CitizenFX_ToolMode")[0] == 0)
	{
		auto name = fmt::sprintf(L"CitizenFX_LauncherMutex%s", IsCL2() ? L"CL2" : L"");

		if (OpenMutex(SYNCHRONIZE, FALSE, name.c_str()) == nullptr)
		{
			// create the mutex
			CreateMutex(nullptr, TRUE, name.c_str());

            if (!LoadOwnershipTicket())
            {
                RunLauncher(L"ros:legit", true);
            }

			if (!LoadOwnershipTicket())
			{
				TerminateProcess(GetCurrentProcess(), 0x8000DEAD);
			}

#ifndef GTA_NY
			RunLauncher(L"ros:launcher", false);
#endif
		}

		if (!LoadOwnershipTicket())
		{
			TerminateProcess(GetCurrentProcess(), 0x8000DEAF);
		}
	}
}

static InitFunction lateInitFunction([]()
{
	PreInitGameSpec();
},
-1000);

#include <winsock2.h>
#include <iphlpapi.h>

DWORD _stdcall NotifyIpInterfaceChangeFake(_In_ ADDRESS_FAMILY Family, _In_ void* Callback, _In_opt_ PVOID CallerContext, _In_ BOOLEAN InitialNotification, _Inout_ HANDLE* NotificationHandle)
{
	*NotificationHandle = NULL;
	return NO_ERROR;
}

static InitFunction initFunctionF([]()
{
	DisableToolHelpScope scope;

	// #TODORDR: this hangs on pre-20H1 Windows in a chain from WinVerifyTrust leading to an infinite wait??
	MH_Initialize();
	MH_CreateHookApi(L"iphlpapi.dll", "NotifyIpInterfaceChange", NotifyIpInterfaceChangeFake, NULL);
	MH_EnableHook(MH_ALL_HOOKS);
});

#include <CrossBuildRuntime.h>

static HookFunction hookFunction([] ()
{
	if (!IsWindows7SP1OrGreater())
	{
		FatalError("Windows 7 SP1 or higher is required to run the FiveM ros:five component.");
	}

	
	if (xbr::IsGameBuild<372>())
	{
		hook::iat("crypt32.dll", CertGetNameStringStubW, "CertGetNameStringW");
		hook::iat("crypt32.dll", CertGetNameStringStubA, "CertGetNameStringA");
		hook::iat("wintrust.dll", WinVerifyTrustStub, "WinVerifyTrust");
	}

	// newer SC SDK will otherwise overflow in cert name
#ifndef IS_RDR3
	hook::iat("crypt32.dll", CertGetNameStringStubA, "CertGetNameStringA");
	hook::iat("kernel32.dll", LocalFreeStub, "LocalFree");
#endif

    hook::iat("user32.dll", LoadIconStub, "LoadIconA");
    hook::iat("user32.dll", LoadIconStub, "LoadIconW");

#if !GTA_NY 
	hook::iat("ole32.dll", CoCreateInstanceStub, "CoCreateInstance");
	hook::iat("kernel32.dll", CreateProcessAStub, "CreateProcessA");
#endif

#ifdef GTA_FIVE
	// bypass the check routine for sky init
	void* skyInit = hook::pattern("48 8D A8 D8 FE FF FF 48 81 EC 10 02 00 00 41 BE").count(1).get(0).get<void>(-0x14);
	char* skyInitLoc = hook::pattern("EB 13 48 8D 0D ? ? ? ? 83 FB 08 74 07").count(2).get(0).get<char>(0x71);

	hook::call(skyInitLoc, skyInit);

	// same for distantlights
	void* distantLightInit = hook::pattern("48 8D 68 A1 48 81 EC F0 00 00 00 BE 01 00").count(1).get(0).get<void>(-0x10);

	hook::call(skyInitLoc + (xbr::IsGameBuildOrGreater<2060>() ? 0x2FA : 0x30B), distantLightInit);
#endif
});
