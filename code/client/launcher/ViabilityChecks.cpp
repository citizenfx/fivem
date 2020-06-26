#include "StdInc.h"
#include <wrl.h>

#include <d3d11.h>
#include <d3d11_1.h>

#include <shellapi.h>
#include <shlobj.h>

#include <CfxLocale.h>

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

        return false;
    }

    return true;
}

bool BaseLdrCheck()
{
	auto addDllDirectory = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "AddDllDirectory");

	if (addDllDirectory == nullptr)
	{
		MessageBox(nullptr, gettext(L"This product requires Security Update for Windows 7 for x64-based systems (KB2758857) to be installed to run. Please install it, and try again.").c_str(), PRODUCT_NAME, MB_OK | MB_ICONSTOP);

		if (!IsWindows8OrGreater())
		{
			ShellExecute(nullptr, L"open", L"https://www.microsoft.com/en-us/download/details.aspx?id=35936", nullptr, nullptr, SW_SHOWNORMAL);
		}

		return false;
	}

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

bool CheckGraphicsLibrary(const std::wstring& path)
{
	DWORD versionInfoSize = GetFileVersionInfoSize(path.c_str(), nullptr);

	if (versionInfoSize)
	{
		std::vector<uint8_t> versionInfo(versionInfoSize);

		if (GetFileVersionInfo(path.c_str(), 0, versionInfo.size(), &versionInfo[0]))
		{
			struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

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
					else if (wcscmp((wchar_t*)productNameBuffer, L"ENBSeries") == 0)
					{
						// ENBSeries <0.3.8.7 is invalid
						if (fixedInfo->dwProductVersionMS < 0x3 || (fixedInfo->dwProductVersionMS == 3 && fixedInfo->dwProductVersionLS < 0x80007))
						{
							return true;
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
								return true;
							}
						}

						return false;
					}
				}
			}
		}

		// if the file exists, but it's not one of our 'whitelisted' known-good variants, load it from system
		// this will break any third-party graphics mods that _aren't_ mainline ReShade or ENBSeries *entirely*, but will hopefully
		// fix initialization issues people have with the behavior instead. (2020-04-18)
		return true;
	}

	return false;
}

bool IsUnsafeGraphicsLibraryWrap()
{
	return CheckGraphicsLibrary(MakeRelativeGamePath(L"dxgi.dll")) || CheckGraphicsLibrary(MakeRelativeGamePath(L"d3d11.dll"));
}

bool IsUnsafeGraphicsLibrary()
{
	__try
	{
		return IsUnsafeGraphicsLibraryWrap();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return true;
	}
}
