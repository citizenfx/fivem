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
			{ L"InstallFolderXboxPc", L"SOFTWARE\\WOW6432Node\\Rockstar Games\\Grand Theft Auto V", 0 },
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

// Out-of-process file dialog server for the game process.
//
// The game process cannot show IFileDialog itself: shell extension creation deadlocks on
// the shell's own worker pool inside the hooked/hand-mapped game process (see
// nui-core/src/NUIFileDialog.cpp). nui-core spawns a clean copy of this exe with
// "-filedialog:<mappingHandle>:<eventHandle>" (inherited-handle-as-integer convention,
// like -dumpserver/-switchcl) and reads the result back from the mapping. A helper that
// hangs is simply killed by the game - that containment is the point of the design.
//
// Lives in GameSelect.cpp because this file is already main-personality-only and already
// hosts the launcher's other IFileDialog use.

#include <FileDialogIPC.h>

namespace
{
class FileDialogServerEvents : public IFileDialogEvents
{
public:
	explicit FileDialogServerEvents(fdipc::Response* response)
		: m_response(response)
	{
	}

	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv) override
	{
		if (!ppv)
		{
			return E_POINTER;
		}

		if (riid == IID_IUnknown || riid == IID_IFileDialogEvents)
		{
			*ppv = static_cast<IFileDialogEvents*>(this);
			AddRef();
			return S_OK;
		}

		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	IFACEMETHODIMP_(ULONG) AddRef() override
	{
		return InterlockedIncrement(&m_refCount);
	}

	IFACEMETHODIMP_(ULONG) Release() override
	{
		auto refCount = InterlockedDecrement(&m_refCount);

		if (!refCount)
		{
			delete this;
		}

		return refCount;
	}

	// IFileDialogEvents
	IFACEMETHODIMP OnFolderChange(IFileDialog* dialog) override
	{
		// first event fired once the dialog window exists: report that to the game (so it
		// knows the helper isn't wedged) and pull the window in front of the game (the
		// game granted us foreground rights via AllowSetForegroundWindow before spawning)
		InterlockedExchange(&m_response->windowCreated, 1);

		if (!m_raised)
		{
			m_raised = true;

			WRL::ComPtr<IOleWindow> oleWindow;

			if (SUCCEEDED(dialog->QueryInterface(IID_PPV_ARGS(&oleWindow))))
			{
				HWND window = nullptr;

				if (SUCCEEDED(oleWindow->GetWindow(&window)) && window)
				{
					// The game spawned us in the background while it holds the foreground, so a
					// bare SetForegroundWindow is usually denied and the dialog opens behind the
					// game - where the user's next click dismisses it (ERROR_CANCELLED). Defeat the
					// foreground lock (same idiom as glue/ConnectToNative.cpp) and toggle topmost to
					// force z-order to the front even if the activation grant is still refused.
					LockSetForegroundWindow(LSFW_UNLOCK);

					SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
					SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

					BringWindowToTop(window);
					SetForegroundWindow(window);
				}
			}
		}

		return S_OK;
	}

	IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*) override
	{
		return S_OK;
	}

	IFACEMETHODIMP OnFileOk(IFileDialog*) override
	{
		return S_OK;
	}

	IFACEMETHODIMP OnSelectionChange(IFileDialog*) override
	{
		return S_OK;
	}

	IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) override
	{
		return S_OK;
	}

	IFACEMETHODIMP OnTypeChange(IFileDialog*) override
	{
		return S_OK;
	}

	IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*) override
	{
		return S_OK;
	}

private:
	virtual ~FileDialogServerEvents() = default;

	fdipc::Response* m_response;
	bool m_raised = false;
	LONG m_refCount = 1;
};

// The helper is a separate short-lived process, so launcher trace() (which targets the launcher's
// own log) is no use for diagnosing it in the field. Append the outcome to a small dedicated file
// next to CitizenFX.log. Narrow throughout to keep %s unambiguous (paths passed as ToNarrow()).
static void FDLog(const char* fmt, ...)
{
	static FILE* f = _wfopen(MakeRelativeCitPath(L"CitizenFX_FileDialog.log").c_str(), L"a");

	if (!f)
	{
		return;
	}

	va_list ap;
	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);

	fflush(f);
}

static void AppendResultPath(fdipc::Response* response, IShellItem* item)
{
	PWSTR filePath = nullptr;

	if (FAILED(item->GetDisplayName(SIGDN_FILESYSPATH, &filePath)) || !filePath)
	{
		return;
	}

	// strings are packed back to back, terminators included
	size_t length = wcslen(filePath) + 1;

	if (response->resultChars + length <= fdipc::kMaxResultChars)
	{
		memcpy(&response->results[response->resultChars], filePath, length * sizeof(wchar_t));
		response->resultChars += static_cast<uint32_t>(length);
		response->numFiles++;
	}

	CoTaskMemFree(filePath);
}

static void RunFileDialogServer(fdipc::SharedBlock* block)
{
	auto& request = block->request;
	auto response = &block->response;

	if (request.version != fdipc::kVersion)
	{
		return;
	}

	ScopedCoInitialize coInit(COINIT_APARTMENTTHREADED);

	if (!coInit)
	{
		return;
	}

	const CLSID dialogClsid = (request.mode == fdipc::kModeSave) ? CLSID_FileSaveDialog : CLSID_FileOpenDialog;

	WRL::ComPtr<IFileDialog> dialog;

	if (FAILED(CoCreateInstance(dialogClsid, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
	{
		return;
	}

	FILEOPENDIALOGOPTIONS options = 0;
	dialog->GetOptions(&options);

	options |= FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR;

	switch (request.mode)
	{
		case fdipc::kModeOpen:
			options |= FOS_FILEMUSTEXIST;
			break;
		case fdipc::kModeOpenMultiple:
			options |= FOS_FILEMUSTEXIST | FOS_ALLOWMULTISELECT;
			break;
		case fdipc::kModeOpenFolder:
			options |= FOS_PICKFOLDERS;
			break;
		case fdipc::kModeSave:
			options |= FOS_OVERWRITEPROMPT;
			break;
	}

	dialog->SetOptions(options);

	if (request.title[0])
	{
		dialog->SetTitle(request.title);
	}

	// default path: split into folder (SetFolder) and file name (SetFileName)
	if (request.defaultPath[0])
	{
		std::wstring normalized = request.defaultPath;
		std::replace(normalized.begin(), normalized.end(), L'/', L'\\');

		auto separator = normalized.find_last_of(L'\\');

		std::wstring folder;
		std::wstring fileName;

		if (separator == std::wstring::npos)
		{
			fileName = normalized;
		}
		else
		{
			folder = normalized.substr(0, separator);
			fileName = normalized.substr(separator + 1);
		}

		if (!folder.empty())
		{
			WRL::ComPtr<IShellItem> folderItem;

			if (SUCCEEDED(SHCreateItemFromParsingName(folder.c_str(), nullptr, IID_PPV_ARGS(&folderItem))))
			{
				dialog->SetFolder(folderItem.Get());
			}
		}

		if (!fileName.empty())
		{
			dialog->SetFileName(fileName.c_str());
		}
	}

	if (request.numFilters > 0 && request.mode != fdipc::kModeOpenFolder)
	{
		uint32_t numFilters = (std::min)(request.numFilters, static_cast<uint32_t>(fdipc::kMaxFilters));

		std::vector<COMDLG_FILTERSPEC> specs(numFilters);

		for (uint32_t i = 0; i < numFilters; i++)
		{
			specs[i].pszName = request.filterNames[i];
			specs[i].pszSpec = request.filterSpecs[i];
		}

		dialog->SetFileTypes(numFilters, specs.data());
		dialog->SetFileTypeIndex(1);

		if (request.mode == fdipc::kModeSave)
		{
			// first spec is "*.ext" or "*.ext;*.ext2" - take the leading extension
			std::wstring pattern = specs[0].pszSpec;

			if (auto end = pattern.find(L';'); end != std::wstring::npos)
			{
				pattern.resize(end);
			}

			if (auto dot = pattern.find(L'.'); dot != std::wstring::npos)
			{
				dialog->SetDefaultExtension(pattern.substr(dot + 1).c_str());
			}
		}
	}

	DWORD adviseCookie = 0;
	WRL::ComPtr<IFileDialogEvents> events;
	events.Attach(new FileDialogServerEvents(response));
	dialog->Advise(events.Get(), &adviseCookie);

	// unowned on purpose: an owner HWND from another process would get EnableWindow(FALSE)'d,
	// and a disabled game window is unrecoverable from here. The events sink raises us instead.
	const HRESULT hr = dialog->Show(nullptr);

	if (adviseCookie)
	{
		dialog->Unadvise(adviseCookie);
	}

	if (FAILED(hr))
	{
		// ERROR_CANCELLED lands here - plain cancel
		FDLog("[fd] Show hr=0x%08x (0x800704C7 = cancelled)\n", (unsigned)hr);
		return;
	}

	if (request.mode == fdipc::kModeOpenMultiple)
	{
		WRL::ComPtr<IFileOpenDialog> openDialog;
		WRL::ComPtr<IShellItemArray> items;

		if (SUCCEEDED(dialog.As(&openDialog)) && SUCCEEDED(openDialog->GetResults(&items)))
		{
			DWORD count = 0;
			items->GetCount(&count);

			for (DWORD i = 0; i < count; i++)
			{
				WRL::ComPtr<IShellItem> item;

				if (SUCCEEDED(items->GetItemAt(i, &item)))
				{
					AppendResultPath(response, item.Get());
				}
			}
		}
	}
	else
	{
		WRL::ComPtr<IShellItem> item;

		if (SUCCEEDED(dialog->GetResult(&item)))
		{
			AppendResultPath(response, item.Get());
		}
	}

	response->succeeded = (response->numFiles > 0) ? 1 : 0;

	FDLog("[fd] done succeeded=%u numFiles=%u\n", (unsigned)response->succeeded, (unsigned)response->numFiles);
}
}

bool InitializeFileDialogServer()
{
	auto commandLine = GetCommandLineW();
	auto argument = wcsstr(commandLine, L"-filedialog:");

	if (!argument)
	{
		return false;
	}

	// -filedialog:<mappingHandle>:<eventHandle>, both inherited
	wchar_t* next = nullptr;
	HANDLE mapping = reinterpret_cast<HANDLE>(wcstoull(&argument[12], &next, 10));
	HANDLE doneEvent = (next && *next == L':') ? reinterpret_cast<HANDLE>(wcstoull(next + 1, nullptr, 10)) : nullptr;

	auto block = reinterpret_cast<fdipc::SharedBlock*>(MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0));

	if (block)
	{
		// SEH so a crash mid-extract is diagnosed rather than silently exiting with succeeded=0
		__try
		{
			RunFileDialogServer(block);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			FDLog("[fd] EXCEPTION 0x%08x during RunFileDialogServer\n", (unsigned)GetExceptionCode());
		}

		UnmapViewOfFile(block);
	}

	if (doneEvent)
	{
		SetEvent(doneEvent);
	}

	// handled - the process should exit without doing anything else
	return true;
}
#endif
