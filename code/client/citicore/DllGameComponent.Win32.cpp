/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "DllGameComponent.h"

#include <Error.h>

#include <winternl.h>
#include <ntstatus.h>

#include <optional>
#include <sstream>

#include "ErrorFormat.Win32.h"
#include <Hooking.Aux.h>

#include <MinHook.h>

static NTSTATUS(NTAPI* g_origRtlRaiseException)(_In_ PEXCEPTION_RECORD ExceptionRecord);
static BOOL(NTAPI* g_origZwQueryDebugFilterState)(_In_ ULONG ComponentId, _In_ ULONG Level);

struct HardErrorScope
{
	HardErrorScope()
		: m_target(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlRaiseException")),
		  m_target2(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "ZwQueryDebugFilterState"))
	{
		ms_curErrScope = this;

		// OSBuildNumber
		auto osBuildNumber = *(WORD*)((char*)NtCurrentTeb()->ProcessEnvironmentBlock + 0x0120);

		// 19H1, Vb and Mn seem to adhere to this memory layout.
		// Not sure about Fe and above, so taking out 20000.
		if (osBuildNumber >= 18362 && osBuildNumber < 19628)
		{
			auto fltUsed = (char*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "_fltused");
			m_oldLdrFlags = *(int*)(fltUsed - 16);

			*(int*)(fltUsed - 16) |= 2; // loader snaps for error
		}

		DisableToolHelpScope thScope;

		MH_Initialize();
		MH_CreateHook(m_target, RtlRaiseExceptionStub, (void**)&g_origRtlRaiseException);
		MH_EnableHook(m_target);

		MH_CreateHook(m_target2, ZwQueryDebugFilterStateStub, (void**)&g_origZwQueryDebugFilterState);
		MH_EnableHook(m_target2);
	}

	~HardErrorScope()
	{
		DisableToolHelpScope thScope;

		MH_DisableHook(m_target);
		MH_RemoveHook(m_target);

		MH_DisableHook(m_target2);
		MH_RemoveHook(m_target2);

		if (m_oldLdrFlags)
		{
			auto fltUsed = (char*)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "_fltused");
			*(int*)(fltUsed - 16) = *m_oldLdrFlags; // loader snaps for error
		}
	}
	
	std::wstring GetErrors()
	{
		return m_messageBuffer.str();
	}

private:
	void* m_target;
	void* m_target2;
	std::optional<int> m_oldLdrFlags;

	static BOOL NTAPI ZwQueryDebugFilterStateStub(_In_ ULONG ComponentId, _In_ ULONG Level)
	{
		if (ms_curErrScope && ComponentId == 0x55)
		{
			return true;
		}

		return g_origZwQueryDebugFilterState(ComponentId, Level);
	}

	static NTSTATUS NTAPI RtlRaiseExceptionStub(PEXCEPTION_RECORD rec)
	{
		if (ms_curErrScope && rec->ExceptionCode == DBG_PRINTEXCEPTION_C)
		{
			auto esv = std::string_view{ (const char*)rec->ExceptionInformation[1], rec->ExceptionInformation[0] - 1 };

			if (esv.find("ERROR") != std::string::npos)
			{
				ms_curErrScope->m_messageBuffer << ToWide(std::string{ esv });
			}

			return STATUS_SUCCESS;
		}

		return g_origRtlRaiseException(rec);
	}

	std::wstringstream m_messageBuffer;
	static HardErrorScope* ms_curErrScope;
};

HardErrorScope* HardErrorScope::ms_curErrScope;

Component* DllGameComponent::CreateComponent()
{
	HardErrorScope scope;
	DisableToolHelpScope thScope;

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	HMODULE hModule = LoadLibrary(MakeRelativeCitPath(m_path).c_str());

	if (!hModule)
	{
		DWORD errorCode = GetLastError();

		// delete caches.xml so the game will be verified
		_wunlink(MakeRelativeCitPath(L"content_index.xml").c_str());

		// sanity check
		if (m_path == L"adhesive.dll" && errorCode == ERROR_PROC_NOT_FOUND)
		{
			HMODULE rosFive = GetModuleHandle(L"ros-patches-five.dll");

			if (rosFive)
			{
				if (GetProcAddress(rosFive, "?IDidntDoNothing@@YAXXZ") == nullptr)
				{
					// vague error to not immediately discourage pirates
					FatalError("Failed to compute buffer alias alignment.");
				}
			}
		}

		auto errors = ToNarrow(scope.GetErrors());
		std::string addtlInfo;

		if (!errors.empty())
		{
			addtlInfo = fmt::sprintf("\n\nAdditional information:\n%s", errors);
		}

		FatalError("Could not load component %s - Windows error code %d. %s%s", converter.to_bytes(m_path).c_str(), errorCode, win32::FormatMessage(errorCode), addtlInfo);

		return nullptr;
	}

	auto createComponent = (Component*(__cdecl*)())GetProcAddress(hModule, "CreateComponent");

	if (!createComponent)
	{
		const char* additionalInfo = "";

		if (m_path.find(L"scripthookv") != std::string::npos)
		{
			additionalInfo = " You likely overwrote scripthookv.dll from Cfx with a non-Cfx version of it. Delete content_index.xml to restore from this heinous act.";
		}

		FatalError("Could not find entry point CreateComponent in component %s.%s", converter.to_bytes(m_path).c_str(), additionalInfo);
	}

	return createComponent ? createComponent() : nullptr;
}

void DllGameComponent::ReadManifest()
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	HMODULE hModule = LoadLibraryEx(MakeRelativeCitPath(m_path).c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);

	if (!hModule)
	{
		if (m_path == L"adhesive.dll")
		{
			m_path = L"sticky.dll";

			hModule = LoadLibraryEx(MakeRelativeCitPath(m_path).c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
		}
	}

	if (!hModule)
	{
		DWORD errorCode = GetLastError();

		// delete caches.xml so the game will be verified
		_wunlink(MakeRelativeCitPath(L"content_index.xml").c_str());

		FatalError("Could not load component manifest %s - Windows error code %d. %s", converter.to_bytes(m_path).c_str(), errorCode, win32::FormatMessage(errorCode));

		return;
	}

	HRSRC hResource = FindResource(hModule, L"FXCOMPONENT", MAKEINTRESOURCE(115));

	if (hResource)
	{
		auto resSize = SizeofResource(hModule, hResource);
		auto resData = LoadResource(hModule, hResource);

		auto resPtr = static_cast<const char*>(LockResource(resData));

		// copy into a zero-terminated std::string
		std::string resourceString(resPtr, resSize);

		m_document.Parse(resourceString.c_str());
	}
	else
	{
		// delete caches.xml so the game will be verified
		_wunlink(MakeRelativeCitPath(L"content_index.xml").c_str());

		const char* additionalInfo = "";

		if (m_path.find(L"scripthookv") != std::string::npos)
		{
			additionalInfo = " You likely overwrote scripthookv.dll from FiveM with a non-FiveM version of it. Delete content_index.xml to restore from this heinous act.";
		}
		FatalError("Component manifest mismatch in component %s.%s", converter.to_bytes(m_path).c_str(), additionalInfo);
	}

	FreeLibrary(hModule);
}
