/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifdef LAUNCHER_PERSONALITY_MAIN
#include <CfxLocale.h>
#include <shellapi.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>
#include <sstream>

#include <CfxState.h>
#include <HostSharedData.h>

#include <citversion.h>

extern std::string GetObjectURL(std::string_view objectHash, std::string_view suffix = "");

static bool Bootstrap_UpdateEXE(int exeSize, const std::string& objectHash)
{
	_unlink("CitizenFX.exe.new");

	const char* fn = "CitizenFX.exe.new";
	CL_QueueDownload(GetObjectURL(objectHash, ".xz").c_str(), fn, exeSize, compressionAlgo_e::XZ);

	UI_DoCreation(true);

	UI_UpdateText(0, fmt::sprintf(gettext(L"Bootstrapping %s..."), PRODUCT_NAME).c_str());

	if (!DL_RunLoop())
	{
		UI_DoDestruction();
		return false;
	}

	UI_DoDestruction();

	wchar_t wfn[512];
	MultiByteToWideChar(CP_ACP, 0, fn, -1, wfn, 512);

	wchar_t exePath[512];
	GetModuleFileName(GetModuleHandle(NULL), exePath, sizeof(exePath) / 2);

	STARTUPINFO startupInfo;
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.wShowWindow = SW_HIDE;
	startupInfo.dwFlags |= STARTF_USESHOWWINDOW;

	PROCESS_INFORMATION processInfo;

	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::wstringstream passThroughStream;
	for (auto i = 1; i < argc; i++) {
		passThroughStream << L" " << argv[i];
	}
	std::wstring passThrough = passThroughStream.str();
	CreateProcess(wfn, (LPWSTR)va(L"%s -bootstrap \"%s\"%s", wfn, exePath, passThrough.c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

	return false;
}

bool Install_RunInstallMode();
void Install_RunPostInstall();

bool VerifyViability();

extern void ResetUpdateChannel();

bool Bootstrap_DoBootstrap()
{
	// first check the bootstrapper version
	char bootstrapVersion[256];

	auto contentHeaders = std::make_shared<HttpHeaderList>();

	auto fetchContent = [&contentHeaders, &bootstrapVersion](const std::string& updateChannel)
	{
		return DL_RequestURL(va(STR_CONTENT_URL + "/heads/" CONTENT_NAME "/%scachebypass%lld", updateChannel, _time64(NULL)), bootstrapVersion, sizeof(bootstrapVersion), contentHeaders);
	};

	int result = fetchContent(GetUpdateChannel());
	bool updateDataValid = true;

	if (result != 0)
	{
		bool recovered = false;

		STR_CONTENT_URL = "https://cdn.vmp.ir/updates";

		// if not production, try to recover
		if (GetUpdateChannel() != "production")
		{
			int newResult = fetchContent("production");

			if (newResult == 0)
			{
				ResetUpdateChannel();
				recovered = true;
			}
		}

		if (!recovered)
		{
			if (GetFileAttributes(MakeRelativeCitPath(L"CoreRT.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				UI_DisplayError(ToWide(va("An error (%i, %s) occurred while checking the bootstrapper version. Check if " CFX_UPDATER_URL " is available in your web browser.", result, DL_RequestURLError())).c_str());
				return false;
			}

			updateDataValid = false;
		}
	}

	if (updateDataValid)
	{
		int version = std::stoi((*contentHeaders)["x-amz-meta-bootstrap-version"]);
		int exeSize = std::stoi((*contentHeaders)["x-amz-meta-bootstrap-size"]);

		if (version == 0 || exeSize == 0)
		{
			if (GetFileAttributes(MakeRelativeCitPath(L"CoreRT.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				UI_DisplayError(ToWide(va("An error (%i, %s) occurred while checking the bootstrapper version. Check if " CFX_UPDATER_URL " is available in your web browser.", result, DL_RequestURLError())).c_str());
				return false;
			}

			updateDataValid = false;
		}

		if (updateDataValid)
		{
			if (version != BASE_EXE_VERSION && GetFileAttributes(MakeRelativeCitPath(L"nobootstrap.txt").c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				return Bootstrap_UpdateEXE(exeSize, (*contentHeaders)["x-amz-meta-bootstrap-object"]);
			}
		}
	}

	// after self-updating, attempt to run install mode if needed
	if (Install_RunInstallMode())
	{
		return false;
	}

	if (GetFileAttributes(MakeRelativeCitPath(L"CoreRT.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		if (GetFileAttributes(MakeRelativeCitPath(L"GTA5.exe").c_str()) != INVALID_FILE_ATTRIBUTES || GetFileAttributes(MakeRelativeCitPath(L"..\\GTA5.exe").c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(NULL, L"Please do not place FiveM.exe in your game folder. Make a new empty folder (for example, on your desktop) instead.", L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return false;
		}
	}

	static HostSharedData<CfxState> initState("CfxInitState");
	initState->ranPastInstaller = true;

    if (!VerifyViability())
    {
        return false;
    }

	if (updateDataValid)
	{
		bool rv = false;

#ifdef GTA_FIVE
		if (launch::IsSDK())
		{
			rv = Updater_RunUpdate({ CONTENT_NAME, "fxdk-five" });
		}
		else
#endif
		{
			rv = Updater_RunUpdate({ CONTENT_NAME });
		}

		if (rv)
		{
			Install_RunPostInstall();
		}

		return rv;
	}

	return true;
}

void Bootstrap_ReplaceExecutable(const wchar_t* fileName, const std::wstring& passThrough)
{
	wchar_t thisFileName[512];
	GetModuleFileName(GetModuleHandle(NULL), thisFileName, sizeof(thisFileName) / 2);

	// try opening the file
	bool opened = false;
	HANDLE hFile;

	while (!opened)
	{
		hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			break;
		}

		int error = GetLastError();

		if (error == ERROR_ACCESS_DENIED)
		{
			if (!Install_RunInstallMode())
			{
				UI_DisplayError(L"An 'access denied' error was encountered when updating " PRODUCT_NAME L".");
			}
			else
			{
				MessageBox(NULL, PRODUCT_NAME L" has been installed and can be launched from the shortcut in the Start menu.", PRODUCT_NAME, MB_OK | MB_ICONINFORMATION);
			}

			return;
		}
		else if (error != ERROR_SHARING_VIOLATION)
		{
			UI_DisplayError(va(L"Win32 error %i was encountered when updating " PRODUCT_NAME L".", error));
			return;
		}

		Sleep(50);
	}

	// move the file
	CloseHandle(hFile);
	DeleteFile(va(L"%s.old", fileName));

	if (!MoveFile(fileName, va(L"%s.old", fileName)))
	{
		int error = GetLastError();

		if (error == ERROR_ACCESS_DENIED)
		{
			UI_DisplayError(L"An 'access denied' error was encountered when updating " PRODUCT_NAME L" (moving to .old).");
			return;
		}
		else
		{
			UI_DisplayError(va(L"Win32 error %i was encountered when updating " PRODUCT_NAME L" (moving to .old).", error));
			return;
		}
	}

	// copy our lovely file
	if (!CopyFile(thisFileName, fileName, TRUE))
	{
		int error = GetLastError();

		if (error == ERROR_ACCESS_DENIED)
		{
			UI_DisplayError(L"An 'access denied' error was encountered when updating " PRODUCT_NAME L" (copying game executable).");
			return;
		}
		else
		{
			UI_DisplayError(va(L"Win32 error %i was encountered when updating " PRODUCT_NAME L" (copying game executable).", error));
			return;
		}
	}

	DeleteFile(va(L"%s.old", fileName));

	ShellExecute(NULL, L"open", fileName, passThrough.c_str(), L"", SW_SHOWDEFAULT);
}

void Bootstrap_MoveExecutable(const wchar_t* mode)
{
	wchar_t thisFileName[512];
	GetModuleFileName(GetModuleHandle(NULL), thisFileName, sizeof(thisFileName) / 2);

	std::wstring outFileName = fmt::sprintf(L"%s\\CitizenFX_uninstall_%d.exe", _wgetenv(L"temp"), time(NULL));

	if (CopyFile(thisFileName, outFileName.c_str(), FALSE))
	{
		wcsrchr(thisFileName, L'\\')[0] = L'\0';

		STARTUPINFO startupInfo;
		memset(&startupInfo, 0, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);

		PROCESS_INFORMATION processInfo;

		CreateProcess(NULL, const_cast<LPWSTR>(va(L"%s -doUninstall \"%s\"", outFileName, thisFileName)), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
}

void Install_Uninstall(const wchar_t* directory);

bool Bootstrap_RunInit()
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	if (argc >= 3)
	{
		if (!_wcsicmp(argv[1], L"-bootstrap"))
		{
			std::wstringstream passThrough(L"");
			for (auto i = 3; i < argc; i++) {
				passThrough << (i > 3 ? L" " : L"") << argv[i];
			}

			Bootstrap_ReplaceExecutable(argv[2], passThrough.str());
			LocalFree(argv);
			return true;
		}

		if (!_wcsicmp(argv[1], L"-uninstall"))
		{
			Bootstrap_MoveExecutable(argv[2]);
			LocalFree(argv);
			return true;
		}

		if (!_wcsicmp(argv[1], L"-doUninstall"))
		{
			Install_Uninstall(argv[2]);
			LocalFree(argv);
			return true;
		}
	}

	LocalFree(argv);

	return false;
}
#endif
