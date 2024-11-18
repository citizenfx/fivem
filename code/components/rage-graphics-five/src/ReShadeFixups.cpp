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

#include "CoreConsole.h"

#include <PureModeState.h>

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
						// ReShade == 5.9 causes heap corruption basically *all the time* so we hard-block it
						else if (fixedInfo->dwProductVersionMS == 0x50009 && fixedInfo->dwProductVersionLS < 0x10000)
						{
							console::Printf("script:reshade", "Blocked load of ReShade version 5.9, it causes crashes in RtlReportFatalFailure.\nUpgrade to 5.9.1 or below to be able to use ReShade.\n");
							return false;
						}
						// as is ReShade v5+ because of an unknown crash (unless setting an override)
						else if (fixedInfo->dwProductVersionMS >= 0x50000)
						{
							std::wstring fpath = MakeRelativeCitPath(L"VMP.ini");

							bool disableReShade5 = true;
							const char* computername = getenv("COMPUTERNAME");
							if (!computername)
							{
								computername = "a";
							}

							uint32_t hash = HashString(computername);
							auto comparand = fmt::sprintf(L"ID:%08x acknowledged that ReShade 5.x has a bug that will lead to game crashes", hash);

							if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
							{
								wchar_t reshadeToggle[256] = { 0 };
								GetPrivateProfileStringW(L"Addons", L"ReShade5", L"", reshadeToggle, std::size(reshadeToggle) - 1, fpath.c_str());

								disableReShade5 = wcscmp(reshadeToggle, comparand.c_str()) != 0;
							}

							if (disableReShade5)
							{
								static bool shown = false;

								if (!shown)
								{
									console::Printf("script:reshade", "Blocked load of ReShade version 5 or higher - it has a bug that will lead to game crashes in GPU drivers or d3d11.dll.\n"
																	  "If you want to force it to load anyway, add the following section to ^2%s:^7\n"
																	  "    [Addons]\n    ReShade5=%s\n\n"
																	  "Note that no support is provided for this and that you should contact the author of ReShade for assistance.\n",
									ToNarrow(fpath),
									ToNarrow(comparand));

									shown = true;
								}

								return false;
							}
						}

						// in-process GPU is incompatible with ReShade (it'll swap out the D3D device underneath, failing a check in ANGLE)
						// this was fixed in 5.9 so allow it if the version is above such
						if (fixedInfo->dwProductVersionMS < 0x50009)
						{
							static ConVar<bool> nuiUseInProcessGpu("nui_useInProcessGpu", ConVar_Archive, false);

							if (nuiUseInProcessGpu.GetValue())
							{
								console::Printf("script:reshade", "Blocked load of ReShade, as it is incompatible with NUI in-process GPU. Update ReShade to version 5.9.1 or above to be able to use it.\n");

								return false;
							}
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
		auto refPath = MakeRelativeCitPath(L"plugins/dxgi.dll");

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

	// try loading all dll files *from plugins/*
	for (auto graphicsDll : reshadeFiles)
	{
		auto dllPath = MakeRelativeCitPath(L"plugins/") + std::wstring{ graphicsDll };
		if (GetFileAttributesW(dllPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			if (IsValidGraphicsLibrary(dllPath))
			{
				auto hMod = LoadLibraryW(dllPath.c_str());

				if (hMod)
				{
					// pin the module since it appears sometimes ReShade or similar unloads despite being hooked
					HMODULE _;
					GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCWSTR)hMod, &_);
				}

				trace("Loaded graphics mod: %s\n", ToNarrow(dllPath));
			}
			else
			{
				trace("Ignored graphics mod: %s\n", ToNarrow(dllPath));
			}

			found = true;
		}
	}

	// warn about ignored DLLs
	for (auto graphicsDll : reshadeFiles)
	{
		auto dllPath = MakeRelativeGamePath(std::wstring{ graphicsDll });
		if (GetFileAttributesW(dllPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			console::Printf("script:dll", "Ignored graphics mod: %s - these should go in plugins/ now!\n", ToNarrow(dllPath));
		}
	}

	if (found)
	{
		hook::iat("kernel32.dll", LoadLibraryAHook, "LoadLibraryA");
	}
}

static HookFunction hookFunction([]()
{
	if (fx::client::GetPureLevel() == 0)
	{
		ScanForReshades();
	}
});
