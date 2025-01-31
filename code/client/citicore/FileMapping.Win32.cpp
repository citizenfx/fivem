/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifndef IS_FXSERVER
#include <jitasm.h>
#include "Hooking.Aux.h"
#include <minhook.h>

#include <ROSSuffix.h>

#pragma comment(lib, "ntdll.lib")

#include <shlobj.h>

#include <winternl.h>

#pragma comment(lib, "comctl32.lib")
#include <commctrl.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <CrossBuildRuntime.h>

typedef wchar_t*(*MappingFunctionType)(const wchar_t*, void*(*)(size_t));

static MappingFunctionType g_mappingFunction;

static NTSTATUS(NTAPI*g_origLoadDll)(const wchar_t*, uint32_t*, UNICODE_STRING*, HANDLE*);

static bool g_d3dx11;

// this uses SHGetFolderPathW since SC SDK does as well
static std::wstring GetRoot(int folder)
{
	wchar_t pathRef[MAX_PATH];
	if (FAILED(SHGetFolderPathW(NULL, folder, NULL, SHGFP_TYPE_CURRENT, pathRef)))
	{
		return L"C:\\dummy_path\\";
	}

	return pathRef;
}

static std::wstring g_documentsRoot = GetRoot(CSIDL_MYDOCUMENTS);
static std::wstring g_localAppDataRoot = GetRoot(CSIDL_LOCAL_APPDATA);
static std::wstring g_programFilesRoot = GetRoot(CSIDL_PROGRAM_FILES);
static std::wstring g_programFilesX86Root = GetRoot(CSIDL_PROGRAM_FILESX86);
static std::wstring g_programDataRoot = GetRoot(CSIDL_COMMON_APPDATA);

static std::wstring g_rsgDocumentsRoot = g_documentsRoot + L"\\Rockstar Games";
static std::wstring g_scDocumentsRoot = g_documentsRoot + L"\\Rockstar Games\\Social Club";
static std::wstring g_launcherDocumentsRoot = g_documentsRoot + L"\\Rockstar Games\\Launcher";
static std::wstring g_launcherAppDataRoot = g_localAppDataRoot + L"\\Rockstar Games\\Launcher";
static std::wstring g_launcherProgramDataRoot = g_programDataRoot + L"\\Rockstar Games\\Launcher";
static std::wstring g_scFilesRoot = g_programFilesRoot + L"\\Rockstar Games\\Social Club";
static std::wstring g_scX86FilesRoot = g_programFilesX86Root + L"\\Rockstar Games\\Social Club";
static std::wstring g_launcherFilesRoot = g_programFilesRoot + L"\\Rockstar Games\\Launcher";

static std::vector<std::wstring> g_socialClubDlls = ([]
{
	std::vector<std::wstring> fns;

	for (const auto fn : {
		 L"cef.pak",
		 L"cef_100_percent.pak",
		 L"cef_200_percent.pak",
		 L"chrome_elf.dll",
		 L"d3dcompiler_47.dll",
		 L"libcef.dll",
		 L"libEGL.dll",
		 L"libGLESv2.dll",
		 L"scui.pak",
		 L"snapshot_blob.bin",
		 L"SocialClubHelper.exe",
		 // RDR3 expects these to exist
#ifndef IS_RDR3
		 L"SocialClubD3D12Renderer.dll",
		 L"SocialClubVulkanLayer.dll",
#endif
		 L"v8_context_snapshot.bin",
		 L"swiftshader\\libEGL.dll",
		 L"swiftshader\\libGLESv2.dll",
		 })
	{
		fns.push_back(fmt::sprintf(L"Social Club\\%s", fn));
	}

	return fns;
})();

static std::wstring MapRedirectedFilename(const wchar_t* origFileName)
{
	//trace("map %s\n", ToNarrow(origFileName));

	for (const auto& fileName : g_socialClubDlls)
	{
		if (StrStrIW(origFileName, fileName.c_str()) != NULL)
		{
			return MakeRelativeCitPath(L"bin\\libEGL.dll");
		}
	}

	if (wcsstr(origFileName, L"autosignin.dat") != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\autosignin.dat");
	}

	if (wcsstr(origFileName, L"signintransfer.dat") != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\signintransfer.dat");
	}

	if ((wcsstr(origFileName, L"d3dx11_43.dll") != nullptr || wcsstr(origFileName, L"D3DX11_43")) && !g_d3dx11)
	{
		return MakeRelativeCitPath(L"bin\\d3dcompiler_43.dll");
	}

	if (wcsstr(origFileName, L"Social Club\\Profiles") != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_profiles") + &wcsstr(origFileName, L"Social Club\\Profiles")[20];
	}

	if (wcsstr(origFileName, L"GTA V\\Profiles") != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\game_profiles") + &wcsstr(origFileName, L"GTA V\\Profiles")[14];
	}

	if (wcsstr(origFileName, L"Red Dead Redemption 2\\Profiles") != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\game_profiles") + &wcsstr(origFileName, L"ion 2\\Profiles")[14];
	}

	if (wcsstr(origFileName, L"version.txt") != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\version_orig.txt");
	}

	if (wcsstr(origFileName, L"PlayGTAV.exe") != nullptr)
	{
		return MakeRelativeCitPath(L"nodontfuckingplaygtav.exe");
	}

	if (wcsstr(origFileName, L"LauncherPatcher.exe") != nullptr)
	{
		return MakeRelativeCitPath(L"nodontfuckingplaygtav.exe");
	}

	// Program Files
	if (wcsstr(origFileName, L"Files\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherFilesRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\launcher") + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	// ProgramData
	if (wcsstr(origFileName, L"Data\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherProgramDataRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_data" ROS_SUFFIX_W) + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	if (wcsstr(origFileName, L"Local\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherAppDataRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_appdata" ROS_SUFFIX_W) + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	if (wcsstr(origFileName, L"Documents\\Rockstar Games\\Social Club") != nullptr || wcsstr(origFileName, g_scDocumentsRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_documents" ROS_SUFFIX_W) + &wcsstr(origFileName, L"Games\\Social Club")[17];
	}

	if (wcsstr(origFileName, L"Documents\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherDocumentsRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_documents" ROS_SUFFIX_W) + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	if (wcsstr(origFileName, L"Documents\\Rockstar Games") != nullptr || wcsstr(origFileName, g_rsgDocumentsRoot.c_str()) != nullptr)
	{
		return g_localAppDataRoot + L"\\" + &wcsstr(origFileName, L"Rockstar Games")[0];
	}

	if (getenv("CitizenFX_ToolMode"))
	{
		if (wcsstr(origFileName, L".lnk"))
		{
			return MakeRelativeCitPath(L"data\\game-storage\\dummy.lnk");
		}

		auto gameDir = []()
		{
			return MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_game" ROS_SUFFIX_W);
		};

		if (wcsstr(origFileName, L".exe.part") != nullptr)
		{
			return MakeRelativeCitPath(L"data\\game-storage\\dummy.exe.part");
		}

		if (wcsstr(origFileName, L"Rockstar Games\\GTA5.exe") != nullptr ||
			wcsstr(origFileName, L"Grand Theft Auto V\\GTA5.exe") != nullptr)
		{
			static thread_local std::wstring s;
			s = MakeRelativeGamePath(L"GTA5.exe");
			origFileName = s.c_str();
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\RDR2.exe") != nullptr || 
				 wcsstr(origFileName, L"Red Dead Redemption 2\\RDR2.exe") != nullptr)
		{
			static thread_local std::wstring s;
			s = MakeRelativeGamePath(L"RDR2.exe");
			origFileName = s.c_str();
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\Games") != nullptr)
		{
			CreateDirectoryW(gameDir().c_str(), NULL);

			return gameDir() + &wcsstr(origFileName, L"ar Games\\Games")[14];
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\Red Dead Redemption 2") != nullptr)
		{
			return gameDir() + &wcsstr(origFileName, L"d Redemption 2")[14];
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\Grand Theft Auto V") != nullptr)
		{
			CreateDirectoryW(gameDir().c_str(), NULL);

			return gameDir() + &wcsstr(origFileName, L"d Theft Auto V")[14];
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\index.bin") != nullptr) // lol
		{
			CreateDirectoryW(gameDir().c_str(), NULL);

			return gameDir() + &wcsstr(origFileName, L"Rockstar Games")[14];
		}
	}

	wchar_t* fileName = g_mappingFunction(origFileName, malloc);

	std::wstring retval(fileName);
	free(fileName);

	return retval;
}

static std::wstring MapRedirectedNtFilename(const wchar_t* origFileName)
{
	std::wstring redirected = MapRedirectedFilename(origFileName);

	if (redirected.find(L"\\\\", 0) == 0)
	{
		return L"\\Device\\Mup\\" + redirected.substr(2);
	}
	else
	{
		return L"\\??\\" + redirected;
	}
}

static bool IsMappedFilename(const std::wstring& fileName)
{
	if (fileName.find(L"Files\\Rockstar Games\\Social Club") != std::string::npos ||
		fileName.find(g_scFilesRoot) != std::string::npos ||
		// weird case: some users have SHGetFolderPathW fail only sometimes into returning "X:\\" with a double backslash
		// leading to "X:\\Rockstar Games\Social Club\" being detected
		fileName.find(L"\\\\Rockstar Games\\Social Club") != std::string::npos)
	{
		return true;
	}

	// hopefully this'll trap most `Program Files (x86)` directories
	if (fileName.find(L"Files (x86)\\Rockstar Games\\Social Club") != std::string::npos ||
		fileName.find(g_scX86FilesRoot) != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"Files\\Rockstar Games\\Launcher") != std::string::npos ||
		fileName.find(g_launcherFilesRoot) != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"Data\\Rockstar Games\\Launcher") != std::string::npos ||
		fileName.find(g_launcherProgramDataRoot) != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"Data\\Local\\Rockstar Games\\Launcher") != std::string::npos ||
		fileName.find(g_launcherAppDataRoot) != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"Documents\\Rockstar Games\\Social Club") != std::string::npos ||
		fileName.find(g_scDocumentsRoot) != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"Documents\\Rockstar Games\\Launcher") != std::string::npos ||
		fileName.find(g_launcherDocumentsRoot) != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"Documents\\Rockstar Games") != std::string::npos ||
		fileName.find(g_rsgDocumentsRoot) != std::string::npos)
	{
		return true;
	}
	
	if (fileName.find(L"autosignin.dat") != std::string::npos)
	{
		return true;
	}

	if (fileName.find(L"signintransfer.dat") != std::string::npos)
	{
		return true;
	}

	if ((wcsstr(fileName.c_str(), L"d3dx11_43.dll") != nullptr || wcsstr(fileName.c_str(), L"D3DX11_43")) && !g_d3dx11)
	{
		return true;
	}

	if (wcsstr(fileName.c_str(), L"Social Club\\Profiles") != nullptr)
	{
		return true;
	}

	if (wcsstr(fileName.c_str(), L"GTA V\\Profiles") != nullptr || wcsstr(fileName.c_str(), L"Redemption 2\\Profiles") != nullptr)
	{
		return true;
	}

	if (wcsstr(fileName.c_str(), L"PlayGTAV.exe") != nullptr)
	{
		return true;
	}

	if (wcsstr(fileName.c_str(), L"LauncherPatcher.exe") != nullptr)
	{
		return true;
	}

	if (getenv("CitizenFX_ToolMode"))
	{
		if (wcsstr(fileName.c_str(), L"Auto V.lnk") != nullptr || wcsstr(fileName.c_str(), L"ion 2.lnk") != nullptr)
		{
			return true;
		}

		if (wcsstr(fileName.c_str(), L".exe.part") != nullptr)
		{
			return true;
		}

		if (wcsstr(fileName.c_str(), L"Rockstar Games\\Red Dead Redemption 2") != nullptr)
		{
			return true;
		}
		else if (wcsstr(fileName.c_str(), L"Rockstar Games\\Grand Theft Auto V") != nullptr)
		{
			return true;
		}
		else if (wcsstr(fileName.c_str(), L"Rockstar Games\\Games") != nullptr)
		{
			return true;
		}
		else if (wcsstr(fileName.c_str(), L"Rockstar Games\\index.bin") != nullptr)
		{
			return true;
		}
		else if (wcsstr(fileName.c_str(), L"Rockstar Games\\GTA5.exe") != nullptr)
		{
			return true;
		}
		else if (wcsstr(fileName.c_str(), L"Rockstar Games\\RDR2.exe") != nullptr)
		{
			return true;
		}
	}

	return false;
}



static thread_local std::tuple<uint16_t, uint16_t, uint16_t, uint16_t> g_nextFileVersion;

static BOOL (WINAPI* g_origVerQueryValueW)(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen);

static BOOL WINAPI VerQueryValueWStub(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen)
{
	auto retval = g_origVerQueryValueW(pBlock, lpSubBlock, lplpBuffer, puLen);

	if (retval)
	{
		if (memcmp(lpSubBlock, L"\\", sizeof(wchar_t) * 2) == 0)
		{
			if (g_nextFileVersion != std::make_tuple<uint16_t, uint16_t, uint16_t, uint16_t>(0, 0, 0, 0))
			{
				auto buffer = (VS_FIXEDFILEINFO*)*lplpBuffer;
				buffer->dwFileVersionMS = (std::get<0>(g_nextFileVersion) << 16) | std::get<1>(g_nextFileVersion);
				buffer->dwFileVersionLS = (std::get<2>(g_nextFileVersion) << 16) | std::get<3>(g_nextFileVersion);

				g_nextFileVersion = { 0, 0, 0, 0 };
			}
		}
	}

	return retval;
}

static DWORD(WINAPI* g_origGetFileVersionInfoSizeW)(_In_ LPCWSTR lptstrFilename, _Out_opt_ LPDWORD lpdwHandle);

static DWORD WINAPI GetFileVersionInfoSizeWStub(_In_ LPCWSTR lptstrFilename, _Out_opt_ LPDWORD lpdwHandle)
{
	std::wstring fileName = MapRedirectedFilename(lptstrFilename);

	return g_origGetFileVersionInfoSizeW(fileName.c_str(), lpdwHandle);
}

static BOOL(WINAPI* g_origGetFileVersionInfoW)(_In_ LPCWSTR lptstrFilename, _Reserved_ DWORD dwHandle, _In_ DWORD dwLen, _Out_writes_bytes_(dwLen) LPVOID lpData);

static BOOL WINAPI GetFileVersionInfoWStub(_In_ LPCWSTR lptstrFilename, _Reserved_ DWORD dwHandle, _In_ DWORD dwLen, _Out_writes_bytes_(dwLen) LPVOID lpData)
{
	std::wstring fileName = MapRedirectedFilename(lptstrFilename);

	BOOL retval = g_origGetFileVersionInfoW(fileName.c_str(), dwHandle, dwLen, lpData);

	if (retval)
	{
		if (StrStrIW(lptstrFilename, L"Social Club\\SocialClub") != NULL)
		{
			g_nextFileVersion = {2, 0, 9, 0};
		}
		else if (StrStrIW(lptstrFilename, L"Social Club\\libcef.dll") != NULL)
		{
			g_nextFileVersion = {85, 3, 9, 0};
		}
	}

	return retval;
}

static DWORD(WINAPI* g_origGetFileVersionInfoSizeA)(_In_ LPCSTR lptstrFilename, _Out_opt_ LPDWORD lpdwHandle);

static DWORD WINAPI GetFileVersionInfoSizeAStub(_In_ LPCSTR lptstrFilename, _Out_opt_ LPDWORD lpdwHandle)
{
	std::wstring fileName = MapRedirectedFilename(ToWide(lptstrFilename).c_str());

	return g_origGetFileVersionInfoSizeA(ToNarrow(fileName).c_str(), lpdwHandle);
}

static BOOL(WINAPI* g_origGetFileVersionInfoA)(_In_ LPCSTR lptstrFilename, _Reserved_ DWORD dwHandle, _In_ DWORD dwLen, _Out_writes_bytes_(dwLen) LPVOID lpData);

static BOOL WINAPI GetFileVersionInfoAStub(_In_ LPCSTR lptstrFilename, _Reserved_ DWORD dwHandle, _In_ DWORD dwLen, _Out_writes_bytes_(dwLen) LPVOID lpData)
{
	std::wstring fileName = MapRedirectedFilename(ToWide(lptstrFilename).c_str());

	return g_origGetFileVersionInfoA(ToNarrow(fileName).c_str(), dwHandle, dwLen, lpData);
}

static HANDLE(WINAPI* g_origCreateFileW)(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile);

static HANDLE WINAPI CreateFileWStub(_In_ LPCWSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return g_origCreateFileW(fileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static BOOL(WINAPI* g_origGetFileAttributesExW)(_In_ LPCWSTR lpFileName, _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId, _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation);

static BOOL WINAPI GetFileAttributesExWStub(_In_ LPCWSTR lpFileName, _In_ GET_FILEEX_INFO_LEVELS fInfoLevelId, _Out_writes_bytes_(sizeof(WIN32_FILE_ATTRIBUTE_DATA)) LPVOID lpFileInformation)
{
	std::wstring fileName = MapRedirectedFilename(lpFileName);

	return g_origGetFileAttributesExW(fileName.c_str(), fInfoLevelId, lpFileInformation);
}

#pragma comment(lib, "version")

static bool CheckGraphicsLibrary(const std::wstring& realPath)
{
	auto path = L"\\\\?\\" + realPath;

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
							return true;
						}

						return false;
					}
				}
			}
		}
	}

	return false;
}

NTSTATUS NTAPI LdrLoadDllStub(const wchar_t* fileName, uint32_t* flags, UNICODE_STRING* moduleName, HANDLE* handle)
{
	UNICODE_STRING newString;
	std::wstring moduleNameStr(moduleName->Buffer, moduleName->Length / sizeof(wchar_t));

	if (moduleNameStr.find(L"socialclub.dll") != std::string::npos ||
		// check .exe files as well for Wine GetFileVersionInfo* calls
		moduleNameStr.find(L".exe") != std::string::npos)
	{
		moduleNameStr = MapRedirectedFilename(moduleNameStr.c_str());

		RtlInitUnicodeString(&newString, moduleNameStr.c_str());

		moduleName = &newString;
	}

	std::transform(moduleNameStr.begin(), moduleNameStr.end(), moduleNameStr.begin(), ::tolower);

	if (moduleNameStr.find(L"dxgi.dll") != std::string::npos)
	{
		if (CheckGraphicsLibrary(moduleNameStr))
		{
			static wchar_t systemPath[512];
			GetSystemDirectory(systemPath, _countof(systemPath));
			wcscat_s(systemPath, L"\\dxgi.dll");

			RtlInitUnicodeString(&newString, systemPath);

			moduleName = &newString;
		}
	}

	// MTL bits do not like nvapi64.dll, maybe
	if (moduleNameStr.find(L"nvapi64.dll") != std::string::npos)
	{
		if (wcsstr(GetCommandLineW(), L"ros:launcher"))
		{
			return 0xC0000428;
		}
	}

	// anything in this if statement **has to be lowercase**, see line above
	if (moduleNameStr.find(L"fraps64.dll") != std::string::npos || moduleNameStr.find(L"avghooka.dll") != std::string::npos ||
		// apparently crashes NUI
		moduleNameStr.find(L"bdcam64.dll") != std::string::npos ||
		// ASUS/A-Volute/Nahimic audio software
		// broken again as of 2021-07 updates to A-Volute
		moduleNameStr.find(L"a-volute") != std::string::npos || moduleNameStr.find(L"audiodevprops") != std::string::npos ||
		// Canon camera software
		moduleNameStr.find(L"\\edsdk.dll") != std::string::npos ||
		// Microsoft camera software
		moduleNameStr.find(L"\\lcproxy.ax") != std::string::npos ||
		// 'PlayClaw' recently started becoming top crasher, no symbols so away with it
		moduleNameStr.find(L"playclawhook64.dll") != std::string::npos ||
		// lots of crashes occur in the DiscordApp overlay
		//moduleNameStr.find(L"overlay.x64.dll") != std::string::npos ||
		// new DiscordApp overlay name :/
		// readded as of 2021-07 since it breaks WM_INPUT messages
		moduleNameStr.find(L"discordhook64.dll") != std::string::npos ||
		// HideMyIP, causes LoopbackTcpServer and libuv to crash
		moduleNameStr.find(L"hmipcore64.dll") != std::string::npos ||
		// NVIDIA SHARE/ShadowPlay capture DLL, high correlation with ERR_GFX_D3D_INIT failures
		moduleNameStr.find(L"nvspcap") != std::string::npos ||
		// Proxifier, causes LoopbackTcpServer crashes
		//moduleNameStr.find(L"prxernsp.dll") != std::string::npos ||
		//moduleNameStr.find(L"prxerdrv.dll") != std::string::npos ||
		// Steam 'crashhandler64.dll', optional library that has a broken GetProcessHeap call
		moduleNameStr.find(L"crashhandler64.dll") != std::string::npos ||
		// Ad Muncher, causes LoopbackTcpServer to crash
		moduleNameStr.find(L"am64-34121.dll") != std::string::npos ||
		// some unknown force feedback driver ('USB Vibration')
		moduleNameStr.find(L"ezfrd64.dll") != std::string::npos ||
#if defined(IS_RDR3)
		// 'Overwolf', corrupts memory on RDR (and is generally undesirable)
		moduleNameStr.find(L"owclient.dll") != std::string::npos ||
		moduleNameStr.find(L"ow-graphics-vulkan.dll") != std::string::npos ||
		moduleNameStr.find(L"ow-graphics-hook64.dll") != std::string::npos ||
		// 'Twitch Native Overlay' crashes Vulkan drivers again ('woo another Vulkan layer crash')
		moduleNameStr.find(L"twitchnativeoverlay64.dll") != std::string::npos ||
		// Mirillis Action! also has a broken Vulkan layer (seriously?)
		moduleNameStr.find(L"mirillisactionvulkanlayer.dll") != std::string::npos ||
#endif
		// 'Lenovo Nerve Center'/'Lenovo Artery', leads to chrome_elf.dll crashing
		moduleNameStr.find(L"gm_gametooldll_x64.dll") != std::string::npos ||
#if !defined(IS_RDR3)
		// VulkanRT loader, we don't use Vulkan, CEF does (to 'collect info'), and this crashes a lot of Vulkan drivers
		moduleNameStr.find(L"vulkan-1.dll") != std::string::npos ||
#else
		// VulkanRT loader, we don't use Vulkan, CEF does (to 'collect info'), and this crashes a lot of Vulkan drivers
		(moduleNameStr.find(L"vulkan-1.dll") != std::string::npos && getenv("CitizenFX_ToolMode")) ||
#endif
		// omen gaming hub (HP's software) - it crashes everyone that has this software installed for unknown reasons
		moduleNameStr.find(L"omen_camera_x64.dll") != std::string::npos ||
		false
	)
	{
		// STATUS_INVALID_IMAGE_HASH will be handled in win32kfull.sys!xxxLoadHmodIndex as a 'don't even try again'
		// without this, any module loaded as windows hook or such will forever load, slowing down win32k system-wide
		return 0xC0000428;
	}

	return g_origLoadDll(fileName, flags, moduleName, handle);
}

LONG (WINAPI* g_origRegOpenKeyExW)(HKEY key, const wchar_t* subKey, DWORD options, REGSAM samDesired, PHKEY outKey);

void LSP_InitializeHooks();

LONG WINAPI RegOpenKeyExWStub(HKEY key, const wchar_t* subKey, DWORD options, REGSAM samDesired, PHKEY outKey)
{
	if (subKey && wcsstr(subKey, L"Rockstar Games Social Club"))
	{
		//LONG status = RegCreateKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\Social Club", 0, nullptr, 0, KEY_READ, nullptr, outKey, nullptr);
		auto setValue = [&] (const wchar_t* name, const wchar_t* keyString)
		{
			RegSetKeyValue(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\Social Club", name, REG_SZ, keyString, (wcslen(keyString) * 2) + 2);
		};

		setValue(L"InstallFolder", MakeRelativeCitPath(L"data\\game-storage\\ros_1241").c_str());
		setValue(L"InstallLang", L"1033");
		setValue(L"Version", L"1.2.4.1");

        LONG status = g_origRegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\CitizenFX\\Social Club", 0, KEY_READ, outKey);

		return status;
	}

	return g_origRegOpenKeyExW(key, subKey, options, samDesired, outKey);
}

static DWORD g_tlsHandle;

struct TlsState
{
	bool inCreateFile;
	bool inOpenFile;
	bool inDeleteFile;
	bool inQueryAttributes;
};

static TlsState* GetTls()
{
	auto data = TlsGetValue(g_tlsHandle);

	if (!data)
	{
		data = HeapAlloc(GetProcessHeap(), 0, sizeof(TlsState));
		memset(data, 0, sizeof(TlsState));

		TlsSetValue(g_tlsHandle, data);
	}

	return reinterpret_cast<TlsState*>(data);
}

NTSTATUS(WINAPI* g_origNtCreateFile)(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, PLARGE_INTEGER allocationSize,
									 ULONG fileAttributes, ULONG shareAccess, ULONG createDisposition, ULONG createOptions, PVOID eaBuffer, ULONG eaLength);

NTSTATUS WINAPI NtCreateFileStub(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, PLARGE_INTEGER allocationSize,
								 ULONG fileAttributes, ULONG shareAccess, ULONG createDisposition, ULONG createOptions, PVOID eaBuffer, ULONG eaLength)
{
	auto tls = GetTls();

	if (!tls->inCreateFile)
	{
		tls->inCreateFile = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		tls->inCreateFile = false;

		NTSTATUS retval = g_origNtCreateFile(fileHandle, desiredAccess, &attributes, ioBlock, allocationSize, fileAttributes, shareAccess, createDisposition, createOptions, eaBuffer, eaLength);

		return retval;
	}

	return g_origNtCreateFile(fileHandle, desiredAccess, objectAttributes, ioBlock, allocationSize, fileAttributes, shareAccess, createDisposition, createOptions, eaBuffer, eaLength);
}

NTSTATUS(WINAPI* g_origNtOpenFile)(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, ULONG shareAccess, ULONG openOptions);

NTSTATUS WINAPI NtOpenFileStub(PHANDLE fileHandle, ACCESS_MASK desiredAccess, POBJECT_ATTRIBUTES objectAttributes, PIO_STATUS_BLOCK ioBlock, ULONG shareAccess, ULONG openOptions)
{
	auto tls = GetTls();

	if (!tls->inOpenFile)
	{
		tls->inOpenFile = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		tls->inOpenFile = false;

		NTSTATUS retval = g_origNtOpenFile(fileHandle, desiredAccess, &attributes, ioBlock, shareAccess, openOptions);

		return retval;
	}

	return g_origNtOpenFile(fileHandle, desiredAccess, objectAttributes, ioBlock, shareAccess, openOptions);
}

typedef struct _FILE_RENAME_INFORMATION
{
	union
	{
		BOOLEAN ReplaceIfExists; // FileRenameInformation
		ULONG Flags; // FileRenameInformationEx
	} DUMMYUNIONNAME;
	HANDLE RootDirectory;
	ULONG FileNameLength;
	WCHAR FileName[1];
} FILE_RENAME_INFORMATION;

static NTSTATUS(WINAPI* g_origNtSetInformationFile)(HANDLE, PIO_STATUS_BLOCK, PVOID, ULONG, FILE_INFORMATION_CLASS);

static NTSTATUS WINAPI NtSetInformationFileStub(HANDLE handle, PIO_STATUS_BLOCK sb, PVOID inf, ULONG infSize, FILE_INFORMATION_CLASS cla)
{
	auto tls = GetTls();

	if (!tls->inOpenFile)
	{
		tls->inOpenFile = true;

		if (cla == 10 /* FileRenameInformation */ || cla == 65 /* FileRenameInformationEx */)
		{
			auto origInfo = (const FILE_RENAME_INFORMATION*)inf;
			std::wstring origFileName{ origInfo->FileName, origInfo->FileNameLength / sizeof(wchar_t) };
			if (IsMappedFilename(origFileName))
			{
				auto newName = MapRedirectedNtFilename(origFileName.c_str());

				// NT doesn't like slashes
				std::replace(newName.begin(), newName.end(), L'/', L'\\');

				std::vector<uint8_t> info(sizeof(FILE_RENAME_INFORMATION) + (newName.length() * 2));
				auto newInfo = (FILE_RENAME_INFORMATION*)info.data();
				newInfo->Flags = origInfo->Flags;
				newInfo->RootDirectory = origInfo->RootDirectory;
				newInfo->FileNameLength = newName.size() * sizeof(wchar_t);
				memcpy(newInfo->FileName, newName.c_str(), newInfo->FileNameLength);

				tls->inOpenFile = false;

				return g_origNtSetInformationFile(handle, sb, info.data(), info.size(), cla);
			}
		}

		tls->inOpenFile = false;

		return g_origNtSetInformationFile(handle, sb, inf, infSize, cla);
	}


	return g_origNtSetInformationFile(handle, sb, inf, infSize, cla);
}

NTSTATUS(WINAPI* g_origNtDeleteFile)(POBJECT_ATTRIBUTES objectAttributes);

NTSTATUS WINAPI NtDeleteFileStub(POBJECT_ATTRIBUTES objectAttributes)
{
	auto tls = GetTls();

	if (!tls->inDeleteFile)
	{
		tls->inDeleteFile = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		tls->inDeleteFile = false;

		NTSTATUS retval = g_origNtDeleteFile(&attributes);

		return retval;
	}

	return g_origNtDeleteFile(objectAttributes);
}

static DWORD(WINAPI* g_origRtlGetFullPathName_U)(const WCHAR* name, ULONG size, WCHAR* buffer, WCHAR** file_part);

static DWORD WINAPI RtlGetFullPathName_UStub(const WCHAR* name, ULONG size, WCHAR* buffer, WCHAR** file_part)
{
	auto tls = GetTls();

	if (!tls->inQueryAttributes)
	{
		tls->inQueryAttributes = true;

		if (IsMappedFilename(name))
		{
			auto rv = g_origRtlGetFullPathName_U(MapRedirectedFilename(name).c_str(), size, buffer, file_part);
			tls->inQueryAttributes = false;
			return rv;
		}

		tls->inQueryAttributes = false;
	}

	return g_origRtlGetFullPathName_U(name, size, buffer, file_part);
}

NTSTATUS(WINAPI* g_origNtQueryFullAttributesFile)(POBJECT_ATTRIBUTES objectAttributes, void* networkInformation);

NTSTATUS WINAPI NtQueryFullAttributesFileStub(POBJECT_ATTRIBUTES objectAttributes, void* networkInformation)
{
	auto tls = GetTls();

	if (!tls->inQueryAttributes)
	{
		tls->inQueryAttributes = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		tls->inQueryAttributes = false;

		NTSTATUS retval = g_origNtQueryFullAttributesFile(&attributes, networkInformation);

		return retval;
	}

	return g_origNtQueryFullAttributesFile(objectAttributes, networkInformation);
}

NTSTATUS(WINAPI* g_origNtQueryAttributesFile)(POBJECT_ATTRIBUTES objectAttributes, void* basicInformation);

NTSTATUS WINAPI NtQueryAttributesFileStub(POBJECT_ATTRIBUTES objectAttributes, void* basicInformation)
{
	auto tls = GetTls();

	if (!tls->inQueryAttributes)
	{
		tls->inQueryAttributes = true;

		OBJECT_ATTRIBUTES attributes = *objectAttributes;
		UNICODE_STRING newString;

		// map the filename
		std::wstring moduleNameStr(objectAttributes->ObjectName->Buffer, objectAttributes->ObjectName->Length / sizeof(wchar_t));

		if (IsMappedFilename(moduleNameStr))
		{
			moduleNameStr = MapRedirectedNtFilename(moduleNameStr.c_str());

			// NT doesn't like slashes
			std::replace(moduleNameStr.begin(), moduleNameStr.end(), L'/', L'\\');

			// set stuff
			RtlInitUnicodeString(&newString, moduleNameStr.c_str());
			attributes.ObjectName = &newString;
		}

		tls->inQueryAttributes = false;

		NTSTATUS retval = g_origNtQueryAttributesFile(&attributes, basicInformation);

		return retval;
	}

	return g_origNtQueryAttributesFile(objectAttributes, basicInformation);
}

static HRESULT(WINAPI* g_origQueryDListForApplication1)(BOOL* pDefaultToDiscrete, HANDLE hAdapter, void* pfnEscapeCB);

static HRESULT WINAPI QueryDListForApplication1Stub(BOOL* pDefaultToDiscrete, HANDLE hAdapter, void* pfnEscapeCB)
{
	auto hr = g_origQueryDListForApplication1(pDefaultToDiscrete, hAdapter, pfnEscapeCB);

	if (SUCCEEDED(hr))
	{
		// log if done
		if (!*pDefaultToDiscrete)
		{
			trace("Overriding QueryDListForApplication1 to run on discrete GPU.\n");
		}

		// we always want to run on the discrete GPU, even if the UMD says not to
		*pDefaultToDiscrete = TRUE;
	}

	return hr;
}

static FARPROC(WINAPI* g_origGetProcAddress)(HMODULE hModule, LPCSTR procName, void* caller);

NTSTATUS NTAPI NtCloseHook(IN HANDLE Handle);
NTSTATUS NTAPI NtQueryInformationProcessHook(IN HANDLE ProcessHandle, IN PROCESSINFOCLASS ProcessInformationClass, OUT PVOID ProcessInformation, IN ULONG ProcessInformationLength, OUT PULONG ReturnLength OPTIONAL);

extern "C" NTSTATUS NTAPI NtQueryVirtualMemory(
	HANDLE                   ProcessHandle,
	PVOID                    BaseAddress,
	INT MemoryInformationClass,
	PVOID                    MemoryInformation,
	SIZE_T                   MemoryInformationLength,
	PSIZE_T                  ReturnLength
);

NTSTATUS NTAPI NtQueryVirtualMemoryHook(
	HANDLE                   ProcessHandle,
	PVOID                    BaseAddress,
	INT MemoryInformationClass,
	PVOID                    MemoryInformation,
	SIZE_T                   MemoryInformationLength,
	PSIZE_T                  ReturnLength
)
{
	return NtQueryVirtualMemory(ProcessHandle, BaseAddress, MemoryInformationClass, MemoryInformation, MemoryInformationLength, ReturnLength);
}

#include <Hooking.h>

static const DWORD bigOne = 1;
static auto ntdllRef = GetModuleHandleW(L"ntdll.dll");

FARPROC WINAPI GetProcAddressStub(HMODULE hModule, LPCSTR lpProcName, void* caller)
{
	if (!IS_INTRESOURCE(lpProcName))
	{
		if (true)
		{
#ifdef _M_AMD64
			if (hModule == ntdllRef && _stricmp(lpProcName, "NtQueryInformationProcess") == 0)
			{
				char* ntdll = (char*)ntdllRef;
				auto ntdllImage = (PIMAGE_DOS_HEADER)ntdll;
				auto ntdllPE = (PIMAGE_NT_HEADERS)(ntdll + ntdllImage->e_lfanew);

				auto lol = ntdll + ntdllPE->OptionalHeader.BaseOfCode + ntdllPE->OptionalHeader.SizeOfCode - 0x40;

				static std::once_flag lold;

				std::call_once(lold, [lol]
				{
					DWORD oldProtect;
					VirtualProtect(lol, 48, PAGE_EXECUTE_READWRITE, &oldProtect);

					static struct : jitasm::Frontend
					{
						uintptr_t f;

						void InternalMain() override
						{
							mov(rax, qword_ptr[rsp + 0x28]);
							sub(rsp, 0x38);
							mov(qword_ptr[rsp + 0x20], rax);
							mov(rax, f);
							call(rax);
							add(rsp, 0x38);
							ret();
						}
					} g;

					g.f = (uint64_t)NtQueryInformationProcessHook;

					auto c = g.GetCode();
					memcpy(lol, c, g.GetCodeSize());

					VirtualProtect(lol, 48, PAGE_EXECUTE_READ, &oldProtect);
				});

				return (FARPROC)lol;

				//return (FARPROC)NtQueryInformationProcessHook;
				//return (FARPROC)NtQueryInformationProcess;
			}
#else
			if (hModule == ntdllRef && _stricmp(lpProcName, "NtQueryInformationProcess") == 0)
			{
				return (FARPROC)NtQueryInformationProcessHook;
			}
#endif

			if (hModule == ntdllRef && _stricmp(lpProcName, "NtQueryVirtualMemory") == 0)
			{
				return (FARPROC)NtQueryVirtualMemoryHook;
				//return (FARPROC)NtQueryInformationProcess;
			}

			if (hModule == ntdllRef && _stricmp(lpProcName, "NtClose") == 0)
			{
				//return (FARPROC)NtCloseHook;
			}
		}

		if (_stricmp(lpProcName, "NvOptimusEnablement") == 0 || _stricmp(lpProcName, "AmdPowerXpressRequestHighPerformance") == 0)
		{
			return (FARPROC)&bigOne;
		}
	}

	return g_origGetProcAddress(hModule, lpProcName, caller);
}

extern "C" BOOLEAN NTAPI RtlEqualString(
	_In_ const STRING  *String1,
	_In_ const STRING  *String2,
	_In_       BOOLEAN CaseInSensitive
);

extern "C" BOOLEAN NTAPI RtlEqualUnicodeString(
	_In_ const UNICODE_STRING  *String1,
	_In_ const UNICODE_STRING  *String2,
	_In_       BOOLEAN CaseInSensitive
);

static NTSTATUS(NTAPI* g_origGetProcedureAddress)(HMODULE hModule, PANSI_STRING functionName, WORD ordinal, PVOID* fnAddress);

NTSTATUS NTAPI LdrGetProcedureAddressStub(HMODULE hModule, PANSI_STRING functionName, WORD ordinal, PVOID* fnAddress)
{
	static const DWORD bigOne = 1;

	if (functionName)
	{
		ANSI_STRING compareNv, compareAmd;
		RtlInitAnsiString(&compareNv, "NvOptimusEnablement");
		RtlInitAnsiString(&compareAmd, "AmdPowerXpressRequestHighPerformance");

		if (RtlEqualString(functionName, &compareNv, TRUE) || RtlEqualString(functionName, &compareAmd, TRUE))
		{
			*fnAddress = const_cast<DWORD*>(&bigOne);
			return 0;
		}
	}

	return g_origGetProcedureAddress(hModule, functionName, ordinal, fnAddress);
}

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA
{
	ULONG Flags;
	PUNICODE_STRING FullDllName;
	PUNICODE_STRING BaseDllName;
	PVOID DllBase;
	ULONG SizeOfImage;
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA
{
	ULONG Flags;
	PCUNICODE_STRING FullDllName;
	PCUNICODE_STRING BaseDllName;
	PVOID DllBase;
	ULONG SizeOfImage;
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA
{
	LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
	LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

typedef VOID(NTAPI *PLDR_DLL_NOTIFICATION_FUNCTION)(
	_In_ ULONG NotificationReason,
	_In_ PLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID Context
	);

NTSYSAPI
NTSTATUS
NTAPI
LdrRegisterDllNotification(
	_In_ ULONG Flags,
	_In_ PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
	_In_ PVOID Context,
	_Out_ PVOID *Cookie
);

static PVOID g_lastDllNotif;

VOID CALLBACK LdrDllNotification(
	_In_     ULONG                       NotificationReason,
	_In_     PLDR_DLL_NOTIFICATION_DATA NotificationData,
	_In_opt_ PVOID                       Context
)
{
	if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED)
	{
		UNICODE_STRING amdHdl;
		RtlInitUnicodeString(&amdHdl, L"amdhdl64.dll");

		UNICODE_STRING nvDlistX;
		RtlInitUnicodeString(&nvDlistX, L"nvdlistx.dll");

		if (RtlEqualUnicodeString(NotificationData->Loaded.BaseDllName, &amdHdl, TRUE) || RtlEqualUnicodeString(NotificationData->Loaded.BaseDllName, &nvDlistX, TRUE))
		{
			void* proc = GetProcAddress(reinterpret_cast<HMODULE>(NotificationData->Loaded.DllBase), "QueryDListForApplication1");
			MH_CreateHook(proc, QueryDListForApplication1Stub, (void**)&g_origQueryDListForApplication1);
			MH_EnableHook(proc);

			trace("Hooked external (mobile) GPU drivers to force enablement.\n");
		}
	}
}

static decltype(&::CreateThread) g_origCreateThread;
static decltype(&::CreateThread) g_origCreateThread32;

template<decltype(&g_origCreateThread) Orig>
static HANDLE WINAPI CreateThreadStub( _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ SIZE_T dwStackSize, _In_ LPTHREAD_START_ROUTINE lpStartAddress, _In_opt_ __drv_aliasesMem LPVOID lpParameter, _In_ DWORD dwCreationFlags, _Out_opt_ LPDWORD lpThreadId )
{
	if (dwStackSize > 0 && dwStackSize < (1 * 1024 * 1024))
	{
		dwStackSize = 0;
	}

	return (*Orig)(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
}

static void(WINAPI* g_origCoUninitialize)();

static void WINAPI CoUninitializeStub()
{
	
}

extern "C" DLL_EXPORT void CoreSetMappingFunction(MappingFunctionType function)
{
	DisableToolHelpScope scope;

	{
		HMODULE dx = LoadLibraryW(L"d3dx11_43.dll");

		if (dx)
		{
			FreeLibrary(dx);

			g_d3dx11 = true;
		}
	}

	g_mappingFunction = function;
	g_tlsHandle = TlsAlloc();

	LSP_InitializeHooks();

	MH_CreateHookApi(L"ntdll.dll", "NtCreateFile", NtCreateFileStub, (void**)&g_origNtCreateFile);
	MH_CreateHookApi(L"ntdll.dll", "NtOpenFile", NtOpenFileStub, (void**)&g_origNtOpenFile);
	MH_CreateHookApi(L"ntdll.dll", "NtDeleteFile", NtDeleteFileStub, (void**)&g_origNtDeleteFile);
	MH_CreateHookApi(L"ntdll.dll", "NtQueryAttributesFile", NtQueryAttributesFileStub, (void**)&g_origNtQueryAttributesFile);
	MH_CreateHookApi(L"ntdll.dll", "NtQueryFullAttributesFile", NtQueryFullAttributesFileStub, (void**)&g_origNtQueryFullAttributesFile);
	MH_CreateHookApi(L"ntdll.dll", "NtSetInformationFile", NtSetInformationFileStub, (void**)&g_origNtSetInformationFile);
	MH_CreateHookApi(L"ntdll.dll", "LdrLoadDll", LdrLoadDllStub, (void**)&g_origLoadDll);
	MH_CreateHookApi(L"ntdll.dll", "LdrGetProcedureAddress", LdrGetProcedureAddressStub, (void**)&g_origGetProcedureAddress);
	MH_CreateHookApi(L"ntdll.dll", "RtlGetFullPathName_U", RtlGetFullPathName_UStub, (void**)&g_origRtlGetFullPathName_U);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoSizeW", GetFileVersionInfoSizeWStub, (void**)&g_origGetFileVersionInfoSizeW);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoSizeA", GetFileVersionInfoSizeAStub, (void**)&g_origGetFileVersionInfoSizeA);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoW", GetFileVersionInfoWStub, (void**)&g_origGetFileVersionInfoW);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoA", GetFileVersionInfoAStub, (void**)&g_origGetFileVersionInfoA);
	MH_CreateHookApi(L"version.dll", "VerQueryValueW", VerQueryValueWStub, (void**)&g_origVerQueryValueW);
	MH_CreateHookApi(L"kernelbase.dll", "RegOpenKeyExW", RegOpenKeyExWStub, (void**)&g_origRegOpenKeyExW);
	MH_CreateHookApi(L"kernelbase.dll", "GetFileAttributesExW", GetFileAttributesExWStub, (void**)&g_origGetFileAttributesExW);
	MH_CreateHookApi(L"kernelbase.dll", "GetProcAddressForCaller", GetProcAddressStub, (void**)&g_origGetProcAddress);
	MH_CreateHookApi(L"kernel32.dll", "CreateThread", CreateThreadStub<&g_origCreateThread32>, (void**)&g_origCreateThread32);
	MH_CreateHookApi(L"kernelbase.dll", "CreateThread", CreateThreadStub<&g_origCreateThread>, (void**)&g_origCreateThread);
	MH_CreateHookApi(L"combase.dll", "CoUninitialize", CoUninitializeStub, (void**)&g_origCoUninitialize);
	MH_EnableHook(MH_ALL_HOOKS);

	static auto _LdrRegisterDllNotification = (decltype(&LdrRegisterDllNotification))GetProcAddress(GetModuleHandle(L"ntdll.dll"), "LdrRegisterDllNotification");
	_LdrRegisterDllNotification(0, LdrDllNotification, nullptr, &g_lastDllNotif);
}
#endif
