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

#include <CfxState.h>
#include <HostSharedData.h>

#include <Error.h>
#include <shared_mutex>

fwPlatformString GetAbsoluteCitPath()
{
	auto getCitizenPath = []()
	{
		fwPlatformString citizenPath;

#ifndef IS_FXSERVER
		HostSharedData<CfxState> initState("CfxInitState");

		citizenPath = initState->GetInitPath();

		// is this a new install, if so, migrate to subdirectory-based Citizen
		if (initState->ranPastInstaller)
		{
			if (GetFileAttributes((citizenPath + L"CoreRT.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
			{
#ifdef IS_RDR3
				if (!CreateDirectory((citizenPath + L"RedM.app").c_str(), nullptr))
#else
				if (!CreateDirectory((citizenPath + L"DevotedStudios.app").c_str(), nullptr))
#endif
				{
					DWORD error = GetLastError();

					if (error != ERROR_ALREADY_EXISTS)
					{
						ExitProcess(1);
					}
				}
			}
		}

		// is this subdirectory-based Citizen? if so, append the subdirectory
		if (GetFileAttributes((citizenPath + L"CoreRT.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			std::wstring subPath = citizenPath +
#ifdef IS_RDR3
								   L"RedM.app";
#else
								   L"DevotedStudios.app";
#endif

			if (GetFileAttributes(subPath.c_str()) != INVALID_FILE_ATTRIBUTES && (GetFileAttributes(subPath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				citizenPath = subPath + L"\\";
			}
		}
#else
		wchar_t modulePath[512];
		GetModuleFileNameW(GetModuleHandle(nullptr), modulePath, sizeof(modulePath) / sizeof(wchar_t));

		wchar_t realModulePath[512];

		GetFullPathNameW(modulePath, _countof(realModulePath), realModulePath, nullptr);

		wchar_t* dirPtr = wcsrchr(realModulePath, L'\\');

		// we do not check if dirPtr happens to be 0, as any valid absolute Win32 file path contains at least one backslash
		dirPtr[1] = '\0';

		citizenPath = realModulePath;
#endif

		return citizenPath;
	};

	static fwPlatformString citizenPaths[2] =
	{
		getCitizenPath(),
		L"",
	};

	static int curCitizenPath = 0;

#ifndef IS_FXSERVER
	static DWORD ranPastInstaller;

	// if we haven't run yet
	if (InterlockedCompareExchange(&ranPastInstaller, 0, 0) == 0)
	{
		// grab the ranPastInstaller state from here
		HostSharedData<CfxState> initState("CfxInitState");

		if (initState->ranPastInstaller)
		{
			// if this is the thread trying to update
			if (InterlockedCompareExchange(&ranPastInstaller, 1, 0) == 0)
			{
				citizenPaths[1] = getCitizenPath();
				curCitizenPath = 1;
			}
		}
	}
#endif

	return citizenPaths[curCitizenPath];
}

fwPlatformString GetAbsoluteGamePath()
{
#ifndef IS_FXSERVER
	HostSharedData<CfxState> initState("CfxInitState");

	if (!initState->gameDirectory[0])
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

		wcscpy_s(initState->gameDirectory, path);
		wcscat_s(initState->gameDirectory, L"\\");
	}

	return initState->gameDirectory;
#else
	return {};
#endif
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
			if (_waccess(ToWide(opath).c_str(), 0))
				_wmkdir(ToWide(opath).c_str());
			*p = L'\\';
		}
	}
	if (_waccess(ToWide(opath).c_str(), 0))
		_wmkdir(ToWide(opath).c_str());
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

void DoNtRaiseException(EXCEPTION_RECORD* record);

void SetThreadName(int dwThreadID, const char* threadName)
{
	auto SetThreadDescription = (HRESULT(WINAPI*)(HANDLE, PCWSTR))GetProcAddress(GetModuleHandle(L"kernelbase.dll"), "SetThreadDescription");

	if (SetThreadDescription)
	{
		HANDLE hThread = (dwThreadID < 0) ? GetCurrentThread() : OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, dwThreadID);

		if (hThread != NULL)
		{
			SetThreadDescription(hThread, ToWide(threadName).c_str());

			if (dwThreadID >= 0)
			{
				CloseHandle(hThread);
			}
		}
	}

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	if (CoreIsDebuggerPresent())
	{
		EXCEPTION_RECORD record;
		record.ExceptionAddress = reinterpret_cast<PVOID>(_ReturnAddress());
		record.ExceptionCode = MS_VC_EXCEPTION;
		record.ExceptionFlags = 0;
		record.NumberParameters = sizeof(info) / sizeof(ULONG_PTR);
		memcpy(record.ExceptionInformation, &info, sizeof(info));
		record.ExceptionRecord = &record;

		DoNtRaiseException(&record);
	}
}

void AddCrashometryV(const std::string& key, const std::string& format, fmt::printf_args value)
{
	std::string formatted = fmt::vsprintf(format, value);

	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\crashometry").c_str(), L"ab");

	if (f)
	{
		uint32_t keyLen = key.size();
		uint32_t valLen = formatted.size();

		fwrite(&keyLen, 1, sizeof(keyLen), f);
		fwrite(&valLen, 1, sizeof(valLen), f);

		fwrite(key.c_str(), 1, key.size(), f);
		fwrite(formatted.c_str(), 1, formatted.size(), f);

		fclose(f);
	}
}

#if !defined(COMPILING_SHARED_LIBC)

extern "C" void Win32TrapAndJump64();

#if defined(_M_IX86)
static void __declspec(naked) Win32TrapAndJumpX86()
{
	__asm {
		pushfd
		or dword ptr [esp], 100h
		popfd

		jmp dword ptr fs:[1Ch]
	}
}
#endif

void __cdecl _wwassert(
	_In_z_ wchar_t const* _Message,
	_In_z_ wchar_t const* _File,
	_In_   unsigned       _Line
)
{
	FatalErrorNoExcept("Assertion failure: %s\nFile: %s, Line: %d", ToNarrow(_Message), ToNarrow(_File), _Line);

#if defined(_M_AMD64)
	__writegsqword(0x38, (uintptr_t)_ReturnAddress());
	*(void**)_AddressOfReturnAddress() = &Win32TrapAndJump64;
#elif defined(_M_IX86)
	__writefsdword(0x1C, (uintptr_t)_ReturnAddress());
	*(void**)_AddressOfReturnAddress() = &Win32TrapAndJumpX86;
#else
#error No architecture for asserts here.
#endif
}
#endif
