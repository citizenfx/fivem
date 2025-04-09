/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <optional>

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <ShlObj.h>
#include <CfxLocale.h>

#include <HostSharedData.h>
#include <CfxState.h>

#include <wrl.h>

namespace WRL = Microsoft::WRL;

struct ScopedCoInitialize
{
	template<typename... TArg>
	ScopedCoInitialize(const TArg&&... args) : m_hr(CoInitializeEx(nullptr, args...))
	{
	}

	~ScopedCoInitialize()
	{
		if (SUCCEEDED(m_hr))
		{
			CoUninitialize();
		}
	}

	inline operator bool()
	{
		return (SUCCEEDED(m_hr));
	}

	inline HRESULT GetResult()
	{
		return m_hr;
	}

private:
	HRESULT m_hr;
};

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

#include <openssl/evp.h>
#include <json.hpp>

using json = nlohmann::json;

static std::string GetMtlGamePath(std::string_view gameName)
{
	auto appdataRoot = GetFolderPath(FOLDERID_ProgramData);
	auto titlesFile = appdataRoot + L"\\Rockstar Games\\Launcher\\titles.dat";

	FILE* f = _wfopen(titlesFile.c_str(), L"rb");
	if (f)
	{
		// read to end
		fseek(f, 0, SEEK_END);
		
		auto len = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<uint8_t> fileData(len);
		fread(fileData.data(), 1, len, f);
		fclose(f);

		uint8_t key[32] = { 0 };
		uint8_t iv[16] = { 0 };

		EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
		if (!ctx)
		{
			return "";
		}

		EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

		int outl = fileData.size();
		size_t finalLen = 0;
		EVP_DecryptUpdate(ctx, fileData.data(), &outl, fileData.data(), outl);
		finalLen += outl;

		int outl2 = fileData.size() - outl;
		EVP_DecryptFinal_ex(ctx, fileData.data() + outl, &outl2);
		finalLen += outl2;

		EVP_CIPHER_CTX_free(ctx);

		try
		{
			std::string fileText(fileData.begin() + 16, fileData.begin() + finalLen);
			auto titleData = json::parse(fileText);

			for (auto& t : titleData["tl"])
			{
				if (t["ti"].get<std::string>() == gameName)
				{
					return t["il"].get<std::string>();
				}
			}
		}
		catch (std::exception& e)
		{
			trace("%s\n", e.what());
		}
	}

	return "";
}

std::optional<int> EnsureGamePath()
{
#ifdef IS_LAUNCHER
	return {};
#endif

	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	const wchar_t* pathKey = L"IVPath";

	if (wcsstr(GetCommandLine(), L"cl2"))
	{
		pathKey = L"PathCL2";
	}

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		wchar_t path[256];

		GetPrivateProfileString(L"Game", pathKey, L"", path, _countof(path), fpath.c_str());

		if (path[0] != L'\0')
		{
#if defined(GTA_FIVE)
			// TEMP: Gen9 specific error, some users rename their .exe to bypass name checks
			// Throw early to avoid unspecific error output
			if (const std::wstring lastComponent = std::filesystem::path(path).filename().wstring();
				lastComponent.find(L"V Enhanced") != std::wstring::npos || lastComponent.find(L"VEnhanced") != std::wstring::npos)
			{
				// Clear "wrong" path entry
				WritePrivateProfileString(L"Game", pathKey, nullptr, fpath.c_str());

				static constexpr auto GEN9_ERROR = L"Your selected game installation folder points to the Enhanced edition of GTA V, which is currently not supported by FiveM.\n\n"
					L"Please select the installation folder for the Legacy version of GTA V.";
				MessageBox(nullptr, GEN9_ERROR, PRODUCT_NAME, MB_OK | MB_ICONWARNING);
			}
			else
			{
#endif
			// check stuff regarding the game executable
			std::wstring gameExecutable = fmt::sprintf(L"%s\\%s", path, GAME_EXECUTABLE);

			if (GetFileAttributes(gameExecutable.c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				return {};
			}
#if defined(GTA_FIVE)
			}
#endif
		}
	}

	ScopedCoInitialize coInit(COINIT_APARTMENTTHREADED);

	if (!coInit)
	{
		MessageBox(nullptr, va(L"CoInitializeEx failed. HRESULT = 0x%08x.", coInit.GetResult()), L"Error", MB_OK | MB_ICONERROR);

		return static_cast<int>(coInit.GetResult());
	}

	WRL::ComPtr<IFileDialog> fileDialog;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileDialog, (void**)fileDialog.GetAddressOf());

	if (FAILED(hr))
	{
		MessageBox(nullptr, va(L"CoCreateInstance(IFileDialog) failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);

		return static_cast<int>(hr);
	}

	FILEOPENDIALOGOPTIONS opts;
	fileDialog->GetOptions(&opts);

	opts |= FOS_FORCEFILESYSTEM;

	fileDialog->SetOptions(opts);
	fileDialog->SetTitle(L"Go to your game directory and select " GAME_EXECUTABLE L" to be able to launch " PRODUCT_NAME);
	
	COMDLG_FILTERSPEC filter = { 0 };
	filter.pszName = L"Game executables";
	filter.pszSpec = GAME_EXECUTABLE;
	fileDialog->SetFileTypes(1, &filter);

#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
	auto proposeDirectory = [&fileDialog](const std::wstring& gameRoot, const std::vector<std::string>& filesToCheck)
	{
		WRL::ComPtr<IShellItem> item;

		if (FAILED(SHCreateItemFromParsingName(gameRoot.c_str(), nullptr, IID_PPV_ARGS(&item))))
		{
			return false;
		}

		if (FAILED(fileDialog->SetFolder(item.Get())))
		{
			return false;
		}

		for (const auto& file : filesToCheck)
		{
			if (GetFileAttributesW((gameRoot + (L"\\" + ToWide(file))).c_str()) == INVALID_FILE_ATTRIBUTES)
			{
				return false;
			}
		}

		return true;
	};

	// set the default folder, if we can find one
	{
		std::vector<std::string> filesToCheck = {
#if defined(GTA_FIVE)
			"x64a.rpf",
			"x64b.rpf",
			"x64g.rpf",
			"common.rpf",
			"bink2w64.dll",
			"x64\\audio\\audio_rel.rpf",
			"GTA5.exe",
			"update\\x64\\dlcpacks\\mpheist3\\dlc.rpf",
			"update\\x64\\dlcpacks\\mptuner\\dlc.rpf",
			"update\\x64\\dlcpacks\\mpsum2\\dlc.rpf"
#elif defined(IS_RDR3)
			"common_0.rpf",
			"appdata0_update.rpf",
			"levels_7.rpf",
			"RDR2.exe",
			"x64\\dlcpacks\\mp007\\dlc.rpf"
#elif defined(GTA_NY)
			"pc/audio/sfx/general.rpf",
#endif
		};

		wchar_t gameRootBuf[1024];
		DWORD gameRootLength = sizeof(gameRootBuf);

		// 5 is the amount of characters to strip off the end
		const std::tuple<std::wstring, std::wstring, int> folderAttempts[] = {
#if defined(GTA_FIVE)
			{ L"InstallFolderSteam", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\GTAV", 5 },
			{ L"InstallFolderEpic", L"SOFTWARE\\Rockstar Games\\Grand Theft Auto V", 0 },
			{ L"InstallFolderEpic", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Grand Theft Auto V", 0 },
#elif defined(IS_RDR3)
			{ L"InstallFolderSteam", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Red Dead Redemption 2", strlen("Red Dead Redemption 2") },
#elif defined(GTA_NY)
			{ L"InstallFolder", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Grand Theft Auto IV", 0 },
#endif
		};

		// try finding the MTL game path first
		auto mtlGamePath = GetMtlGamePath(
#if defined(GTA_FIVE)
		"gta5"
#elif defined(IS_RDR3)
		"rdr2"
#elif defined(GTA_NY)
		"gta4"
#endif
		);

		if (!mtlGamePath.empty())
		{
			if (proposeDirectory(ToWide(mtlGamePath), filesToCheck))
			{
				WritePrivateProfileString(L"Game", pathKey, ToWide(mtlGamePath).c_str(), fpath.c_str());
				return {};
			}
		}

		for (const auto& folder : folderAttempts)
		{
			if (RegGetValue(HKEY_LOCAL_MACHINE,
				std::get<1>(folder).c_str(), std::get<0>(folder).c_str(),
				RRF_RT_REG_SZ, nullptr, gameRootBuf, &gameRootLength) == ERROR_SUCCESS)
			{
				std::wstring gameRoot(gameRootBuf);

				// strip \GTAV if needed
				gameRoot = gameRoot.substr(0, gameRoot.length() - std::get<int>(folder));

				if (proposeDirectory(gameRoot, filesToCheck))
				{
					WritePrivateProfileString(L"Game", pathKey, gameRoot.c_str(), fpath.c_str());
					return {};
				}
			}
		}
	}
#endif

#if defined(GTA_FIVE)
	{
		// TEMP: Gen9 specific warning, inform users about no Enhanced support early
		std::vector<std::string> filesToCheck = {
			"x64a.rpf",
			"x64b.rpf",
			"x64g.rpf",
			"common.rpf",
			"bink2w64.dll",
			"x64\\audio\\audio_rel.rpf",
			"GTA5_Enhanced.exe",
			"update\\x64\\dlcpacks\\mpheist3\\dlc.rpf",
			"update\\x64\\dlcpacks\\mptuner\\dlc.rpf",
			"update\\x64\\dlcpacks\\mpsum2\\dlc.rpf"
		};

		wchar_t gameRootBuf[1024];
		DWORD gameRootLength = sizeof(gameRootBuf);

		const std::tuple<std::wstring, std::wstring, int> folderAttemptsGen9[] = {
			{ L"InstallFolder", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\GTAV Enhanced", 0 }, // RGL install
			{ L"InstallFolderSteam", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\GTA V Enhanced", 0 },
			{ L"InstallFolderEpic", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\GTAV Enhanced", 0 },
			{ L"InstallFolderXboxPc", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\GTAV Enhanced", 0 } // Xbox Game Pass install
		};

		for (const auto& folder : folderAttemptsGen9)
		{
			if (RegGetValue(HKEY_LOCAL_MACHINE,std::get<1>(folder).c_str(), std::get<0>(folder).c_str(),
				RRF_RT_REG_SZ, nullptr, gameRootBuf, &gameRootLength) == ERROR_SUCCESS)
			{
				std::wstring gameRoot(gameRootBuf);

				if (proposeDirectory(gameRoot, filesToCheck))
				{
					static constexpr auto GEN9_ERROR = L"We could not detect a valid GTA V Legacy installation. However, we found a valid installation for GTA V Enhanced.\n\n"
						L"Please ensure you have GTA V Legacy installed and select its installation folder in the file dialog after closing this message.";
					MessageBox(nullptr, GEN9_ERROR, PRODUCT_NAME, MB_OK | MB_ICONWARNING);
					break;
				}
			}
		}
	}
#endif

	hr = fileDialog->Show(nullptr);

	if (FAILED(hr))
	{
		if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED))
		{
			MessageBox(nullptr, va(L"Could not show game folder selection window: IFileDialog::Show failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		}

		return 0;
	}

	WRL::ComPtr<IShellItem> result;
	hr = fileDialog->GetResult(result.GetAddressOf());

	if (!result)
	{
		MessageBox(nullptr, va(L"You did not select a game folder: IFileDialog::GetResult failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	PWSTR resultPath;

	if (FAILED(hr = result->GetDisplayName(SIGDN_FILESYSPATH, &resultPath)))
	{
		MessageBox(nullptr, va(L"Could not get game directory: IShellItem::GetDisplayName failed. HRESULT = 0x%08x.", hr), L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	// check if there's a game EXE in the path
	std::wstring gamePath = resultPath;
	auto exeNameLength = std::size(GAME_EXECUTABLE); // counts null terminator, but here we use that for a backslash

#if defined(GTA_FIVE)
	// TEMP: Gen9 specific error, some users rename their .exe to bypass name checks
	if (const std::wstring lastComponent = std::filesystem::path(gamePath).parent_path().filename().wstring();
		lastComponent.find(L"V Enhanced") != std::wstring::npos || lastComponent.find(L"VEnhanced") != std::wstring::npos)
	{
		static constexpr auto GEN9_ERROR = L"Your selected game installation folder points to the Enhanced edition of GTA V, which is currently not supported by FiveM.\n\n"
			L"Please select the installation folder for the Legacy version of GTA V.";
		MessageBox(nullptr, GEN9_ERROR, PRODUCT_NAME, MB_OK | MB_ICONWARNING);
		return 0;
	}
#endif

	if (gamePath.rfind(L"\\" GAME_EXECUTABLE) != (gamePath.length() - exeNameLength))
	{
		MessageBox(nullptr, va(gettext(L"The selected path does not contain a %s file."), GAME_EXECUTABLE), PRODUCT_NAME, MB_OK | MB_ICONWARNING);
		return 0;
	}

	if (GetFileAttributes(gamePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
#if defined(GTA_NY)
		std::wstring eflcPath = std::wstring(resultPath) + L"\\EFLC.exe";

		if (GetFileAttributes(eflcPath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			MessageBox(nullptr, L"The selected path does not contain a GTAIV.exe file. As this is an EFLC installation, placing a GTAIV.exe (version 1.0.7.0) from any source will work as well.", PRODUCT_NAME, MB_OK | MB_ICONWARNING);
		}
		else
#endif
		{
			MessageBox(nullptr, va(gettext(L"The selected path does not contain a %s file."), GAME_EXECUTABLE), PRODUCT_NAME, MB_OK | MB_ICONWARNING);
		}

		return 0;
	}

	WritePrivateProfileString(L"Game", pathKey, gamePath.substr(0, gamePath.length() - exeNameLength).c_str(), fpath.c_str());

	{
		static HostSharedData<CfxState> initState("CfxInitState");
		initState->gameDirectory[0] = L'\0';
	}

	CoTaskMemFree(resultPath);

	return {};
}
#endif
