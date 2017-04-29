/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <shellapi.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <softpub.h>

#include <citversion.h>

#pragma comment(lib, "wintrust")
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

bool Bootstrap_UpdateEXE(int exeSize)
{
	_unlink("CitizenFX.exe.new");

	const char* fn = "CitizenFX.exe.new";
	CL_QueueDownload(va(CONTENT_URL "/%s/bootstrap/CitizenFX.exe.xz", GetUpdateChannel()), fn, exeSize, true);

	UI_DoCreation();

	if (!DL_RunLoop())
	{
		UI_DoDestruction();
		return false;
	}

	UI_DoDestruction();

	// verify the signature on the EXE
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

	CreateProcess(wfn, (LPWSTR)va(L"%s -bootstrap \"%s\"", wfn, exePath), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

	return false;
}

bool Install_RunInstallMode();

bool VerifyViability();

bool Bootstrap_DoBootstrap()
{
	// first check the bootstrapper version
	char bootstrapVersion[256];

	int result = DL_RequestURL(va(CONTENT_URL "/%s/bootstrap/version.txt", GetUpdateChannel()), bootstrapVersion, sizeof(bootstrapVersion));

	if (result != 0)
	{
		if (GetFileAttributes(MakeRelativeCitPath(L"CoreRT.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(NULL, va(L"An error (%i) occurred while checking the bootstrapper version. Check if " CONTENT_URL_WIDE L" is available in your web browser.", result), L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return false;
		}

		return true;
	}

	//int version = atoi(bootstrapVersion);
	int version;
	int exeSize;
	sscanf(bootstrapVersion, "%i %i", &version, &exeSize);

	if (version != BASE_EXE_VERSION && GetFileAttributes(MakeRelativeCitPath(L"nobootstrap.txt").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return Bootstrap_UpdateEXE(exeSize);
	}

	// after self-updating, attempt to run install mode if needed
	if (Install_RunInstallMode())
	{
		return false;
	}

	if (GetFileAttributes(MakeRelativeCitPath(L"CoreRT.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		if (GetFileAttributes(MakeRelativeCitPath(L"GTA5.exe").c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(NULL, L"Please do not place FiveM.exe in your game folder. Make a new empty folder (for example, on your desktop) instead.", L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return false;
		}
	}

    if (!VerifyViability())
    {
        return false;
    }

#ifdef GTA_NY
	return Updater_RunUpdate(1, "citiv");
#else
	return Updater_RunUpdate(1, "fivereborn");
#endif
}

void Bootstrap_ReplaceExecutable(const wchar_t* fileName)
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
			MessageBox(NULL, L"An 'access denied' error was encountered when updating " PRODUCT_NAME L". Please try to run the game as an administrator, or contact support.", L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return;
		}
		else if (error != ERROR_SHARING_VIOLATION)
		{
			MessageBox(NULL, va(L"Win32 error %i was encountered when updating " PRODUCT_NAME L".", error), L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
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
			MessageBox(NULL, L"An 'access denied' error was encountered when updating " PRODUCT_NAME L" (moving to .old). Please try to run the game as an administrator, or contact support.", L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return;
		}
		else
		{
			MessageBox(NULL, va(L"Win32 error %i was encountered when updating " PRODUCT_NAME L" (moving to .old).", error), L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return;
		}
	}

	// copy our lovely file
	if (!CopyFile(thisFileName, fileName, TRUE))
	{
		int error = GetLastError();

		if (error == ERROR_ACCESS_DENIED)
		{
			MessageBox(NULL, L"An 'access denied' error was encountered when updating " PRODUCT_NAME L" (copying game executable). Please try to run the game as an administrator, or contact support.", L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return;
		}
		else
		{
			MessageBox(NULL, va(L"Win32 error %i was encountered when updating " PRODUCT_NAME L" (copying game executable).", error), L"O\x448\x438\x431\x43A\x430", MB_OK | MB_ICONSTOP);
			return;
		}
	}

	DeleteFile(va(L"%s.old", fileName));

	ShellExecute(NULL, L"open", fileName, L"", L"", SW_SHOWDEFAULT);
}

bool Bootstrap_RunInit()
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	if (argc == 3)
	{
		if (!_wcsicmp(argv[1], L"-bootstrap"))
		{
			Bootstrap_ReplaceExecutable(argv[2]);
			LocalFree(argv);
			return true;
		}
	}

	LocalFree(argv);

	return false;
}