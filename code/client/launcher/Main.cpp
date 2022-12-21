/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CitizenGame.h"

// *NASTY* include path
#include <../citicore/LaunchMode.h>
#include <CL2LaunchMode.h>

#include <io.h>
#include <fcntl.h>

#include <CfxState.h>
#include <UUIState.h>
#include "TickCountData.h"
#include <HostSharedData.h>

#include <array>
#include <optional>

#include <shellscalingapi.h>

#include <CrossBuildRuntime.h>

#include <shobjidl.h>

#include <Error.h>

#include <CfxLocale.h>
#include <filesystem>

void InitializeDummies();
std::optional<int> EnsureGamePath();

extern "C" bool InitializeExceptionHandler();
bool InitializeExceptionServer();

std::map<std::string, std::string> UpdateGameCache();

#pragma comment(lib, "version.lib")

std::map<std::string, std::string> g_redirectionData;

void DoPreLaunchTasks();
void NVSP_DisableOnStartup();
void SteamInput_Initialize();
void XBR_EarlySelect();
bool ExecutablePreload_Init();
void InitLogging();

#include <MinMode.h>
#include <fstream>

auto InitMinMode()
{
#if defined(LAUNCHER_PERSONALITY_GAME) || defined(LAUNCHER_PERSONALITY_MAIN)
	const wchar_t* cli = GetCommandLineW();

	auto minmodePos = wcsstr(cli, L"+set minmodemanifest \"");

	try
	{
		if (minmodePos != nullptr)
		{
			std::wstring fileName = &minmodePos[wcslen(L"+set minmodemanifest \"")];
			auto fnEnd = fileName.find_first_of('"');

			if (fnEnd != std::string::npos)
			{
				fileName = fileName.substr(0, fnEnd);

				std::ifstream fs(fileName);

				nlohmann::json j;
				fs >> j;

				return std::make_shared<fx::MinModeManifest>(j);
			}
		}
	}
	catch (std::exception& e)
	{

	}

	return std::make_shared<fx::MinModeManifest>();
#else
	return nullptr;
#endif
}

HANDLE g_uiDoneEvent;
HANDLE g_uiExitEvent;

bool IsUnsafeGraphicsLibrary();
void MigrateCacheFormat202105();
void UI_DestroyTen();

HMODULE tlsDll;

// list of DLLs to load from system before any other code execution
const static const wchar_t* g_delayDLLs[] = {
#include "DelayList.h"
};

void DLLError(DWORD errorCode, std::string_view dllName)
{
	// force verifying game files
	_wunlink(MakeRelativeCitPath(L"content_index.xml").c_str());

	wchar_t errorText[512];
	errorText[0] = L'\0';

	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorText, std::size(errorText), nullptr);

	FatalError("Could not load %s\nThis is usually a sign of an incomplete game installation. Please restart %s and try again.\n\nError 0x%08x - %s",
		dllName,
		ToNarrow(PRODUCT_NAME),
		HRESULT_FROM_WIN32(errorCode),
		ToNarrow(errorText));
}

int RealMain()
{
	if (auto setSearchPathMode = (decltype(&SetSearchPathMode))GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "SetSearchPathMode"))
	{
		setSearchPathMode(BASE_SEARCH_PATH_ENABLE_SAFE_SEARCHMODE | BASE_SEARCH_PATH_PERMANENT);
	}

	// load early (delayimp'd) system DLLs
	auto loadSystemDll = [](auto dll)
	{
		wchar_t systemPath[512];
		GetSystemDirectoryW(systemPath, _countof(systemPath));

		StringCchCat(systemPath, std::size(systemPath), dll);

		LoadLibraryW(systemPath);
	};

	for (auto dll : g_delayDLLs)
	{
		loadSystemDll(dll);
	}

	bool toolMode = false;

	if (getenv("CitizenFX_ToolMode"))
	{
		toolMode = true;
	}

	auto isSubProcess = []()
	{
		wchar_t fxApplicationName[MAX_PATH];
		GetModuleFileName(GetModuleHandle(nullptr), fxApplicationName, _countof(fxApplicationName));

		return wcsstr(fxApplicationName, L"subprocess") != nullptr;
	}();

#ifdef LAUNCHER_PERSONALITY_MAIN
	bool devMode = toolMode;

	// toggle wait for switch
	if (wcsstr(GetCommandLineW(), L"-switchcl"))
	{
		if (!isSubProcess)
		{
			HANDLE hProcess = NULL;

			{
				HostSharedData<CfxState> initStateOld("CfxInitState");

				hProcess = OpenProcess(SYNCHRONIZE, FALSE, initStateOld->initialGamePid);
				initStateOld->initialGamePid = 0;

				if (auto eventStr = wcsstr(GetCommandLineW(), L"-switchcl:"))
				{
					HANDLE eventHandle = reinterpret_cast<HANDLE>(wcstoull(&eventStr[10], nullptr, 10));
					SetEvent(eventHandle);
					CloseHandle(eventHandle);
				}
			}

			if (hProcess)
			{
				WaitForSingleObject(hProcess, INFINITE);
				CloseHandle(hProcess);
			}

			// switchcl'd client shouldn't check for updates again
			devMode = true;
		}
	}
#endif

	if (!toolMode)
	{
#ifdef LAUNCHER_PERSONALITY_MAIN
		if (!isSubProcess)
		{
			static HostSharedData<TickCountData> initTickCount("CFX_SharedTickCount");
		}

		// run exception handler
		if (InitializeExceptionServer())
		{
			return 0;
		}

		// bootstrap the game
		if (Bootstrap_RunInit())
		{
			return 0;
		}
#endif
	}

#if 0
	// TEST
	auto tenner = UI_InitTen();

	UI_DoCreation();

	while (!UI_IsCanceled())
	{
		HANDLE h = GetCurrentThread();
		MsgWaitForMultipleObjects(1, &h, FALSE, 50, QS_ALLEVENTS);
		
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		UI_UpdateText(0, va(L"%d", GetTickCount()));
		UI_UpdateText(1, va(L"%x", GetTickCount()));

		UI_UpdateProgress(50.0);
	}

	UI_DoDestruction();

	tenner = {};

	ExitProcess(0);
#endif

	// delete any old .exe.new file
	_unlink("CitizenFX.exe.new");

	// initialize our initState instance
	// this needs to be before *any* MakeRelativeCitPath use in main process
	HostSharedData<CfxState> initState("CfxInitState");

	// path environment appending of our primary directories
	static wchar_t pathBuf[32768];
	GetEnvironmentVariable(L"PATH", pathBuf, std::size(pathBuf));

	std::wstring newPath = MakeRelativeCitPath(L"bin") + L";" + MakeRelativeCitPath(L"") + L";" + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	SetDllDirectory(MakeRelativeCitPath(L"bin").c_str()); // to prevent a) current directory DLL search being disabled and b) xlive.dll being taken from system if not overridden

	// GTA_NY can't use TLS DLL since it doesn't consistently use the TLS index variable
	// this is also only important for 'game'-type processes, and may be wrong/unsafe in any other types
#if !defined(GTA_NY) && defined(LAUNCHER_PERSONALITY_GAME)
	// first attempt at loading the TLS holder DLL
	tlsDll = LoadLibraryW(MakeRelativeCitPath(L"CitiLaunch_TLSDummy.dll").c_str());
#endif

	wchar_t initCwd[1024];
	GetCurrentDirectoryW(std::size(initCwd), initCwd);

	if (!toolMode)
	{
		SetCurrentDirectory(MakeRelativeCitPath(L"").c_str());
	}

	auto addDllDirectory = (decltype(&AddDllDirectory))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "AddDllDirectory");
	auto setDefaultDllDirectories = (decltype(&SetDefaultDllDirectories))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetDefaultDllDirectories");

	// don't set DLL directories for ros:legit under Windows 7
	// see https://connect.microsoft.com/VisualStudio/feedback/details/2281687/setdefaultdlldirectories-results-in-exception-during-opening-a-winform-on-win7
	if (!IsWindows8OrGreater() && toolMode && wcsstr(GetCommandLineW(), L"ros:legit"))
	{
		setDefaultDllDirectories = nullptr;
	}

	auto addDllDirs = [&]()
	{
		if (addDllDirectory && setDefaultDllDirectories)
		{
			setDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
			addDllDirectory(MakeRelativeCitPath(L"").c_str());
			addDllDirectory(MakeRelativeCitPath(L"bin").c_str());
		}
	};

	// add DLL directories pre-installer
	addDllDirs();

	// determine dev mode and do updating
#ifdef LAUNCHER_PERSONALITY_MAIN
	{
		wchar_t exeName[512];
		GetModuleFileName(GetModuleHandle(NULL), exeName, std::size(exeName));

		std::wstring exeNameSaved = exeName;

		wchar_t* exeBaseName = wcsrchr(exeName, L'\\');
		exeBaseName[0] = L'\0';
		exeBaseName++;

		if (GetFileAttributes(MakeRelativeCitPath(fmt::sprintf(L"%s.formaldev", exeBaseName)).c_str()) != INVALID_FILE_ATTRIBUTES ||
			GetFileAttributes(fmt::sprintf(L"%s.formaldev", exeNameSaved).c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			devMode = true;
		}
	}
#else
	bool devMode = true;
#endif

	// don't allow running a subprocess executable directly
	if (MakeRelativeCitPath(L"").find(L"cache\\subprocess") != std::string::npos)
	{
		return 0;
	}

	// store the last run directory for assistance purposes
	{
		auto regPath = MakeRelativeCitPath(L"");

		RegSetKeyValueW(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\" PRODUCT_NAME, L"Last Run Location", REG_SZ, regPath.c_str(), (regPath.size() + 1) * 2);
	}

	SetCurrentProcessExplicitAppUserModelID(va(L"CitizenFX.%s.%s", PRODUCT_NAME, launch::IsSDK() ? L"SDK" : L"Client"));

#ifdef IS_LAUNCHER
	initState->isReverseGame = true;
#endif

	// check if the master process still lives
	{
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, initState->GetInitialPid());

		if (hProcess == nullptr)
		{
			initState->SetInitialPid(GetCurrentProcessId());
		}
		else
		{
			DWORD exitCode = STILL_ACTIVE;
			GetExitCodeProcess(hProcess, &exitCode);

			if (exitCode != STILL_ACTIVE)
			{
				initState->SetInitialPid(GetCurrentProcessId());
			}

			CloseHandle(hProcess);
		}
	}

	// MasterProcess is safe from this point on

	// copy main process command line, if needed
	if (initState->IsMasterProcess())
	{
		wcsncpy(initState->initCommandLine, GetCommandLineW(), std::size(initState->initCommandLine) - 1);
	}

#ifdef LAUNCHER_PERSONALITY_MAIN
	// if not the master process, force devmode
	if (!devMode)
	{
		devMode = !initState->IsMasterProcess();
	}

	// init tenUI
	std::unique_ptr<TenUIBase> tui;

	if (initState->IsMasterProcess())
	{
		tui = UI_InitTen();
	}

	if (!devMode)
	{
		if (!Bootstrap_DoBootstrap())
		{
			return 0;
		}
	}

	// in case Bootstrap_* didn't set this
	initState->ranPastInstaller = true;

	XBR_EarlySelect();
#endif

	// crossbuildruntime is safe from this point on

	// try loading TLS DLL a second time, and ensure it *is* loaded
#if !defined(GTA_NY) && defined(LAUNCHER_PERSONALITY_GAME)
	if (!tlsDll)
	{
		tlsDll = LoadLibraryW(MakeRelativeCitPath(L"CitiLaunch_TLSDummy.dll").c_str());

		if (!tlsDll)
		{
			DWORD errorCode = GetLastError();

			DLLError(errorCode, "TLS DLL");
		}

		assert(tlsDll);
	}
#endif

	// remove any app compat layers, if present
	{
		HKEY hKey;
		if (RegOpenKeyW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers", &hKey) == ERROR_SUCCESS)
		{
			std::vector<wchar_t> valueName(32768);
			std::wstring citPath = MakeRelativeCitPath(L"");

			if (citPath.find(L".app") != std::string::npos)
			{
				citPath += L"..\\";
			}

			// get the canonical version of the path
			wchar_t finalPath[512];
			GetFullPathNameW(citPath.c_str(), std::size(finalPath), finalPath, NULL);

			auto finalPathLength = wcslen(finalPath);
			std::vector<std::wstring> toDelete;

			for (DWORD i = 0; ; i++)
			{
				DWORD cchValueName = valueName.size();
				auto error = RegEnumValueW(hKey, i, valueName.data(), &cchValueName, NULL, NULL, NULL, NULL);

				if (error != ERROR_SUCCESS)
				{
					break;
				}

				// if the key is in 'our' path
				if (_wcsnicmp(valueName.data(), finalPath, finalPathLength) == 0)
				{
					toDelete.push_back(valueName.data());
				}
			}

			// delete any queued values
			for (auto& value : toDelete)
			{
				RegDeleteValueW(hKey, value.c_str());
			}

			RegCloseKey(hKey);
		}
	}

	// add DLL directories post-installer (in case we moved into a Product.app directory)
	addDllDirs();

	// we have to migrate *before* launching the crash handler as that will otherwise create data/cache/
	if (initState->IsMasterProcess())
	{
		MigrateCacheFormat202105();
	}

	if (InitializeExceptionHandler())
	{
		return 0;
	}

	InitLogging();

	// load some popular DLLs as system-wide variants instead of game variants
	auto systemDlls = {
		// common ASI loaders
		L"\\dinput8.dll",
		L"\\dsound.dll", // breaks DSound init in game code

		// X360CE v3 is buggy with COM hooks
		L"\\xinput9_1_0.dll",
		L"\\xinput1_1.dll",
		L"\\xinput1_2.dll",
		L"\\xinput1_3.dll",
		L"\\xinput1_4.dll",

		// packed DLL commonly shipping with RDR mods
		L"\\version.dll"
	};

	for (auto dll : systemDlls)
	{
		loadSystemDll(dll);
	}

	// assign us to a job object
	if (initState->IsMasterProcess())
	{
		// set DPI-aware
		HMODULE shCore = LoadLibrary(L"shcore.dll");

		if (shCore)
		{
			auto SetProcessDpiAwareness = (decltype(&::SetProcessDpiAwareness))GetProcAddress(shCore, "SetProcessDpiAwareness");

			if (SetProcessDpiAwareness)
			{
				SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
			}
		}

		// delete crashometry
		_wunlink(MakeRelativeCitPath(L"data\\cache\\crashometry").c_str());
		_wunlink(MakeRelativeCitPath(L"data\\cache\\error_out").c_str());

		if (GetFileAttributesW(MakeRelativeCitPath(L"permalauncher").c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			// create job
			HANDLE hJob = CreateJobObject(nullptr, nullptr);

			if (hJob)
			{
				if (AssignProcessToJobObject(hJob, GetCurrentProcess()))
				{
					JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
					info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_BREAKAWAY_OK;
					if (SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &info, sizeof(info)))
					{
						initState->inJobObject = true;
					}
				}
			}
		}
	}

	// exit if not in a job object
	if (initState->inJobObject)
	{
		BOOL result;
		IsProcessInJob(GetCurrentProcess(), nullptr, &result);

		if (!result)
		{
			// and if this isn't a subprocess
			wchar_t fxApplicationName[MAX_PATH];
			GetModuleFileName(GetModuleHandle(nullptr), fxApplicationName, _countof(fxApplicationName));

			if (wcsstr(fxApplicationName, L"subprocess") == nullptr)
			{
				// and not a fivem:// protocol handler
				if (wcsstr(GetCommandLineW(), L"fivem://") == nullptr)
				{
					return 0;
				}
			}
		}
	}

	if (initState->IsMasterProcess())
	{
		DoPreLaunchTasks();
	}

	// make sure the game path exists
	if (auto gamePathExit = EnsureGamePath(); gamePathExit)
	{
		return *gamePathExit;
	}

	// don't load anything resembling ReShade/ENBSeries *at all* until the game is loading(!)
	loadSystemDll(L"\\dxgi.dll");

	// *must* load a d3d11.dll before anything else!
	// if not, system d3d10.dll etc. may load a d3d11.dll from search path anyway and this may be a 'weird' one
	loadSystemDll(L"\\d3d11.dll");

	loadSystemDll(L"\\d3d9.dll");
	loadSystemDll(L"\\d3d10.dll");
	loadSystemDll(L"\\d3d10_1.dll");
	loadSystemDll(L"\\opengl32.dll");

#ifndef LAUNCHER_PERSONALITY_CHROME
	LoadLibrary(MakeRelativeCitPath(L"botan.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"dinput8.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"steam_api64.dll").c_str());

	// laod V8 DLLs in case end users have these in a 'weird' directory
	LoadLibrary(MakeRelativeCitPath(L"bin/icuuc.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"bin/icui18n.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"v8_libplatform.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"v8_libbase.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"v8.dll").c_str());
#endif

	if (addDllDirectory)
	{
		addDllDirectory(MakeRelativeGamePath(L"").c_str());
	}

	if (!toolMode)
	{
#if defined(LAUNCHER_PERSONALITY_MAIN)
		// try removing any old updater files
		try
		{
			for (auto& f : std::filesystem::directory_iterator{ MakeRelativeCitPath(L"") })
			{
				std::filesystem::path path = f.path();
				if (path.filename().string().find(".updater-remove-") == 0)
				{
					std::error_code ec;
					std::filesystem::remove(path, ec);
				}
			}
		}
		catch (std::exception&)
		{
		}
#endif

		if (OpenMutex(SYNCHRONIZE, FALSE, L"CitizenFX_LogMutex") == nullptr)
		{
			// create the mutex
			CreateMutex(nullptr, TRUE, L"CitizenFX_LogMutex");

			// rotate any CitizenFX.log files cleanly
			const int MaxLogs = 10;

			auto makeLogName = [] (int idx)
			{
				return MakeRelativeCitPath(va(L"CitizenFX.log%s", (idx == 0) ? L"" : va(L".%d", idx)));
			};

			for (int i = (MaxLogs - 1); i >= 0; i--)
			{
				std::wstring logPath = makeLogName(i);
				std::wstring newLogPath = makeLogName(i + 1);

				if ((i + 1) == MaxLogs)
				{
					_wunlink(logPath.c_str());
				}
				else
				{
					_wrename(logPath.c_str(), newLogPath.c_str());
				}
			}

#if defined(LAUNCHER_PERSONALITY_MAIN)
			// also do checks here to complain at BAD USERS
			if (!GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetThreadDescription")) // kernel32 forwarder only got this export in 1703, kernelbase.dll got this in 1607.
			{
				std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

				bool showOSWarning = true;

				if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
				{
					showOSWarning = (GetPrivateProfileInt(L"Game", L"DisableOSVersionCheck", 0, fpath.c_str()) != 1);
				}

				if (showOSWarning)
				{
					MessageBox(nullptr, fmt::sprintf(gettext(L"You are currently using an outdated version of Windows. This may lead to issues using the %s client. Please update to Windows 10 version 1703 (\"Creators Update\") or higher in case you are experiencing "
															 L"any issues. The game will continue to start now."),
										PRODUCT_NAME).c_str(),
					PRODUCT_NAME, MB_OK | MB_ICONWARNING);
				}
			}

			bool checkElevation = !CfxIsWine();

#ifdef _DEBUG
			checkElevation = false;
#endif

			if (checkElevation)
			{
				HANDLE hToken;

				if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
				{
					TOKEN_ELEVATION_TYPE elevationData;
					DWORD size;

					if (GetTokenInformation(hToken, TokenElevationType, &elevationData, sizeof(elevationData), &size))
					{
						if (elevationData == TokenElevationTypeFull)
						{
							const wchar_t* elevationComplaint = va(gettext(L"%s does not support running under elevated privileges. Please change your Windows settings to not run %s as administrator.\nThe game will exit now."), PRODUCT_NAME, PRODUCT_NAME);
							MessageBox(nullptr, elevationComplaint, PRODUCT_NAME, MB_OK | MB_ICONERROR);

							return 0;
						}
					}

					CloseHandle(hToken);
				}
			}

			{
				HANDLE hFile = CreateFile(MakeRelativeCitPath(L"writable_test").c_str(), GENERIC_WRITE, FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);

				if (hFile == INVALID_HANDLE_VALUE)
				{
					if (GetLastError() == ERROR_ACCESS_DENIED)
					{
						MessageBoxW(nullptr, va(gettext(L"%s could not create a file in the folder it is placed in. Please move your installation out of Program Files or another protected folder."), PRODUCT_NAME), L"Error", MB_OK | MB_ICONSTOP);
						return 0;
					}
				}
				else
				{
					CloseHandle(hFile);
				}
			}
#endif
		}
	}

	if (initState->IsMasterProcess())
	{
#ifdef LAUNCHER_PERSONALITY_MAIN
		NVSP_DisableOnStartup();
		SteamInput_Initialize();
#endif

		GetModuleFileNameW(NULL, initState->gameExePath, std::size(initState->gameExePath));
	}

	// readd the game path into the PATH
	newPath = MakeRelativeCitPath(L"bin\\crt") + L";" + MakeRelativeCitPath(L"bin") + L";" + MakeRelativeCitPath(L"") + L";" + MakeRelativeGamePath(L"") + L"; " + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	if (launch::IsSDK())
	{
		SetCurrentDirectory(initCwd);
	}
	else if (!toolMode)
	{
		SetCurrentDirectory(MakeRelativeGamePath(L"").c_str());
	}

	// check stuff regarding the game executable
	std::wstring gameExecutable = MakeRelativeGamePath(GAME_EXECUTABLE);

#if defined(GTA_FIVE) || defined(IS_RDR3)
	if (!ExecutablePreload_Init())
	{
		return 0;
	}
#endif

#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
#if (defined(LAUNCHER_PERSONALITY_GAME) || defined(LAUNCHER_PERSONALITY_MAIN))
	// ensure game cache is up-to-date, and obtain redirection metadata from the game cache
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	auto redirectionData = UpdateGameCache();

	if (redirectionData.empty())
	{
		return 0;
	}

	g_redirectionData = redirectionData;

#if !defined(IS_RDR3)
#ifdef GTA_FIVE
	gameExecutable = converter.from_bytes(redirectionData["GTA5.exe"]);
#endif

	{
		DWORD versionInfoSize = GetFileVersionInfoSize(gameExecutable.c_str(), nullptr);

		if (versionInfoSize)
		{
			std::vector<uint8_t> versionInfo(versionInfoSize);

			if (GetFileVersionInfo(gameExecutable.c_str(), 0, versionInfo.size(), &versionInfo[0]))
			{
				void* fixedInfoBuffer;
				UINT fixedInfoSize;

				VerQueryValue(&versionInfo[0], L"\\", &fixedInfoBuffer, &fixedInfoSize);

				VS_FIXEDFILEINFO* fixedInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(fixedInfoBuffer);

#if defined(GTA_FIVE)
				auto expectedVersion = xbr::GetGameBuild();

				if ((fixedInfo->dwFileVersionLS >> 16) != expectedVersion)
#else
				auto expectedVersion = 43;

				if ((fixedInfo->dwFileVersionLS & 0xFFFF) != expectedVersion)
#endif
				{
					MessageBox(nullptr, va(L"The found game executable (%s) has version %d.%d.%d.%d, but we're trying to run build %d. Please obtain this version, and try again.",
										   gameExecutable.c_str(),
										   (fixedInfo->dwFileVersionMS >> 16),
										   (fixedInfo->dwFileVersionMS & 0xFFFF),
										   (fixedInfo->dwFileVersionLS >> 16),
										   (fixedInfo->dwFileVersionLS & 0xFFFF),
										   xbr::GetGameBuild()), PRODUCT_NAME, MB_OK | MB_ICONERROR);

					return 0;
				}
			}
		}
	}
#endif
#endif
#endif

#ifdef LAUNCHER_PERSONALITY_MAIN
	tui = {};
#endif

	auto minModeManifest = InitMinMode();

	g_uiExitEvent = CreateEventW(NULL, TRUE, FALSE, va(L"CitizenFX_PreUIExit%s", IsCL2() ? L"CL2" : L""));
	g_uiDoneEvent = CreateEventW(NULL, FALSE, FALSE, va(L"CitizenFX_PreUIDone%s", IsCL2() ? L"CL2" : L""));

	if (initState->IsMasterProcess() && !toolMode && !launch::IsSDK())
	{
		std::thread([minModeManifest]() mutable
		{
			static HostSharedData<CfxState> initState("CfxInitState");
			static HostSharedData<UpdaterUIState> uuiState("CfxUUIState");

#if !defined(_DEBUG) && defined(LAUNCHER_PERSONALITY_MAIN)
			auto runUUILoop = [minModeManifest](bool firstLoop)
			{
				static constexpr const uint32_t loadWait = 5000;
				auto tuiTen = UI_InitTen();

				// say hi
				UI_DoCreation(false);

				std::string firstTitle = fmt::sprintf("Starting %s",
					minModeManifest->Get("productName", ToNarrow(PRODUCT_NAME)));
				std::string firstSubtitle = (wcsstr(GetCommandLineW(), L"-switchcl"))
					? gettext("Transitioning to another build...") 
					: minModeManifest->Get("productSubtitle", gettext("We're getting there."));

				std::string lastTitle = firstTitle;
				std::string lastSubtitle = firstSubtitle;

				auto st = GetTickCount64();
				UI_UpdateText(0, ToWide(lastTitle).c_str());
				UI_UpdateText(1, ToWide(lastSubtitle).c_str());

				auto expired = [&st]()
				{
					return GetTickCount64() >= (st + loadWait);
				};

				auto shouldBeOpen = [expired]()
				{
					if (!uuiState->finalized)
					{
						return true;
					}

					return !expired();
				};

				auto shouldBeCustom = [expired, firstLoop]()
				{
					// UUI customizations do not apply if finalized
					if (uuiState->finalized)
					{
						return false;
					}

					return (uuiState->waitForExpiration && firstLoop) ? expired() : true;
				};

				bool wasCustom = false;

				while (shouldBeOpen())
				{
					HANDLE hs[] = {
						g_uiExitEvent
					};

					auto res = MsgWaitForMultipleObjects(std::size(hs), hs, FALSE, 50, QS_ALLEVENTS);

					// bail out if wait failed, or if the handle is signaled
					if (res == WAIT_OBJECT_0 || res == WAIT_FAILED)
					{
						break;
					}

					MSG msg;
					while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}

					if (uuiState->progress < 0.0 || !shouldBeCustom())
					{
						UI_UpdateProgress((GetTickCount64() - st) / (loadWait / 100.0));
					}
					else
					{
						UI_UpdateProgress(uuiState->progress);
					}

					if (shouldBeCustom())
					{
						wasCustom = true;

						std::string titleStr = uuiState->title;
						std::string subtitleStr = uuiState->subtitle;

						if (!titleStr.empty() && titleStr != lastTitle)
						{
							UI_UpdateText(0, ToWide(titleStr).c_str());
							lastTitle = titleStr;
						}

						if (!subtitleStr.empty() && subtitleStr != lastSubtitle)
						{
							UI_UpdateText(1, ToWide(subtitleStr).c_str());
							lastSubtitle = subtitleStr;
						}
					}
					else if (wasCustom)
					{
						UI_UpdateText(0, ToWide(firstTitle).c_str());
						UI_UpdateText(1, ToWide(firstSubtitle).c_str());

						st = GetTickCount64();

						wasCustom = false;
					}
				}

				UI_DoDestruction();
			};

			if (!initState->isReverseGame)
			{
				runUUILoop(true);
			}
#endif

			SetEvent(g_uiDoneEvent);

#if !defined(_DEBUG) && defined(LAUNCHER_PERSONALITY_MAIN)
			if (!initState->isReverseGame)
			{
				// run UI polling loop if need be, anyway
				while (true)
				{
					// UI exit event ends the thread
					if (WaitForSingleObject(g_uiExitEvent, 50) != WAIT_TIMEOUT)
					{
						break;
					}

					// did we get asked to open UUI again?
					if (!uuiState->finalized)
					{
						runUUILoop(false);
					}
				}
			}
#endif

			UI_DestroyTen();
		}).detach();
	}

	if (launch::IsSDKGuest())
	{
		SetEvent(g_uiDoneEvent);
	}

	if (!toolMode)
	{
#if defined(LAUNCHER_PERSONALITY_GAME) || defined(LAUNCHER_PERSONALITY_MAIN)
		CitizenGame::SetMinModeManifest(minModeManifest->GetRaw());
#endif

		wchar_t fxApplicationName[MAX_PATH];
		GetModuleFileName(GetModuleHandle(nullptr), fxApplicationName, _countof(fxApplicationName));

		if (launch::IsSDK() && initState->IsMasterProcess())
		{
			// run game mode
			HMODULE coreRT = LoadLibrary(MakeRelativeCitPath(L"CoreRT.dll").c_str());

			if (coreRT)
			{
				auto gameProc = (void (*)())GetProcAddress(coreRT, "GameMode_RunSDK");

				if (gameProc)
				{
					gameProc();
				}
			}

			return 0;
		}

#ifdef IS_LAUNCHER
		// is this the game runtime subprocess?
		if (wcsstr(fxApplicationName, L"GameRuntime") != nullptr)
		{
#else
		if (initState->IsMasterProcess() || wcsstr(fxApplicationName, L"GameRuntime") != nullptr)
		{
#endif
#ifdef _DEBUG
			//MessageBox(nullptr, va(L"Gameruntime starting (pid %d)", GetCurrentProcessId()), L"CitizenFx", MB_OK);
#endif

#if (defined(GTA_FIVE) || defined(IS_RDR3)) && !defined(LAUNCHER_PERSONALITY_GAME) && defined(LAUNCHER_PERSONALITY_MAIN)
			if (initState->IsMasterProcess())
			{
				// run game mode
				HMODULE coreRT = LoadLibrary(MakeRelativeCitPath(L"CoreRT.dll").c_str());

				if (coreRT)
				{
					auto gameProc = (void (*)())GetProcAddress(coreRT, "EarlyMode_Init");

					if (gameProc)
					{
						gameProc();
					}
				}
				else
				{
					DWORD errorCode = GetLastError();
					DLLError(errorCode, "CoreRT.dll");
				}

				return 0;
			}
#endif

			// game launcher initialization
			CitizenGame::Launch(gameExecutable);
		}
#ifdef IS_LAUNCHER
		// it's not, is this the first process running?
		else if (initState->IsMasterProcess())
		{
			// run game mode
			HMODULE coreRT = LoadLibrary(MakeRelativeCitPath(L"CoreRT.dll").c_str());

			if (coreRT)
			{
				auto gameProc = (void(*)())GetProcAddress(coreRT, "GameMode_Init");

				if (gameProc)
				{
					gameProc();
				}
			}
		}
#endif
		else
		{
			// could be it's a prelauncher like Chrome
			CitizenGame::Launch(gameExecutable, true);
		}
	}
	else
	{
		HMODULE coreRT = LoadLibrary(MakeRelativeCitPath(L"CoreRT.dll").c_str());

		if (coreRT)
		{
			auto toolProc = (void(*)())GetProcAddress(coreRT, "ToolMode_Init");

			if (toolProc)
			{
				auto gameFunctionProc = (void(*)(void(*)(const wchar_t*)))GetProcAddress(coreRT, "ToolMode_SetGameFunction");

				if (gameFunctionProc)
				{
					static auto gameExecutableStr = gameExecutable;

					gameFunctionProc([] (const wchar_t* customExecutable)
					{
						if (customExecutable == nullptr)
						{
							SetCurrentDirectory(MakeRelativeGamePath(L"").c_str());

							if (OpenMutex(SYNCHRONIZE, FALSE, L"CitizenFX_GameMutex") == nullptr)
							{
								CreateMutex(nullptr, TRUE, L"CitizenFX_GameMutex");

								CitizenGame::Launch(gameExecutableStr);
							}
						}
						else
						{
							CitizenGame::Launch(customExecutable);
						}
					});
				}

                CitizenGame::SetCoreMapping();

				toolProc();
			}
			else
			{
				printf("Could not find ToolMode_Init in CoreRT.dll.\n");
			}
		}
		else
		{
			printf("Could not initialize CoreRT.dll.\n");
		}
	}

	return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	return RealMain();
}

int main()
{
	return RealMain();
}

extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;
