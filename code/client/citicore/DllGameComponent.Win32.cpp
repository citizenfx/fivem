/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "DllGameComponent.h"

#include <Error.h>

Component* DllGameComponent::CreateComponent()
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	HMODULE hModule = LoadLibrary(MakeRelativeCitPath(m_path).c_str());

	if (!hModule)
	{
		DWORD errorCode = GetLastError();

		// delete caches.xml so the game will be verified
		_wunlink(MakeRelativeCitPath(L"caches.xml").c_str());

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

		FatalError("Could not load component %s - Windows error code %d.", converter.to_bytes(m_path).c_str(), errorCode);

		return nullptr;
	}

	auto createComponent = (Component*(__cdecl*)())GetProcAddress(hModule, "CreateComponent");

	if (!createComponent)
	{
		const char* additionalInfo = "";

		if (m_path.find(L"scripthookv") != std::string::npos)
		{
			additionalInfo = " You likely overwrote scripthookv.dll from Cfx with a non-Cfx version of it. Delete caches.xml to restore from this heinous act.";
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
		DWORD errorCode = GetLastError();

		// delete caches.xml so the game will be verified
		_wunlink(MakeRelativeCitPath(L"caches.xml").c_str());

		FatalError("Could not load component manifest %s - Windows error code %d.", converter.to_bytes(m_path).c_str(), errorCode);

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
		_wunlink(MakeRelativeCitPath(L"caches.xml").c_str());

		const char* additionalInfo = "";

		if (m_path.find(L"scripthookv") != std::string::npos)
		{
			additionalInfo = " You likely overwrote scripthookv.dll from FiveM with a non-FiveM version of it. Delete caches.xml to restore from this heinous act.";
		}
		FatalError("Component manifest mismatch in component %s.%s", converter.to_bytes(m_path).c_str(), additionalInfo);
	}

	FreeLibrary(hModule);
}