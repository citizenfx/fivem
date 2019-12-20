#include "StdInc.h"

#include <CfxState.h>
#include <HostSharedData.h>

#include <shlwapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>

#include <propkey.h>
#include <propvarutil.h>

#include <wrl.h>

#include <filesystem>

#pragma comment(lib, "shlwapi.lib")

namespace WRL = Microsoft::WRL;

namespace fs = std::filesystem;

static void SetAumid(const WRL::ComPtr<IShellLink>& link)
{
	WRL::ComPtr<IPropertyStore> propertyStore;

	if (SUCCEEDED(link.As(&propertyStore)))
	{
		PROPVARIANT pv;
		if (SUCCEEDED(InitPropVariantFromString(L"CitizenFX." PRODUCT_NAME ".Client", &pv)))
		{
			propertyStore->SetValue(PKEY_AppUserModel_ID, pv);

			PropVariantClear(&pv);
		}
	}
}

static std::wstring GetFolderPath(const KNOWNFOLDERID& folderId)
{
	PWSTR path;
	if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, nullptr, &path)))
	{
		std::wstring pathStr = path;

		CoTaskMemFree(path);

		return pathStr;
	}


	return L"";
}

static std::wstring GetRootPath()
{
	std::wstring appDataPath = GetFolderPath(FOLDERID_LocalAppData);

	if (!appDataPath.empty())
	{
#ifdef GTA_FIVE
		appDataPath += L"\\FiveM";
#elif defined(IS_RDR3)
		appDataPath += L"\\RedM";
#else
		appDataPath += L"\\Cfx.re";
#endif

		CreateDirectory(appDataPath.c_str(), nullptr);
	}

	return appDataPath;
}

static void CreateUninstallEntryIfNeeded()
{
	// is this actually running from an install root?
	wchar_t filename[MAX_PATH];
	GetModuleFileNameW(NULL, filename, std::size(filename));

	if (_wcsnicmp(filename, GetRootPath().c_str(), GetRootPath().length()) != 0)
	{
		return;
	}

	auto setUninstallString = [](const std::wstring& name, const std::wstring& value)
	{
		RegSetKeyValueW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CitizenFX_" PRODUCT_NAME, name.c_str(), REG_SZ, value.c_str(), (value.length() * 2) + 2);
	};

	auto setUninstallDword = [](const std::wstring& name, DWORD value)
	{
		RegSetKeyValueW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CitizenFX_" PRODUCT_NAME, name.c_str(), REG_DWORD, &value, sizeof(value));
	};

	setUninstallString(L"DisplayName", PRODUCT_NAME);
	setUninstallString(L"DisplayIcon", filename + std::wstring(L",0"));
	setUninstallString(L"HelpLink", L"https://fivem.net/");
	setUninstallString(L"InstallLocation", GetRootPath());
	setUninstallString(L"Publisher", L"The CitizenFX Collective");
	setUninstallString(L"UninstallString", fmt::sprintf(L"\"%s\" -uninstall app", filename));
	setUninstallString(L"URLInfoAbout", L"https://fivem.net/");
	setUninstallDword(L"NoModify", 1);
	setUninstallDword(L"NoRepair", 1);
}

void Install_Uninstall(const wchar_t* directory)
{
	// check if this is actually a FiveM directory we're trying to uninstall
	if (GetFileAttributes(fmt::sprintf(L"%s\\%s", directory, PRODUCT_NAME L".app").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return;
	}

	// prompt the user
	int button;

	if (FAILED(TaskDialog(
		NULL,
		GetModuleHandle(NULL),
		L"Uninstall " PRODUCT_NAME,
		L"Uninstall " PRODUCT_NAME  L"?",
		fmt::sprintf(L"Are you sure you want to remove " PRODUCT_NAME L" from the installation root at %s?", directory).c_str(),
		TDCBF_YES_BUTTON | TDCBF_NO_BUTTON,
		NULL,
		&button)))
	{
		return;
	}

	if (button != IDYES)
	{
		return;
	}

	CoInitialize(NULL);

	WRL::ComPtr<IFileOperation> ifo;
	HRESULT hr = CoCreateInstance(CLSID_FileOperation, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOperation, (void**)&ifo);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"Failed to create instance of IFileOperation.", L"InsnailShield", MB_OK | MB_ICONSTOP);
		return;
	}

	ifo->SetOperationFlags(FOF_NOCONFIRMATION);

	auto addDelete = [ifo](const std::wstring & item)
	{
		WRL::ComPtr<IShellItem> shitem;
		if (FAILED(SHCreateItemFromParsingName(item.c_str(), NULL, IID_IShellItem, (void**)& shitem)))
		{
			return false;
		}

		ifo->DeleteItem(shitem.Get(), NULL);
		return true;
	};

	addDelete(directory);
	addDelete(GetFolderPath(FOLDERID_Programs) + L"\\" PRODUCT_NAME L".lnk");
	addDelete(GetFolderPath(FOLDERID_Desktop) + L"\\" PRODUCT_NAME L".lnk");
	addDelete(GetFolderPath(FOLDERID_Programs) + L"\\" PRODUCT_NAME L" Singleplayer.lnk");
	addDelete(GetFolderPath(FOLDERID_Desktop) + L"\\" PRODUCT_NAME L" Singleplayer.lnk");

	hr = ifo->PerformOperations();

	if (FAILED(hr))
	{
		MessageBox(NULL, fmt::sprintf(L"Failed to uninstall " PRODUCT_NAME L". HRESULT = 0x%08x", hr).c_str(), L"InsnailShield", MB_OK | MB_ICONSTOP);
		return;
	}

	BOOL aborted = FALSE;
	ifo->GetAnyOperationsAborted(&aborted);

	if (aborted)
	{
		MessageBox(NULL, L"The uninstall operation was canceled. Some files may still remain. Please remove these files manually.", L"InsnailShield", MB_OK | MB_ICONSTOP);
	}

	RegDeleteKey(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\CitizenFX_" PRODUCT_NAME);
}

bool Install_PerformInstallation()
{
	auto rootPath = GetRootPath();

	if (rootPath.empty())
	{
		return false;
	}

	// the executable goes to the target
	auto targetExePath = rootPath + L"\\" PRODUCT_NAME L".exe";

	auto doHandoff = [targetExePath]()
	{
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		if (CreateProcess(nullptr, const_cast<wchar_t*>(va(L"\"%s\"", targetExePath)),
			nullptr, nullptr, FALSE, CREATE_BREAKAWAY_FROM_JOB | CREATE_SUSPENDED, nullptr, nullptr, &si, &pi))
		{
			static HostSharedData<CfxState> hostData("CfxInitState");
			hostData->inJobObject = false;
			hostData->SetInitialPid(pi.dwProcessId);

			ResumeThread(pi.hThread);

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			return true;
		}
		
		return false;
	};

	// but only if it doesn't exist
	if (GetFileAttributes(targetExePath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		// at least re-verify the game, if the user 'tried' to reinstall
		DeleteFileW((rootPath + L"\\caches.xml").c_str());
		DeleteFileW((rootPath + L"\\" PRODUCT_NAME L".app\\caches.xml").c_str());

		// hand off to the actual game
		if (!doHandoff())
		{
			MessageBox(nullptr, PRODUCT_NAME L" is already installed. You should launch it through the shortcut in the Start menu.\nIf you want to create a portable installation, put " PRODUCT_NAME L".exe into an empty folder instead.", PRODUCT_NAME, MB_OK | MB_ICONINFORMATION);
		}

		return true;
	}

	// actually copy the executable to the target
	wchar_t thisFileName[512];
	GetModuleFileName(GetModuleHandle(NULL), thisFileName, sizeof(thisFileName) / 2);

	if (!CopyFile(thisFileName, targetExePath.c_str(), FALSE))
	{
		return false;
	}

	// create shortcuts
	{
		CoInitialize(NULL);

		WRL::ComPtr<IShellLink> shellLink;
		HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)shellLink.ReleaseAndGetAddressOf());

		if (SUCCEEDED(hr))
		{
			shellLink->SetPath(targetExePath.c_str());
			shellLink->SetDescription(PRODUCT_NAME L" is a modification framework for Grand Theft Auto V");
			shellLink->SetIconLocation(targetExePath.c_str(), 0);
			
			SetAumid(shellLink);

			WRL::ComPtr<IPersistFile> persist;
			hr = shellLink.As(&persist);

			if (SUCCEEDED(hr))
			{
				persist->Save((GetFolderPath(FOLDERID_Programs) + L"\\" PRODUCT_NAME L".lnk").c_str(), TRUE);
				persist->Save((GetFolderPath(FOLDERID_Desktop) + L"\\" PRODUCT_NAME L".lnk").c_str(), TRUE);
			}
		}

		// make the SP shortcut
		hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)shellLink.ReleaseAndGetAddressOf());

		if (SUCCEEDED(hr))
		{
			shellLink->SetPath(targetExePath.c_str());
			shellLink->SetArguments(L"-sp");
			shellLink->SetDescription(PRODUCT_NAME L" is a modification framework for Grand Theft Auto V");
			shellLink->SetIconLocation(targetExePath.c_str(), -202);

			SetAumid(shellLink);

			WRL::ComPtr<IPersistFile> persist;
			hr = shellLink.As(&persist);

			if (SUCCEEDED(hr))
			{
				persist->Save((GetFolderPath(FOLDERID_Programs) + L"\\" PRODUCT_NAME L" Singleplayer.lnk").c_str(), TRUE);
				persist->Save((GetFolderPath(FOLDERID_Desktop) + L"\\" PRODUCT_NAME L" Singleplayer.lnk").c_str(), TRUE);
			}
		}

		CoUninitialize();
	}

	// create installroot dirs
	{
		auto appPath = rootPath + L"\\" PRODUCT_NAME L".app";
		CreateDirectory(appPath.c_str(), nullptr);

		FILE* f = _wfopen((appPath + L"\\" PRODUCT_NAME L".installroot").c_str(), L"w");
		if (f)
		{
			fclose(f);
		}
	}

	// hand-off the process elsewhere
	if (doHandoff())
	{
		return true;
	}

	return false;
}

bool Install_RunInstallMode()
{
	// if we're already installed 'sufficiently', this isn't a new install
	if (GetFileAttributes(MakeRelativeCitPath(L"CoreRT.dll").c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributes(MakeRelativeCitPath(L"citizen-resources-client.dll").c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributes(MakeRelativeCitPath(L"CitizenFX.ini").c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributes(MakeRelativeCitPath(PRODUCT_NAME L".installroot").c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		using namespace std::string_literals;

		wchar_t exePath[260];
		GetModuleFileName(GetModuleHandle(nullptr), exePath, _countof(exePath));

		std::wstring exeName = exePath;

		wcsrchr(exePath, L'\\')[0] = L'\0';

		std::wstring linkPath = exePath + L"\\" PRODUCT_NAME L" Singleplayer.lnk"s;

		if (GetFileAttributes(linkPath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			CoInitialize(NULL);

			WRL::ComPtr<IShellLink> shellLink;
			HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)shellLink.ReleaseAndGetAddressOf());

			if (SUCCEEDED(hr))
			{
				shellLink->SetPath(exeName.c_str());
				shellLink->SetArguments(L"-sp");
				shellLink->SetDescription(PRODUCT_NAME L" is a modification framework for Grand Theft Auto V");
				shellLink->SetIconLocation(exeName.c_str(), -202);

				SetAumid(shellLink);

				WRL::ComPtr<IPersistFile> persist;
				hr = shellLink.As(&persist);

				if (SUCCEEDED(hr))
				{
					persist->Save(linkPath.c_str(), TRUE);
				}
			}

			CoUninitialize();
		}

		CreateUninstallEntryIfNeeded();

		return false;
	}

	// if we're running from a folder that 'smells' like a downloads folder, run as installer
	static HostSharedData<CfxState> hostData("CfxInitState");

	bool isDownloadsFolder = false;

	if (StrStrIW(hostData->GetInitPath().c_str(), L"downloads") != nullptr ||
		StrStrIW(hostData->GetInitPath().c_str(), L"\\dls") != nullptr)
	{
		isDownloadsFolder = true;
	}

	size_t maxOtherFiles = (isDownloadsFolder) ? 3 : 5;

	// count the amount of files 'together' with us in our folder
	fs::directory_iterator it(hostData->GetInitPath()), end;
	size_t numFiles = std::count_if(it, end, [](const fs::directory_entry& entry)
	{
		try
		{
			if (entry.path().filename().string()[0] == '.')
			{
				return false;
			}
		}
		catch (std::exception& e)
		{

		}

		return true;
	});

	// compare
	if (numFiles <= maxOtherFiles)
	{
		return false;
	}

	return Install_PerformInstallation();
}
