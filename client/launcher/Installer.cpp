#include "StdInc.h"

#include <CfxState.h>
#include <HostSharedData.h>

#include <shlwapi.h>
#include <shlobj.h>

#include <wrl.h>

#include <filesystem>

#pragma comment(lib, "shlwapi.lib")

namespace WRL = Microsoft::WRL;

namespace fs = std::experimental::filesystem;

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
		appDataPath += L"\\FiveM";

		CreateDirectory(appDataPath.c_str(), nullptr);
	}

	return appDataPath;
}

bool Install_PerformInstallation()
{
	auto rootPath = GetRootPath();

	if (rootPath.empty())
	{
		return false;
	}

	// the executable goes to the target
	auto targetExePath = rootPath + L"\\FiveM.exe";

	// but only if it doesn't exist
	if (GetFileAttributes(targetExePath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		MessageBox(nullptr, L"FiveM is already installed. You should launch it through the shortcut in the Start menu.\nIf you want to create a portable installation, put FiveM.exe into an empty folder instead.", L"FiveM", MB_OK | MB_ICONINFORMATION);
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
			shellLink->SetDescription(L"FiveM is a modification framework for Grand Theft Auto V");
			shellLink->SetIconLocation(targetExePath.c_str(), 0);
			
			WRL::ComPtr<IPersistFile> persist;
			hr = shellLink.As(&persist);

			if (SUCCEEDED(hr))
			{
				persist->Save((GetFolderPath(FOLDERID_Programs) + L"\\FiveM.lnk").c_str(), TRUE);
				persist->Save((GetFolderPath(FOLDERID_Desktop) + L"\\FiveM.lnk").c_str(), TRUE);
			}
		}

		CoUninitialize();
	}

	// create installroot dirs
	{
		auto appPath = rootPath + L"\\FiveM.app";
		CreateDirectory(appPath.c_str(), nullptr);

		FILE* f = _wfopen((appPath + L"\\FiveM.installroot").c_str(), L"w");
		if (f)
		{
			fclose(f);
		}
	}

	// hand-off the process elsewhere
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;

	if (CreateProcess(nullptr, const_cast<wchar_t*>(va(L"\"%s\"", targetExePath)),
		nullptr, nullptr, FALSE, CREATE_BREAKAWAY_FROM_JOB | CREATE_SUSPENDED, nullptr, nullptr, &si, &pi))
	{
		static HostSharedData<CfxState> hostData("CfxInitState");
		hostData->inJobObject = false;
		hostData->initialPid = pi.dwProcessId;

		ResumeThread(pi.hThread);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

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
		GetFileAttributes(MakeRelativeCitPath(L"FiveM.installroot").c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	// if we're running from a folder that 'smells' like a downloads folder, run as installer
	static HostSharedData<CfxState> hostData("CfxInitState");

	bool isDownloadsFolder = false;

	if (StrStrIW(hostData->initPath, L"downloads") != nullptr ||
		StrStrIW(hostData->initPath, L"\\dls") != nullptr)
	{
		isDownloadsFolder = true;
	}

	size_t maxOtherFiles = (isDownloadsFolder) ? 4 : 16;

	// count the amount of files 'together' with us in our folder
	fs::directory_iterator it(hostData->initPath), end;
	size_t numFiles = std::count_if(it, end, [](const fs::directory_entry& entry)
	{
		if (entry.path().filename().string()[0] == '.')
		{
			return false;
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