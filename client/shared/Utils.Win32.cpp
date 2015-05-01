/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Utils.h"

#include <direct.h>
#include <io.h>

fwPlatformString GetAbsoluteCitPath()
{
	static fwPlatformString citizenPath;

	if (!citizenPath.size())
	{
		wchar_t modulePath[512];
		GetModuleFileName(GetModuleHandle(nullptr), modulePath, sizeof(modulePath) / sizeof(wchar_t));

		wchar_t realModulePath[512];

		GetFullPathName(modulePath, _countof(realModulePath), realModulePath, nullptr);

		wchar_t* dirPtr = wcsrchr(realModulePath, L'\\');

		// we do not check if dirPtr happens to be 0, as any valid absolute Win32 file path contains at least one backslash
		dirPtr[1] = '\0';

		citizenPath = realModulePath;
	}

	return citizenPath;
}

fwPlatformString GetAbsoluteGamePath()
{
	static fwPlatformString gamePath;

	if (!gamePath.size())
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		if (GetFileAttributes(fpath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			return L"null";
		}

		wchar_t path[512];

		const wchar_t* pathKey = L"IVPath";

		if (wcsstr(GetCommandLine(), L"cl2"))
		{
			pathKey = L"PathCL2";
		}

		GetPrivateProfileString(L"Game", pathKey, L"", path, _countof(path), fpath.c_str());

		gamePath = path;
		gamePath += L"\\";
	}

	return gamePath;
}

bool IsRunningTests()
{
	wchar_t path[512];
	GetModuleFileName(GetModuleHandle(NULL), path, sizeof(path));

	wchar_t* filenamePart = wcsrchr(path, L'\\');

	filenamePart++;

	return !_wcsnicmp(filenamePart, L"tests_", 6);
}

void CreateDirectoryAnyDepth(const char *path)
{
	char opath[MAX_PATH];
	char *p;
	size_t len;
	strncpy_s(opath, path, sizeof(opath));
	len = strlen(opath);
	if (opath[len - 1] == L'/')
		opath[len - 1] = L'\0';

	for (p = opath; *p; p++)
	{
		if (*p == L'/' || *p == L'\\')
		{
			*p = L'\0';
			if (_access(opath, 0))
				_mkdir(opath);
			*p = L'\\';
		}
	}
	if (_access(opath, 0))
		_mkdir(opath);
}

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(int dwThreadID, char* threadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}