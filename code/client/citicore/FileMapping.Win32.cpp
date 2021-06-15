/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#pragma comment(lib, "comctl32.lib")

#ifndef IS_FXSERVER
#include <jitasm.h>
#include "Hooking.Aux.h"
#include <minhook.h>

#pragma comment(lib, "ntdll.lib")

#include <shlobj.h>

#include <winternl.h>
#include <commctrl.h>

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

static std::wstring g_scDocumentsRoot = g_documentsRoot + L"\\Rockstar Games\\Social Club";
static std::wstring g_launcherDocumentsRoot = g_documentsRoot + L"\\Rockstar Games\\Launcher";
static std::wstring g_launcherAppDataRoot = g_localAppDataRoot + L"\\Rockstar Games\\Launcher";
static std::wstring g_launcherProgramDataRoot = g_programDataRoot + L"\\Rockstar Games\\Launcher";
static std::wstring g_scFilesRoot = g_programFilesRoot + L"\\Rockstar Games\\Social Club";
static std::wstring g_scX86FilesRoot = g_programFilesX86Root + L"\\Rockstar Games\\Social Club";
static std::wstring g_launcherFilesRoot = g_programFilesRoot + L"\\Rockstar Games\\Launcher";

static std::wstring MapRedirectedFilename(const wchar_t* origFileName)
{
	//trace("map %s\n", ToNarrow(origFileName));

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

	if (wcsstr(origFileName, L"NVIDIA Corporation\\NV_Cache") != nullptr)
	{
		return MakeRelativeCitPath(L"data\\cache\\") + &wcsstr(origFileName, L"NVIDIA Corporation\\NV_Cache")[19];
	}

	// Program Files
	if (wcsstr(origFileName, L"Files\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherFilesRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\launcher") + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	// ProgramData
	if (wcsstr(origFileName, L"Data\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherProgramDataRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_data3") + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	if (wcsstr(origFileName, L"Local\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherAppDataRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_appdata3") + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	if (wcsstr(origFileName, L"Documents\\Rockstar Games\\Social Club") != nullptr || wcsstr(origFileName, g_scDocumentsRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_documents2") + &wcsstr(origFileName, L"Games\\Social Club")[17];
	}

	if (wcsstr(origFileName, L"Documents\\Rockstar Games\\Launcher") != nullptr || wcsstr(origFileName, g_launcherDocumentsRoot.c_str()) != nullptr)
	{
		return MakeRelativeCitPath(L"data\\game-storage\\ros_launcher_documents2") + &wcsstr(origFileName, L"Games\\Launcher")[14];
	}

	if (getenv("CitizenFX_ToolMode"))
	{
		if (wcsstr(origFileName, L".lnk"))
		{
			return MakeRelativeCitPath(L"data\\game-storage\\dummy.lnk");
		}

		auto gameDir = MakeRelativeCitPath(fmt::sprintf(L"data\\game-storage\\ros_launcher_game_%d", xbr::GetGameBuild()));

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
			CreateDirectoryW(gameDir.c_str(), NULL);
			//CreateDirectoryW((gameDir + L"\\Grand Theft Auto V").c_str(), NULL);
			//CreateDirectoryW((gameDir + L"\\Red Dead Redemption 2").c_str(), NULL);

			return gameDir + &wcsstr(origFileName, L"ar Games\\Games")[14];
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\Red Dead Redemption 2") != nullptr)
		{
			return gameDir + &wcsstr(origFileName, L"d Redemption 2")[14];
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\Grand Theft Auto V") != nullptr)
		{
			CreateDirectoryW(gameDir.c_str(), NULL);

			return gameDir + &wcsstr(origFileName, L"d Theft Auto V")[14];
		}
		else if (wcsstr(origFileName, L"Rockstar Games\\index.bin") != nullptr) // lol
		{
			CreateDirectoryW(gameDir.c_str(), NULL);

			return gameDir + &wcsstr(origFileName, L"Rockstar Games")[14];
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

	if (fileName.find(L"NVIDIA Corporation\\NV_Cache") != std::string::npos)
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

	return g_origGetFileVersionInfoW(fileName.c_str(), dwHandle, dwLen, lpData);
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

	// anything in this if statement **has to be lowercase**, see line above
	if (moduleNameStr.find(L"fraps64.dll") != std::string::npos || moduleNameStr.find(L"avghooka.dll") != std::string::npos ||
		// apparently crashes NUI
		moduleNameStr.find(L"bdcam64.dll") != std::string::npos ||
		// ASUS/A-Volute/Nahimic audio software
		//moduleNameStr.find(L"a-volute") != std::string::npos || moduleNameStr.find(L"audiodevprops") != std::string::npos ||
		// Canon camera software
		moduleNameStr.find(L"\\edsdk.dll") != std::string::npos ||
		// Microsoft camera software
		moduleNameStr.find(L"\\lcproxy.ax") != std::string::npos ||
		// 'PlayClaw' recently started becoming top crasher, no symbols so away with it
		moduleNameStr.find(L"playclawhook64.dll") != std::string::npos ||
		// lots of crashes occur in the DiscordApp overlay
		//moduleNameStr.find(L"overlay.x64.dll") != std::string::npos ||
		// new DiscordApp overlay name :/
		//moduleNameStr.find(L"DiscordHook64.dll") != std::string::npos ||
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
#if !defined(IS_RDR3)
		// VulkanRT loader, we don't use Vulkan, CEF does (to 'collect info'), and this crashes a lot of Vulkan drivers
		moduleNameStr.find(L"vulkan-1.dll") != std::string::npos ||
#else
		// VulkanRT loader, we don't use Vulkan, CEF does (to 'collect info'), and this crashes a lot of Vulkan drivers
		(moduleNameStr.find(L"vulkan-1.dll") != std::string::npos && getenv("CitizenFX_ToolMode")) ||
#endif
		false
	)
	{
		return 0xC0000135;
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
	MH_CreateHookApi(L"ntdll.dll", "LdrLoadDll", LdrLoadDllStub, (void**)&g_origLoadDll);
	MH_CreateHookApi(L"ntdll.dll", "LdrGetProcedureAddress", LdrGetProcedureAddressStub, (void**)&g_origGetProcedureAddress);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoSizeW", GetFileVersionInfoSizeWStub, (void**)&g_origGetFileVersionInfoSizeW);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoSizeA", GetFileVersionInfoSizeAStub, (void**)&g_origGetFileVersionInfoSizeA);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoW", GetFileVersionInfoWStub, (void**)&g_origGetFileVersionInfoW);
	MH_CreateHookApi(L"version.dll", "GetFileVersionInfoA", GetFileVersionInfoAStub, (void**)&g_origGetFileVersionInfoA);
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
