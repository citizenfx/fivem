/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CitizenGame.h"

#include <io.h>
#include <fcntl.h>

#include <CfxState.h>
#include <HostSharedData.h>

#include <array>

extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);

void InitializeDummies();
void EnsureGamePath();

bool InitializeExceptionHandler();

std::map<std::string, std::string> UpdateGameCache();

#pragma comment(lib, "version.lib")

std::map<std::string, std::string> g_redirectionData;

extern "C" int wmainCRTStartup();

void DoPreLaunchTasks();

void main()
{
	bool toolMode = false;

	if (getenv("CitizenFX_ToolMode"))
	{
		toolMode = true;
	}

	if (!toolMode)
	{
		// bootstrap the game
		if (Bootstrap_RunInit())
		{
			return;
		}
	}

	// delete any old .exe.new file
	_unlink("CitizenFX.exe.new");

	// path environment appending of our primary directories
	static wchar_t pathBuf[32768];
	GetEnvironmentVariable(L"PATH", pathBuf, sizeof(pathBuf));

	std::wstring newPath = MakeRelativeCitPath(L"bin") + L";" + MakeRelativeCitPath(L"") + L";" + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	SetDllDirectory(MakeRelativeCitPath(L"bin").c_str()); // to prevent a) current directory DLL search being disabled and b) xlive.dll being taken from system if not overridden

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

	if (addDllDirectory && setDefaultDllDirectories)
	{
		setDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
		addDllDirectory(MakeRelativeCitPath(L"").c_str());
		addDllDirectory(MakeRelativeCitPath(L"bin").c_str());
	}

	// determine dev mode and do updating
	wchar_t exeName[512];
	GetModuleFileName(GetModuleHandle(NULL), exeName, sizeof(exeName) / 2);

	wchar_t* exeBaseName = wcsrchr(exeName, L'\\');
	exeBaseName[0] = L'\0';
	exeBaseName++;

	bool devMode = toolMode;

	if (GetFileAttributes(va(L"%s.formaldev", exeBaseName)) != INVALID_FILE_ATTRIBUTES)
	{
		devMode = true;
	}

	// don't allow running a subprocess executable directly
	if (MakeRelativeCitPath(L"").find(L"cache\\subprocess") != std::string::npos)
	{
		return;
	}

	static HostSharedData<CfxState> initState("CfxInitState");

	// if not the master process, force devmode
	if (!devMode)
	{
		devMode = !initState->IsMasterProcess();
	}

	if (!devMode)
	{
		if (!Bootstrap_DoBootstrap())
		{
			return;
		}
	}

	if (InitializeExceptionHandler())
	{
		return;
	}

	LoadLibrary(MakeRelativeCitPath(L"steam_api64.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"scripthookv.dll").c_str());

	// assign us to a job object
	if (initState->IsMasterProcess())
	{
		HANDLE hJob = CreateJobObject(nullptr, nullptr);

		if (hJob)
		{
			if (AssignProcessToJobObject(hJob, GetCurrentProcess()))
			{
				JOBOBJECT_EXTENDED_LIMIT_INFORMATION info = {};
				info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
				if (SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &info, sizeof(info)))
				{
					initState->inJobObject = true;
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
				return;
			}
		}
	}

	if (initState->IsMasterProcess())
	{
		DoPreLaunchTasks();
	}

	// make sure the game path exists
	EnsureGamePath();

	if (addDllDirectory)
	{
		addDllDirectory(MakeRelativeGamePath(L"").c_str());
	}

	if (!toolMode)
	{
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
					MessageBox(nullptr, L"You are currently using an outdated version of Windows. This may lead to issues using the FiveM client. Please update to Windows 10 version 1703 (\"Creators Update\") or higher in case you are experiencing "
						L"any issues. The game will continue to start now.", L"FiveM", MB_OK | MB_ICONWARNING);
				}
			}

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
							const wchar_t* elevationComplaint = L"FiveM does not support running under elevated privileges. Please change your Windows settings to not run FiveM as administrator.\nThat won't fix anything. The game will exit now.";

							auto result = MessageBox(nullptr, elevationComplaint, L"FiveM", MB_ABORTRETRYIGNORE | MB_ICONERROR);

							if (result == IDIGNORE)
							{
								MessageBox(nullptr, L"No, you can't ignore this. The game will exit now.", L"FiveM", MB_OK | MB_ICONINFORMATION);
							}
							else if (result == IDRETRY)
							{
								MessageBox(nullptr, elevationComplaint, L"FiveM", MB_OK | MB_ICONWARNING);
							}

							return;
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
						MessageBox(nullptr, L"FiveM could not create a file in the folder it is placed in. Please move your installation out of Program Files or another protected folder.", L"Error", MB_OK | MB_ICONSTOP);
						return;
					}
				}
				else
				{
					CloseHandle(hFile);
				}
			}
		}
	}

	// readd the game path into the PATH
	newPath = MakeRelativeCitPath(L"bin\\crt") + L";" + MakeRelativeCitPath(L"bin") + L";" + MakeRelativeCitPath(L"") + L";" + MakeRelativeGamePath(L"") + L"; " + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	if (!toolMode)
	{
		SetCurrentDirectory(MakeRelativeGamePath(L"").c_str());
	}

#ifdef GTA_NY
	// initialize TLS variable so we get a TLS directory
	InitializeDummies();
#endif

	// check stuff regarding the game executable
	std::wstring gameExecutable = MakeRelativeGamePath(GAME_EXECUTABLE);

	if (GetFileAttributes(gameExecutable.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		MessageBox(nullptr, L"Could not find the game executable (" GAME_EXECUTABLE L") at the configured path. Please check your CitizenFX.ini file.", PRODUCT_NAME, MB_OK | MB_ICONERROR);
		return;
	}

#ifdef GTA_FIVE
	// ensure game cache is up-to-date, and obtain redirection metadata from the game cache
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	auto redirectionData = UpdateGameCache();
	g_redirectionData = redirectionData;

	gameExecutable = converter.from_bytes(redirectionData["GTA5.exe"]);

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
				
				if ((fixedInfo->dwFileVersionLS >> 16) != 505)
				{
					MessageBox(nullptr, va(L"The found GTA executable (%s) has version %d.%d.%d.%d, but only 1.0.505 is currently supported. Please obtain this version, and try again.",
										   gameExecutable.c_str(),
										   (fixedInfo->dwFileVersionMS >> 16),
										   (fixedInfo->dwFileVersionMS & 0xFFFF),
										   (fixedInfo->dwFileVersionLS >> 16),
										   (fixedInfo->dwFileVersionLS & 0xFFFF)), PRODUCT_NAME, MB_OK | MB_ICONERROR);

					return;
				}
			}
		}
	}
#endif

	if (!toolMode)
	{
		// game launcher initialization
		CitizenGame::Launch(gameExecutable);
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
}

extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 1;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;