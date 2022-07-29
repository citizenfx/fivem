#include "StdInc.h"

#include <wrl.h>

#include <d3d11.h>
#include <d3d11_1.h>

#include <shellapi.h>
#include <shlobj.h>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <CfxLocale.h>
#endif

#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

bool DXGICheck()
{
    ComPtr<ID3D11Device> device;
    
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, device.GetAddressOf(), nullptr, nullptr);

    if (FAILED(hr))
    {
        trace("Couldn't create a D3D11 device - HRESULT %08x\n", hr);
        return true;
    }

    ComPtr<IDXGIDevice> dxgiOrig;
    hr = device.As(&dxgiOrig);

    if (FAILED(hr))
    {
        trace("Not an IDXGIDevice?\n");
        return true;
    }

    ComPtr<IDXGIDevice2> dxgiCheck;
    hr = dxgiOrig.As(&dxgiCheck);

    if (FAILED(hr))
    {
#if defined(LAUNCHER_PERSONALITY_MAIN)
		std::wstring suggestion = gettext(L"The game will exit now.");

        if (!IsWindows7SP1OrGreater())
        {
			suggestion = gettext(L"Please install Windows 7 SP1 or greater, and try again.");
        }
        else if (!IsWindows8OrGreater())
        {
            suggestion = gettext(L"Please install the Platform Update for Windows 7, and try again.");
        }

        MessageBox(nullptr, va(gettext(L"DXGI 1.2 support is required to run this product %s"), suggestion), PRODUCT_NAME, MB_OK | MB_ICONSTOP);

        if (IsWindows7SP1OrGreater() && !IsWindows8OrGreater())
        {
            ShellExecute(nullptr, L"open", L"https://www.microsoft.com/en-us/download/details.aspx?id=36805", nullptr, nullptr, SW_SHOWNORMAL);
        }
#endif

        return false;
    }

    return true;
}

bool BaseLdrCheck()
{
	auto addDllDirectory = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "AddDllDirectory");

	if (addDllDirectory == nullptr)
	{
#if defined(LAUNCHER_PERSONALITY_MAIN)
		MessageBox(nullptr, gettext(L"This product requires Security Update for Windows 7 for x64-based systems (KB2758857) to be installed to run. Please install it, and try again.").c_str(), PRODUCT_NAME, MB_OK | MB_ICONSTOP);
#endif

		if (!IsWindows8OrGreater())
		{
			ShellExecute(nullptr, L"open", L"https://www.microsoft.com/en-us/download/details.aspx?id=35936", nullptr, nullptr, SW_SHOWNORMAL);
		}

		return false;
	}

	return true;
}

bool MediaFeatureCheck()
{
	auto module = LoadLibraryW(L"mfreadwrite.dll");

	if (!module)
	{
#if defined(LAUNCHER_PERSONALITY_MAIN)
		MessageBox(nullptr, fmt::sprintf(
			gettext(L"%s requires the Windows Media Feature Pack for Windows N editions to be installed to run. Please install it, and try again."),
			PRODUCT_NAME)
		.c_str(), PRODUCT_NAME, MB_OK | MB_ICONSTOP);

		ShellExecute(nullptr, L"open", L"https://support.microsoft.com/help/3145500/media-feature-pack-list-for-windows-n-editions", nullptr, nullptr, SW_SHOWNORMAL);
#endif

		return false;
	}

	FreeLibrary(module);

	return true;
}

bool VerifyViability()
{
	// if we run DXGI checks on non-downlevel versions, DList drivers won't be hooked, and iGPU+dGPU systems
	// will cache the fact that we (incorrectly) want to run on the iGPU, leading to a mismatch since the *UI process*
	// still runs on the dGPU.
	//
	// this will still cause the issue on downlevel systems, but those have broken iGPU+dGPU support anyway.
	if (!IsWindows8Point1OrGreater())
	{
		if (!DXGICheck())
		{
			return false;
		}
	}

	if (!BaseLdrCheck())
	{
		return false;
	}

	if (!MediaFeatureCheck())
	{
		return false;
	}

    return true;
}

void DoPreLaunchTasks()
{
	auto SetProcessMitigationPolicy = (decltype(&::SetProcessMitigationPolicy))GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetProcessMitigationPolicy");

	if (SetProcessMitigationPolicy)
	{
		PROCESS_MITIGATION_EXTENSION_POINT_DISABLE_POLICY dp;
		dp.DisableExtensionPoints = true;

		SetProcessMitigationPolicy(ProcessExtensionPointDisablePolicy, &dp, sizeof(dp));
	}

	if (MakeRelativeCitPath(L"").find(L".app") != std::string::npos)
	{
		wchar_t thisFileName[512];
		GetModuleFileName(GetModuleHandle(NULL), thisFileName, sizeof(thisFileName) / 2);

		SHFOLDERCUSTOMSETTINGS fcs = { 0 };
		fcs.dwSize = sizeof(SHFOLDERCUSTOMSETTINGS);
		fcs.dwMask = FCSM_ICONFILE;
		fcs.pszIconFile = thisFileName;
		fcs.cchIconFile = 0;
		fcs.iIconIndex = -201;

		SHGetSetFolderCustomSettings(&fcs, MakeRelativeCitPath(L"").c_str(), FCS_FORCEWRITE);

		SHSetLocalizedName(MakeRelativeCitPath(L"").c_str(), thisFileName, 101);
	}
}

void MigrateCacheFormat202105()
{
	// execute the big cache emigration
	CreateDirectoryW(MakeRelativeCitPath(L"data/").c_str(), NULL);
	CreateDirectoryW(MakeRelativeCitPath(L"data/server-cache/").c_str(), NULL);

	std::vector<std::tuple<std::string, std::string>> moveList = {
		{ "cache/game/", "data/game-storage/" },
		{ "cache/priv/", "data/server-cache-priv/" },
		{ "cache/fxdk/", "data/server-cache-fxdk/" },
		{ "cache/browser-fxdk/", "data/nui-storage-fxdk/" },
		{ "cache/browser/", "data/nui-storage/" },
		{ "cache/db/", "data/server-cache/db/" },
		{ "cache/ipfs_data/", "data/ipfs/" },
		{ "cache/", "data/cache/" },
	};

	for (const auto& entry : moveList)
	{
		auto src = MakeRelativeCitPath(ToWide(std::get<0>(entry)));
		auto dest = MakeRelativeCitPath(ToWide(std::get<1>(entry)));

		if (GetFileAttributesW(src.c_str()) != INVALID_FILE_ATTRIBUTES && GetFileAttributesW(dest.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			if (!MoveFileW(src.c_str(), dest.c_str()))
			{
				// #TODO: warn user? log it?
			}
		}
	}

	// enumerate cache_* files
	{
		auto cacheRoot = MakeRelativeCitPath(L"data/cache/");
		auto cacheDest = MakeRelativeCitPath(L"data/server-cache/");

		WIN32_FIND_DATAW findData;
		if (HANDLE hFind = FindFirstFileW((cacheRoot + L"cache_*").c_str(), &findData); hFind != INVALID_HANDLE_VALUE)
		{
			do 
			{
				MoveFileW((cacheRoot + findData.cFileName).c_str(), (cacheDest + findData.cFileName).c_str());
			} while (FindNextFile(hFind, &findData));

			FindClose(hFind);
		}
	}
}
