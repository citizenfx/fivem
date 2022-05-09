/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/
#include "StdInc.h"
#include <boost/algorithm/string.hpp>

#include "Hooking.h"
#include "DrawCommands.h"

#pragma comment(lib, "version.lib")

bool IsValidGraphicsLibrary(const std::wstring& path)
{
	DWORD versionInfoSize = GetFileVersionInfoSize(path.c_str(), nullptr);

	if (versionInfoSize)
	{
		std::vector<uint8_t> versionInfo(versionInfoSize);

		if (GetFileVersionInfo(path.c_str(), 0, versionInfo.size(), &versionInfo[0]))
		{
			struct LANGANDCODEPAGE
			{
				WORD wLanguage;
				WORD wCodePage;
			} * lpTranslate;

			UINT cbTranslate = 0;

			// Read the list of languages and code pages.

			VerQueryValue(&versionInfo[0],
			TEXT("\\VarFileInfo\\Translation"),
			(LPVOID*)&lpTranslate,
			&cbTranslate);

			if (cbTranslate > 0)
			{
				void* productNameBuffer;
				UINT productNameSize = 0;

				VerQueryValue(&versionInfo[0],
				va(L"\\StringFileInfo\\%04x%04x\\ProductName", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage),
				&productNameBuffer,
				&productNameSize);

				void* fixedInfoBuffer;
				UINT fixedInfoSize = 0;

				VerQueryValue(&versionInfo[0], L"\\", &fixedInfoBuffer, &fixedInfoSize);

				VS_FIXEDFILEINFO* fixedInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(fixedInfoBuffer);

				if (productNameSize > 0 && fixedInfoSize > 0)
				{
					if (wcscmp((wchar_t*)productNameBuffer, L"ReShade") == 0)
					{
						// ReShade <3.1 is invalid
						if (fixedInfo->dwProductVersionMS < 0x30001)
						{
							return false;
						}

						return true;
					}
					else if (wcscmp((wchar_t*)productNameBuffer, L"ENBSeries") == 0)
					{
						// ENBSeries <0.3.8.7 is invalid
						if (fixedInfo->dwProductVersionMS < 0x3 || (fixedInfo->dwProductVersionMS == 3 && fixedInfo->dwProductVersionLS < 0x80007))
						{
							return false;
						}

						// so is ENBSeries from 2019
						void* copyrightBuffer;
						UINT copyrightSize = 0;

						VerQueryValue(&versionInfo[0],
						va(L"\\StringFileInfo\\%04x%04x\\LegalCopyright", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage),
						&copyrightBuffer,
						&copyrightSize);

						if (copyrightSize > 0)
						{
							if (wcsstr((wchar_t*)copyrightBuffer, L"2019, Boris"))
							{
								return false;
							}
						}

						return true;
					}
				}
			}
		}

		// if the file exists, but it's not one of our 'whitelisted' known-good variants, load it from system
		// this will break any third-party graphics mods that _aren't_ mainline ReShade or ENBSeries *entirely*, but will hopefully
		// fix initialization issues people have with the behavior instead. (2020-04-18)
		return false;
	}

	return true;
}

static HMODULE LoadLibraryAHook(const char* libName)
{
	if (strcmp(libName, "dxgi.dll") == 0)
	{
		auto refPath = MakeRelativeGamePath(L"dxgi.dll");

		if (GetFileAttributes(refPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			if (IsValidGraphicsLibrary(refPath))
			{
				auto hMod = LoadLibraryW(refPath.c_str());

				if (hMod)
				{
					return hMod;
				}
			}
		}
	}

	return LoadLibraryA(libName);
}

void ScanForReshades()
{
	bool found = false;

	std::vector<std::wstring_view> reshadeFiles{ L"d3d8.dll",
		L"d3d9.dll",
		L"d3d10.dll",
		L"d3d10_1.dll",
		L"d3d11.dll",
		L"dxgi.dll" };

	// Try loading all dll files in the directory, that are in the list
	for (auto graphicsDll : reshadeFiles)
	{
		auto dllPath = MakeRelativeGamePath(std::wstring{ graphicsDll });
		if (GetFileAttributesW(dllPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			if (IsValidGraphicsLibrary(dllPath))
			{
				LoadLibraryW(dllPath.c_str());

				trace("Loaded graphics mod: %s\n", ToNarrow(dllPath));
			}
			else
			{
				trace("Ignored graphics mod: %s\n", ToNarrow(dllPath));
			}

			found = true;
		}
	}

	if (found)
	{
		hook::iat("kernel32.dll", LoadLibraryAHook, "LoadLibraryA");
	}
}

static HookFunction hookFunction([]()
{
	ScanForReshades();
});
