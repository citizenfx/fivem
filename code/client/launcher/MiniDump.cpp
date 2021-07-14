/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <time.h>
#include <dbghelp.h>

#include <client/windows/handler/exception_handler.h>
#include <client/windows/crash_generation/client_info.h>
#include <client/windows/crash_generation/crash_generation_client.h>
#include <client/windows/crash_generation/crash_generation_server.h>
#include <common/windows/http_upload.h>

#include <CfxLocale.h>
#include <CfxState.h>
#include <CfxSubProcess.h>
#include <HostSharedData.h>

using namespace google_breakpad;

#if defined(LAUNCHER_PERSONALITY_MAIN)
#include <commctrl.h>
#include <shellapi.h>

#include <json.hpp>

#include <regex>
#include <sstream>

#include <optional>

#include <cpr/cpr.h>
#include <citversion.h>

#include <fmt/time.h>

using json = nlohmann::json;

static json load_json_file(const std::wstring& path)
{
	FILE* f = _wfopen(MakeRelativeCitPath(path).c_str(), L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		int len = ftell(f);
		fseek(f, 0, SEEK_SET);

		std::vector<uint8_t> text(len);
		fread(&text[0], 1, len, f);

		fclose(f);

		try
		{
			return json::parse(text);
		}
		catch (std::exception& e)
		{
		}
	}

	return json(nullptr);
}

static void send_sentry_session(const json& data)
{
	std::stringstream bodyData;
	bodyData << "{}\n";
	bodyData << R"({"type":"session"})" << "\n";
	bodyData << data.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace) << "\n";

	auto r = cpr::Post(
	cpr::Url{ "https://sentry.fivem.net/api/2/envelope/" },
	cpr::Body{bodyData.str()},
	cpr::VerifySsl{ false },
	cpr::Header{
		{
			"X-Sentry-Auth",
			fmt::sprintf("Sentry sentry_version=7, sentry_key=9902acf744d546e98ca357203f19278b")
		}
	},
	cpr::Timeout{ 2500 });
}

std::string g_entitlementSource;

bool LoadOwnershipTicket();

static json g_session;

static void UpdateSession(json& session)
{
	send_sentry_session(session);

	session["init"] = false;

	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\session").c_str(), L"wb");

	if (f)
	{
		auto s = session.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
		fwrite(s.data(), 1, s.size(), f);
		fclose(f);
	}

	g_session = session;
}

static void OnStartSession()
{
	auto oldSession = load_json_file(L"data\\cache\\session");

	if (!oldSession.is_null())
	{
		oldSession["status"] = "abnormal";
		send_sentry_session(oldSession);

		_wunlink(MakeRelativeCitPath(L"data\\cache\\session").c_str());
	}

	UUID uuid;
	UuidCreate(&uuid);
	char* str;
	UuidToStringA(&uuid, (RPC_CSTR*)&str);
	
	std::string sid = str;

	RpcStringFreeA((RPC_CSTR*)&str);

	LoadOwnershipTicket();

	if (g_entitlementSource.empty())
	{
		g_entitlementSource = "default";
	}

	FILE* f = _wfopen(MakeRelativeCitPath(L"citizen/release.txt").c_str(), L"r");
	std::string version;

	if (f)
	{
		char ver[128];

		fgets(ver, sizeof(ver), f);
		fclose(f);

		version = fmt::sprintf("cfx-%d", atoi(ver));
	}
	else
	{
		version = fmt::sprintf("cfx-legacy-%d", BASE_EXE_VERSION);
	}

	std::time_t t = std::time(nullptr);

	static std::string curChannel;

	wchar_t resultPath[1024];

	static std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	GetPrivateProfileString(L"Game", L"UpdateChannel", L"production", resultPath, std::size(resultPath), fpath.c_str());

	curChannel = ToNarrow(resultPath);

	auto session = json::object({ 
		{ "sid", sid },
		{ "did", g_entitlementSource },
		{ "init", true },
		{ "started", fmt::format("{:%Y-%m-%dT%H:%M:%S}Z", *std::gmtime(&t)) },
		{ "attrs", json::object({
			{ "release", version },
			{ "environment", curChannel }
		}) }
	});

	UpdateSession(session);
}

static json load_error_pickup()
{
	return load_json_file(L"data\\cache\\error-pickup");
}

static std::map<std::string, std::string> load_crashometry()
{
	std::map<std::string, std::string> rv;

	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\crashometry").c_str(), L"rb");

	if (f)
	{
		while (!feof(f))
		{
			uint32_t keyLen = 0;
			uint32_t valLen = 0;

			fread(&keyLen, 1, sizeof(keyLen), f);
			fread(&valLen, 1, sizeof(valLen), f);

			if (keyLen > 0 && valLen > 0)
			{
				std::vector<char> data(keyLen + valLen + 2);
				fread(&data[0], 1, keyLen, f);
				fread(&data[keyLen + 1], 1, valLen, f);

				rv[&data[0]] = &data[keyLen + 1];
			}
		}

		fclose(f);
	}

	return rv;
}

static std::wstring crashHash;

static std::wstring HashCrash(const std::wstring& key);

static void add_crashometry(json& data)
{
	auto map = load_crashometry();
	_wunlink(MakeRelativeCitPath(L"data\\cache\\crashometry").c_str());

	for (const auto& pair : map)
	{
		data["crashometry_" + pair.first] = pair.second;
	}

	if (!crashHash.empty())
	{
		auto ch = ToNarrow(crashHash);

		data["crash_hash"] = ch;
		data["crash_hash_id"] = HashString(ch.c_str());
		data["crash_hash_key"] = ToNarrow(HashCrash(crashHash));
	}
}

struct GameErrorData
{
	std::string errorName;
	std::string errorDescription;

	GameErrorData()
	{
	}

	GameErrorData(const std::string& errorName, const std::string& errorDescription)
		: errorName(errorName), errorDescription(errorDescription)
	{

	}
};

static GameErrorData LookupError(uint32_t hash)
{
	FILE* f = _wfopen(MakeRelativeGamePath(L"update/x64/data/errorcodes/american.txt").c_str(), L"r");

	if (f)
	{
		char line[8192] = { 0 };

		while (fgets(line, 8191, f))
		{
			if (line[0] == '[')
			{
				strrchr(line, ']')[0] = '\0';

				if (HashString(&line[1]) == hash)
				{
					char data[8192] = { 0 };
					fgets(data, 8191, f);

					return GameErrorData{ &line[1], data };
				}
			}
		}
	}

	return GameErrorData{};
}

static std::optional<std::tuple<GameErrorData, uint64_t>> LoadErrorData()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\error_out").c_str(), L"rb");

	if (f)
	{
		uint32_t error;
		uint64_t retAddr;
		fread(&error, 1, 4, f);
		fread(&retAddr, 1, 8, f);
		fclose(f);

		return { { LookupError(error), retAddr } };
	}

	return {};
}

template<typename T>
static std::string ParseLinks(const T& text)
{
	// parse hyperlinks in the error text
	std::regex url_re(R"((http|ftp|https):\/\/[\w-]+(\.[\w-]+)+([\w.,@?^=%&amp;:\/~+#-]*[\w@?^=%&amp;\/~+#-])?)");

	std::ostringstream oss;
	std::regex_replace(std::ostreambuf_iterator<char>(oss),
		text.begin(), text.end(), url_re, "<A HREF=\"$&\">$&</A>");

	return oss.str();
}

static void OverloadCrashData(TASKDIALOGCONFIG* config)
{
	// error files?
	{
		auto data = LoadErrorData();

		if (data)
		{
			_wunlink(MakeRelativeCitPath(L"data\\cache\\error_out").c_str());

			static GameErrorData errData = std::get<GameErrorData>(*data);
			static uint64_t retAddr = std::get<uint64_t>(*data);

			if (errData.errorName.empty())
			{
				errData.errorName = "UNKNOWN";
				errData.errorDescription = "";
			}

			static std::wstring errTitle = fmt::sprintf(gettext(L"RAGE error: %s"), ToWide(errData.errorName));
			static std::wstring errDescription = fmt::sprintf(gettext(L"A game error (at %016llx) caused %s to stop working. "
				L"A crash report has been uploaded to the %s developers.\n\n%s"),
				retAddr,
				PRODUCT_NAME,
				PRODUCT_NAME,
				ToWide(ParseLinks(errData.errorDescription)));

			config->pszMainInstruction = errTitle.c_str();
			config->pszContent = errDescription.c_str();

			return;
		}
	}

	// FatalError crash pickup?
	{
		json pickup = load_error_pickup();

		if (!pickup.is_null())
		{
			static std::wstring errTitle = fmt::sprintf(PRODUCT_NAME L" has encountered an error");
			static std::wstring errDescription = ToWide(ParseLinks(pickup["message"].get<std::string>()));

			if (errDescription.find(L'\n') != std::string::npos)
			{
				auto nlPos = errDescription.find_first_of(L"\r\n");
				errTitle = errDescription.substr(0, nlPos);

				if (auto msgStart = errDescription.find_first_not_of(L"\r\n", nlPos); msgStart
																					  != std::string::npos)
				{
					errDescription = errDescription.substr(msgStart);
				}
			}

			config->pszMainInstruction = errTitle.c_str();
			config->pszContent = errDescription.c_str();

			config->dwFlags &= ~(TDF_USE_COMMAND_LINKS | TDF_EXPANDED_BY_DEFAULT | TDF_SHOW_PROGRESS_BAR);
			config->pszMainIcon = TD_ERROR_ICON;

			static std::wstring saveStr = gettext(L"Save information");
			const_cast<TASKDIALOG_BUTTON*>(config->pButtons)[0].pszButtonText = saveStr.c_str();

			return;
		}
	}

	// module blame?
	const wchar_t* blame = nullptr;
	const wchar_t* blame_two = nullptr;

	if (wcsstr(crashHash.c_str(), L"nvwgf"))
	{
		blame = L"NVIDIA GPU drivers";
		blame_two = L"This is not the fault of the " PRODUCT_NAME L" developers, and can not be resolved by them. NVIDIA does not provide any error reporting contacts to use to report this problem, nor do they provide "
			L"debugging information that the developers can use to resolve this issue.";
	}

	if (wcsstr(crashHash.c_str(), L"guard64"))
	{
		blame = L"Comodo Internet Security";
		blame_two = L"Please uninstall Comodo Internet Security and try again, or report the issue on the Comodo forums.";
	}

	if (wcsstr(crashHash.c_str(), L".asi"))
	{
		blame = va(L"a third-party game plugin (%s)", crashHash);
		blame_two = L"Please try removing the above file from the \"plugins\" folder in your " PRODUCT_NAME L" installation and restarting the game.";
	}

	if (wcsstr(crashHash.c_str(), L"atidxx"))
	{
		blame = L"AMD GPU drivers";
		blame_two = L"Please try updating your Radeon Software, restarting your PC and then starting the game again.";
	}

	if (blame)
	{
		static std::wstring errTitle = fmt::sprintf(L"%s encountered an error", blame);
		static std::wstring errDescription = fmt::sprintf(L"FiveM crashed due to %s.\n%s", blame, blame_two);

		config->pszMainInstruction = errTitle.c_str();
		config->pszContent = errDescription.c_str();

		return;
	}
}

static std::string exType;
static std::string exWhat;

static std::wstring GetAdditionalData()
{
	{
		auto errorData = LoadErrorData();

		if (errorData)
		{
			json jsonData = json::object({
				{ "type", "rage_error" },
				{ "key", std::get<GameErrorData>(*errorData).errorName },
				{ "description", std::get<GameErrorData>(*errorData).errorDescription },
				{ "retAddr", std::get<uint64_t>(*errorData) },
			});

			add_crashometry(jsonData);

			return ToWide(jsonData.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));
		}
	}

	{
		json error_pickup = load_error_pickup();

		if (!error_pickup.is_null())
		{
			if (error_pickup["line"] != 99999)
			{
				error_pickup["type"] = "error_pickup";
			}

			add_crashometry(error_pickup);

			return ToWide(error_pickup.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));
		}
	}

	{
		json data = json::object();
		add_crashometry(data);

		if (!exType.empty())
		{
			data["exception"] = exType;
		}

		if (!exWhat.empty())
		{
			data["what"] = exWhat;
		}

		return ToWide(data.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));
	}
}

#include <psapi.h>

static const char* const wordList[256] = {
#include "CrashWordList.h"
};

static std::wstring HashCrash(const std::wstring& key)
{
	uint32_t hash = HashString(ToNarrow(key).c_str());

	return ToWide(fmt::sprintf("%s-%s-%s",
		std::string{ wordList[(hash >>  0) & 0xFF] },
		std::string{ wordList[(hash >>  8) & 0xFF] },
		std::string{ wordList[(hash >> 16) & 0xFF] }
	));
}

static std::wstring UnblameCrash(const std::wstring& hash)
{
	auto retval = hash;

	if (_wcsnicmp(hash.c_str(), L"fivem.exe+", 10) == 0)
	{
		retval = L"GTA5+" + retval.substr(10);
	}
	else if (_wcsnicmp(hash.c_str(), L"redm.exe+", 9) == 0)
	{
		retval = L"RDR2+" + retval.substr(9);
	}

	return retval;
}

void SteamInput_Reset();
void NVSP_ShutdownSafely();
#endif

// a safe exception buffer to be allocated in low (32-bit) memory to contain what() data
struct ExceptionBuffer
{
	char data[4096];
};

static ExceptionBuffer* g_exceptionBuffer;

static void AllocateExceptionBuffer()
{
	auto _NtAllocateVirtualMemory = (HRESULT(WINAPI*)(
		HANDLE    ProcessHandle,
		PVOID     *BaseAddress,
		ULONG_PTR ZeroBits,
		PSIZE_T   RegionSize,
		ULONG     AllocationType,
		ULONG     Protect
		))GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtAllocateVirtualMemory");

	PVOID baseAddr = NULL;
	SIZE_T size = sizeof(ExceptionBuffer);
	
	if (SUCCEEDED(_NtAllocateVirtualMemory(GetCurrentProcess(), &baseAddr, 0xFFFFFFFF80000000, &size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)))
	{
		g_exceptionBuffer = (ExceptionBuffer*)baseAddr;
	}
}

extern "C" DLL_EXPORT DWORD WINAPI RemoteExceptionFunc(LPVOID objectPtr)
{
	__try
	{
		std::exception* object = (std::exception*)objectPtr;

		if (g_exceptionBuffer)
		{
			strncpy(g_exceptionBuffer->data, object->what(), sizeof(g_exceptionBuffer->data));
			g_exceptionBuffer->data[sizeof(g_exceptionBuffer->data) - 1] = '\0';

			return (DWORD)(DWORD_PTR)g_exceptionBuffer;
		}

		return 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return 0;
	}
}

extern "C" DLL_EXPORT DWORD WINAPI BeforeTerminateHandler(LPVOID arg)
{
	__try
	{
		auto coreRt = GetModuleHandleW(L"CoreRT.dll");

		if (coreRt)
		{
			auto func = (void(*)(void*))GetProcAddress(coreRt, "CoreOnProcessAbnormalTermination");

			if (func)
			{
				func(arg);
			}
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}

	return 0;
}

#if defined(LAUNCHER_PERSONALITY_MAIN)
// c/p from ros-patches:five
// #TODO: factor out sanely

// {E091E21C-C61F-49F6-8560-CEF64DC42002}
#include <KnownFolders.h>
#include <ShlObj.h>

#include <dpapi.h>

#define INITGUID
#include <guiddef.h>

// {38D8F400-AA8A-4784-A9F0-26A08628577E}
DEFINE_GUID(CfxStorageGuid,
	0x38d8f400, 0xaa8a, 0x4784, 0xa9, 0xf0, 0x26, 0xa0, 0x86, 0x28, 0x57, 0x7e);

#pragma comment(lib, "rpcrt4.lib")

std::string GetOwnershipPath()
{
	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &appDataPath))) {
		std::string cfxPath = ToNarrow(appDataPath) + "\\DigitalEntitlements";
		CreateDirectory(ToWide(cfxPath).c_str(), nullptr);

		CoTaskMemFree(appDataPath);

		RPC_CSTR str;
		UuidToStringA(&CfxStorageGuid, &str);

		cfxPath += "\\";
		cfxPath += (char*)str;

		RpcStringFreeA(&str);

		return cfxPath;
	}

	return "";
}

#include "mz.h"
#include "mz_os.h"
#include "mz_strm.h"
#include "mz_strm_buf.h"
#include "mz_strm_split.h"
#include "mz_zip.h"
#include "mz_zip_rw.h"

#include <wrl.h>

namespace WRL = Microsoft::WRL;

static std::wstring g_dumpPath;

static HRESULT GetUIObjectOfFile(HWND hwnd, LPCWSTR pszPath, REFIID riid, void** ppv)
{
	*ppv = NULL;
	HRESULT hr;
	LPITEMIDLIST pidl;
	SFGAOF sfgao;
	if (SUCCEEDED(hr = SHParseDisplayName(pszPath, NULL, &pidl, 0, &sfgao))) {
		IShellFolder* psf;
		LPCITEMIDLIST pidlChild;
		if (SUCCEEDED(hr = SHBindToParent(pidl, IID_IShellFolder,
			(void**)& psf, &pidlChild))) {
			hr = psf->GetUIObjectOf(hwnd, 1, &pidlChild, riid, NULL, ppv);
			psf->Release();
		}
		CoTaskMemFree(pidl);
	}
	return hr;
}

struct TickCountData
{
	uint64_t tickCount;
	SYSTEMTIME initTime;

	TickCountData()
	{
		tickCount = GetTickCount64();
		GetSystemTime(&initTime);
	}
};

static void GatherCrashInformation()
{
	void* writer = nullptr;

	SYSTEMTIME curTime;
	GetSystemTime(&curTime);

	std::wstring tempDir = _wgetenv(L"temp");
	tempDir += fmt::sprintf(L"\\CfxCrashDump_%04d_%02d_%02d_%02d_%02d_%02d.zip", curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond);

	mz_zip_writer_create(&writer);
	mz_zip_writer_set_compress_level(writer, 9);
	mz_zip_writer_set_compress_method(writer, MZ_COMPRESS_METHOD_DEFLATE);

	static HostSharedData<TickCountData> initTickCount("CFX_SharedTickCount");

	bool success = false;
	
	int err = mz_zip_writer_open_file(writer, ToNarrow(tempDir).c_str(), 0, false);

	if (err == MZ_OK)
	{
		static fwPlatformString dateStamp = fmt::sprintf(L"%04d-%02d-%02dT%02d%02d%02d", initTickCount->initTime.wYear, initTickCount->initTime.wMonth,
			initTickCount->initTime.wDay, initTickCount->initTime.wHour, initTickCount->initTime.wMinute, initTickCount->initTime.wSecond);

		static fwPlatformString fp = MakeRelativeCitPath(fmt::sprintf(L"logs/CitizenFX_log_%s.log", dateStamp));

		err = mz_zip_writer_add_path(writer, ToNarrow(fp).c_str(), nullptr, false, false);

		if (err == MZ_OK)
		{
			err = mz_zip_writer_add_path(writer, ToNarrow(g_dumpPath).c_str(), nullptr, false, false);

			if (err == MZ_OK)
			{
				auto extraDumpFiles = {
					L"data\\cache\\extra_dump_info.bin",
					L"data\\cache\\extra_dump_info2.bin",
					L"data\\game-storage\\ros_launcher_documents2\\launcher.log",
					L"data\\game-storage\\ros_documents2\\socialclub.log",
					L"data\\game-storage\\ros_documents2\\socialclub_launcher.log"
				};

				for (auto path : extraDumpFiles)
				{
					auto extraDumpPath = MakeRelativeCitPath(path);

					if (GetFileAttributesW(extraDumpPath.c_str()) != INVALID_FILE_ATTRIBUTES)
					{
						mz_zip_writer_add_path(writer, ToNarrow(extraDumpPath).c_str(), nullptr, false, false);
					}
				}

				success = true;
			}
		}
	}

	err = mz_zip_writer_close(writer);

	if (err == MZ_OK)
	{
		if (success)
		{
			// initialize OLE
			OleInitialize(nullptr);

			// copy the file to the clipboard
			WRL::ComPtr<IDataObject> dataObject;
			GetUIObjectOfFile(nullptr, tempDir.c_str(), IID_PPV_ARGS(&dataObject));

			OleSetClipboard(dataObject.Get());
			OleFlushClipboard();

			// open message box
			MessageBoxW(NULL, va(L"Saved the crash dump as %s. Please upload the .zip file when you're asking for support. (copy/paste to upload)", tempDir), L"CitizenFX", MB_OK | MB_ICONINFORMATION);

			// open Explorer with the file selected
			STARTUPINFOW si = { 0 };
			si.cb = sizeof(si);

			PROCESS_INFORMATION pi;

			CreateProcessW(nullptr, const_cast<wchar_t*>(va(L"explorer /select,\"%s\"", tempDir)), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
		}
	}

	mz_zip_writer_delete(&writer);
}

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

bool LoadOwnershipTicket()
{
	std::string filePath = GetOwnershipPath();

	FILE* f = _wfopen(ToWide(filePath).c_str(), L"rb");

	if (!f)
	{
		return false;
	}

	std::vector<uint8_t> fileData;
	int pos;

	// get the file length
	fseek(f, 0, SEEK_END);
	pos = ftell(f);
	fseek(f, 0, SEEK_SET);

	// resize the buffer
	fileData.resize(pos);

	// read the file and close it
	fread(&fileData[0], 1, pos, f);

	fclose(f);

	// decrypt the stored data - setup blob
	DATA_BLOB cryptBlob;
	cryptBlob.pbData = &fileData[0];
	cryptBlob.cbData = fileData.size();

	DATA_BLOB outBlob;

	// call DPAPI
	if (CryptUnprotectData(&cryptBlob, nullptr, nullptr, nullptr, nullptr, 0, &outBlob))
	{
		// parse the file
		std::string data(reinterpret_cast<char*>(outBlob.pbData), outBlob.cbData);

		// free the out data
		LocalFree(outBlob.pbData);

		rapidjson::Document doc;
		doc.Parse(data.c_str(), data.size());

		if (!doc.HasParseError())
		{
			if (doc.IsObject())
			{
				g_entitlementSource = doc["guid"].GetString();
				return true;
			}
		}
	}

	return false;
}

// copied here so that we don't have to rebuild Shared (and HostSharedData does not support custom names)
struct MDSharedTickCount
{
	struct Data
	{
		uint64_t tickCount;

		Data()
		{
			tickCount = GetTickCount64();
		}
	};

	MDSharedTickCount()
	{
		m_data = &m_fakeData;

		bool initTime = true;
		m_fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(Data), L"CFX_SharedTickCount");

		if (m_fileMapping != nullptr)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				initTime = false;
			}

			m_data = (Data*)MapViewOfFile(m_fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Data));

			if (initTime)
			{
				m_data = new(m_data) Data();
			}
		}
	}

	inline Data& operator*()
	{
		return *m_data;
	}

	inline Data* operator->()
	{
		return m_data;
	}

private:
	HANDLE m_fileMapping;
	Data* m_data;

	Data m_fakeData;
};

#include "UserLibrary.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

#include <winternl.h>

static LPTHREAD_START_ROUTINE GetFunc(HANDLE hProcess, const char* name)
{
	wchar_t modPath[MAX_PATH];
	DWORD modPathSize = MAX_PATH;
	if (!QueryFullProcessImageNameW(hProcess, 0, modPath, &modPathSize))
	{
		return NULL;
	}

	PROCESS_BASIC_INFORMATION pbi;
	DWORD pbil = sizeof(pbi);
	if (FAILED(NtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, pbil, NULL)))
	{
		return NULL;
	}

#ifdef _WIN64
	size_t baseOffset = 0x10;
#else
	size_t baseOffset = 0x08;
#endif

	uintptr_t imageBase = 0;
	size_t memRead = 0;
	if (!ReadProcessMemory(hProcess, (char*)pbi.PebBaseAddress + baseOffset, &imageBase, sizeof(imageBase), &memRead) || memRead != sizeof(imageBase))
	{
		return NULL;
	}

	UserLibrary lib(modPath);
	auto off = lib.GetExportCode(name);

	if (off == 0)
	{
		return NULL;
	}

	return (LPTHREAD_START_ROUTINE)((char*)imageBase + off);
}

extern nlohmann::json SymbolicateCrash(HANDLE hProcess, HANDLE hThread, PEXCEPTION_RECORD er, PCONTEXT ctx);
extern void ParseSymbolicCrash(nlohmann::json& crash, std::string* signature, std::string* stackTrace);

void InitializeDumpServer(int inheritedHandle, int parentPid)
{
	static bool g_running = true;

	// needed to initialize logging(!)
	trace("DumpServer is active and waiting.\n");

	HANDLE inheritedHandleBit = (HANDLE)inheritedHandle;
	static HANDLE parentProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | SYNCHRONIZE | PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, parentPid);
	static HANDLE crashReport = CreateEvent(NULL, TRUE, TRUE, NULL);

	CrashGenerationServer::OnClientConnectedCallback connectCallback = [] (void*, const ClientInfo* info)
	{

	};

	CrashGenerationServer::OnClientDumpRequestCallback dumpCallback = [] (void*, const ClientInfo* info, const std::wstring* filePath)
	{
		// we're going to be reporting, unsignal
		ResetEvent(crashReport);

		auto process_handle = info->process_handle();
		DWORD exceptionCode = 0;

		json symCrash;

		{
			EXCEPTION_POINTERS* ei;
			if (info->GetClientExceptionInfo(&ei))
			{
				auto readClient = [&](void* ptr, auto* out)
				{
					SIZE_T bytes_count = 0;
					if (!ReadProcessMemory(process_handle,
						ptr,
						out,
						sizeof(*out),
						&bytes_count)) {
						return false;
					}

					return bytes_count == sizeof(*out);
				};

				EXCEPTION_POINTERS ep;
				if (readClient(ei, &ep))
				{
					EXCEPTION_RECORD ex;
					CONTEXT cx;

					bool valid = readClient(ep.ExceptionRecord, &ex);
					valid = valid && readClient(ep.ContextRecord, &cx);

					if (valid)
					{
						DWORD thread;
						if (info->GetClientThreadId(&thread))
						{
							auto th = OpenThread(THREAD_ALL_ACCESS, FALSE, thread);

							if (th)
							{
								symCrash = SymbolicateCrash(process_handle, th, &ex, &cx);
							}
						}

						bool moduleFound = false;
						DWORD processLen = 0;
						if (EnumProcessModules(process_handle, nullptr, 0, &processLen))
						{
							std::vector<HMODULE> buffer(processLen / sizeof(HMODULE));

							if (EnumProcessModules(process_handle, buffer.data(), buffer.size() * sizeof(HMODULE), &processLen))
							{
								for (HMODULE module : buffer)
								{
									const wchar_t* moduleBaseString = L"";
									MODULEINFO mi;

									if (GetModuleInformation(process_handle, module, &mi, sizeof(mi)))
									{
										auto base = reinterpret_cast<char*>(mi.lpBaseOfDll);

										if (ex.ExceptionAddress >= base && ex.ExceptionAddress < (base + mi.SizeOfImage))
										{
											wchar_t filename[MAX_PATH] = { 0 };
											GetModuleFileNameExW(process_handle, module, filename, _countof(filename));

											if (wcsstr(filename, L".exe") != nullptr)
											{
#ifdef GTA_FIVE
												wcscpy(filename, L"\\FiveM.exe");
#elif defined(IS_RDR3)
												wcscpy(filename, L"\\RedM.exe");
#else
												wcscpy(filename, L"\\CitiLaunch.exe");
#endif
											}

											// lowercase the filename
											for (wchar_t* p = filename; *p; ++p)
											{
												if (*p >= 'A' && *p <= 'Z')
												{
													*p += 0x20;
												}
											}

											// create the string
											moduleBaseString = va(L"%s+%X", wcsrchr(filename, '\\') + 1, (uintptr_t)((char*)ex.ExceptionAddress - (char*)module));

											crashHash = moduleBaseString;
											moduleFound = true;

											break;
										}
									}
								}
							}
						}

						if (!moduleFound)
						{
							// is this an unloaded module?
							typedef VOID (WINAPI* _tRtlGetUnloadEventTraceEx)(
							_Out_ PULONG * ElementSize,
							_Out_ PULONG * ElementCount,
							_Out_ PVOID * EventTrace);

							typedef struct _RTL_UNLOAD_EVENT_TRACE
							{
								PVOID BaseAddress; // Base address of dll
								SIZE_T SizeOfImage; // Size of image
								ULONG Sequence; // Sequence number for this event
								ULONG TimeDateStamp; // Time and date of image
								ULONG CheckSum; // Image checksum
								WCHAR ImageName[32]; // Image name
							} RTL_UNLOAD_EVENT_TRACE, *PRTL_UNLOAD_EVENT_TRACE;

							// collect memory addresses of unloaded modules in *client*
							auto _RtlGetUnloadEventTraceEx = (_tRtlGetUnloadEventTraceEx)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetUnloadEventTraceEx");

							if (_RtlGetUnloadEventTraceEx)
							{
								PULONG RtlpUnloadEventTraceExSizePtr;
								PULONG RtlpUnloadEventTraceExNumberPtr;
								PVOID RtlpUnloadEventTraceExPtr;

								_RtlGetUnloadEventTraceEx(&RtlpUnloadEventTraceExSizePtr, &RtlpUnloadEventTraceExNumberPtr, &RtlpUnloadEventTraceExPtr);

								// these addresses are going to be the same in the client process, so...
								PCHAR RtlpUnloadEventTraceEx = NULL;
								ULONG RtlpUnloadEventTraceExSize = 0;
								ULONG RtlpUnloadEventTraceExNumber = 0;

								bool canDo = readClient(RtlpUnloadEventTraceExSizePtr, &RtlpUnloadEventTraceExSize)
									&& readClient(RtlpUnloadEventTraceExNumberPtr, &RtlpUnloadEventTraceExNumber)
									&& readClient(RtlpUnloadEventTraceExPtr, &RtlpUnloadEventTraceEx);

								if (canDo && RtlpUnloadEventTraceExSize >= sizeof(RTL_UNLOAD_EVENT_TRACE))
								{
									for (ULONG idx = 0; idx < RtlpUnloadEventTraceExNumber; idx++)
									{
										RTL_UNLOAD_EVENT_TRACE traceEntry;
										if (readClient(RtlpUnloadEventTraceEx + (idx * RtlpUnloadEventTraceExSize), &traceEntry))
										{
											auto base = reinterpret_cast<char*>(traceEntry.BaseAddress);

											if (ex.ExceptionAddress >= base && ex.ExceptionAddress < (base + traceEntry.SizeOfImage))
											{
												wchar_t filename[MAX_PATH] = { 0 };
												wcscpy(filename, traceEntry.ImageName);

												// lowercase the filename
												for (wchar_t* p = filename; *p; ++p)
												{
													if (*p >= 'A' && *p <= 'Z')
													{
														*p += 0x20;
													}
												}

												// create the string
												auto moduleBaseString = va(L"%s_unloaded+%X", filename, (uintptr_t)((char*)ex.ExceptionAddress - base));
												crashHash = moduleBaseString;

												break;
											}
										}
									}
								}
							}
						}

						// store exception code
						exceptionCode = ex.ExceptionCode;

						// try parsing any C++ exception
						if (ex.ExceptionCode == 0xE06D7363 && ex.ExceptionInformation[0] == 0x19930520)
						{
							struct CatchableType
							{
								__int32 properties;
								__int32 pType;
								__int32 thisDisplacement;
								__int32 sizeOrOffset;
								__int32 copyFunction;
							};

							struct ThrowInfo
							{
								__int32 attributes;
								__int32 pmfnUnwind;
								__int32 pForwardCompat;
								__int32 pCatchableTypeArray;
							};

							struct CatchableTypeArray
							{
								__int32 count;
								__int32 pFirstType;
							};

							ThrowInfo ti;
							if (readClient((void*)ex.ExceptionInformation[2], &ti))
							{
								CatchableTypeArray cta;

								if (readClient((void*)(ex.ExceptionInformation[3] + ti.pCatchableTypeArray), &cta))
								{
									CatchableType type;

									if (cta.count > 0 && readClient((void*)(ex.ExceptionInformation[3] + cta.pFirstType), &type))
									{
										struct tid
										{
											const char* undName;
											uint8_t name[4096];
										} ti;

										if (type.pType && readClient((void*)(ex.ExceptionInformation[3] + type.pType), &ti))
										{
											ti.undName = nullptr;

											std::type_info& typeInfo = *(std::type_info*)&ti;
											exType = typeInfo.name();

											// strip `class ` prefix
											if (exType.substr(0, 6) == "class ")
											{
												exType = exType.substr(6);
											}

											// try getting exception data as well
											auto func = GetFunc(process_handle, "RemoteExceptionFunc");

											if (func)
											{
												HANDLE hThread = CreateRemoteThread(process_handle, NULL, 0, func, (void*)(ex.ExceptionInformation[1] + type.thisDisplacement), 0, NULL);
												WaitForSingleObject(hThread, 5000);

												DWORD ret = 0;

												if (GetExitCodeThread(hThread, &ret))
												{
													void* exPtr = (void*)ret;

													ExceptionBuffer buf;

													if (exPtr && readClient(exPtr, &buf))
													{
														exWhat = buf.data;
													}
												}

												CloseHandle(hThread);
											}
										}
									}
								}
							}
						}
					}
				}
			}
			
		}

		std::map<std::wstring, std::wstring> parameters;
#ifdef GTA_NY
		parameters[L"ProductName"] = L"CitizenFX";
		parameters[L"Version"] = L"1.0";
		parameters[L"BuildID"] = L"20141213000000"; // todo i bet
#elif defined(GTA_FIVE)
		LoadOwnershipTicket();

		if (g_entitlementSource.empty())
		{
			g_entitlementSource = "default";
		}

		parameters[L"ProductName"] = L"FiveM";

		FILE* f = _wfopen(MakeRelativeCitPath(L"citizen/release.txt").c_str(), L"r");

		if (f)
		{
			char ver[128];

			fgets(ver, sizeof(ver), f);
			fclose(f);

			parameters[L"Version"] = va(L"cfx-%d", atoi(ver));
		}
		else
		{
			parameters[L"Version"] = va(L"cfx-legacy-%d", BASE_EXE_VERSION);
		}

		parameters[L"BuildID"] = L"20170101";
		parameters[L"UserID"] = ToWide(g_entitlementSource);

        parameters[L"prod"] = L"FiveM";
        parameters[L"ver"] = L"1.0";
#endif

		auto crashometry = load_crashometry();

		parameters[L"ReleaseChannel"] = L"release";

		parameters[L"AdditionalData"] = GetAdditionalData();

		{
			static MDSharedTickCount tickCount;
			parameters[L"StartTime"] = fmt::sprintf(L"%lld", _time64(nullptr) - ((GetTickCount64() - tickCount->tickCount) / 1000));
		}

		std::wstring responseBody;
		int responseCode;

		std::map<std::wstring, std::wstring> files;
		files[L"upload_file_minidump"] = *filePath;

		static HostSharedData<TickCountData> initTickCount("CFX_SharedTickCount");
		static fwPlatformString dateStamp = fmt::sprintf(L"%04d-%02d-%02dT%02d%02d%02d", initTickCount->initTime.wYear, initTickCount->initTime.wMonth,
			initTickCount->initTime.wDay, initTickCount->initTime.wHour, initTickCount->initTime.wMinute, initTickCount->initTime.wSecond);

		static fwPlatformString fp = MakeRelativeCitPath(fmt::sprintf(L"logs/CitizenFX_log_%s.log", dateStamp));

		files[L"upload_file_log"] = fp;

		// avoid libcef.dll subprocess crashes terminating the entire job
		bool shouldTerminate = true;
		bool shouldUpload = true;

		if (GetProcessId(parentProcess) != GetProcessId(info->process_handle()))
		{
			wchar_t imageName[MAX_PATH];
			GetProcessImageFileNameW(info->process_handle(), imageName, std::size(imageName));

			if (wcsstr(imageName, L"GameRuntime") != nullptr)
			{
				shouldTerminate = false;
			}

			if (crashHash.find(L"libcef") != std::string::npos)
			{
				shouldTerminate = false;

				// we want a cef.log and don't want the core log (given its frequency)
				files[L"cef_log"] = MakeRelativeCitPath(L"cef.log");
				files.erase(L"upload_file_log");
			}

			// NVIDIA crashes in Chrome GPU process
			if (wcsstr(imageName, L"GTAProcess") == nullptr && crashHash.find(L"nvwgf2") != std::string::npos)
			{
				shouldTerminate = false;
				shouldUpload = false;
			}

			// Chrome OOM situations (kOomExceptionCode)
			if (exceptionCode == 0xE0000008)
			{
				shouldTerminate = false;
				shouldUpload = false;
			}
		}

		static std::wstring windowTitle = PRODUCT_NAME;
		static std::wstring mainInstruction = PRODUCT_NAME L" has stopped working";
		
		std::wstring cuz = L"An error";
		std::string stackTrace;
		std::string csignature;

		if (!symCrash.is_null())
		{
			ParseSymbolicCrash(symCrash, &csignature, &stackTrace);
		}

		if (!crashHash.empty())
		{
			auto ch = UnblameCrash(crashHash);

			if (!csignature.empty())
			{
				ch = ToWide(csignature);
			}

			if (crashHash.find(L".exe") != std::string::npos)
			{
				windowTitle = fmt::sprintf(gettext(L"Error %s"), ch);
			}

			mainInstruction = fmt::sprintf(L"%s", ch);
			cuz = fmt::sprintf(gettext(L"An error at %s"), ch);

			json crashData = load_json_file(L"citizen/crash-data.json");

			if (crashData.is_object())
			{
				auto cd = crashData.value(ToNarrow(HashCrash(ch)), "");

				if (!cd.empty())
				{
					mainInstruction = gettext(L"FiveM crashed... but we're on it!");
					cd += "\n\n";
				}

				cuz = ToWide(cd) + cuz;
			}
		}

		if (!exType.empty())
		{
			mainInstruction = L"Exception, unhandled!";

			cuz = ToWide(fmt::sprintf("An unhandled exception (of type %s)", exType));
		}

		static std::wstring content = fmt::sprintf(gettext(L"%s caused %s to stop working. A crash report is being uploaded to the %s developers."), cuz, PRODUCT_NAME, PRODUCT_NAME);

		if (!exWhat.empty())
		{
			content += fmt::sprintf(gettext(L"\n\nException details: %s"), ToWide(exWhat));
		}

		if (!crashHash.empty() && crashHash.find(L".exe") != std::string::npos)
		{
			content += fmt::sprintf(gettext(L"\n\nLegacy crash hash: %s"), HashCrash(crashHash));
		}

		if (!stackTrace.empty())
		{
			content += fmt::sprintf(gettext(L"\nStack trace:\n%s"), ToWide(stackTrace));
		}

		if (shouldTerminate)
		{
			std::thread([csignature]()
			{
				static HostSharedData<CfxState> hostData("CfxInitState");
				HANDLE gameProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, hostData->gamePid);

				if (!gameProcess)
				{
					gameProcess = parentProcess;
				}

				if (gameProcess)
				{
					std::string friendlyReason = ToNarrow(HashCrash(crashHash) + L" (" + UnblameCrash(crashHash) + L")");

					if (!csignature.empty())
					{
						friendlyReason = csignature;
					}

					if (!exType.empty())
					{
						friendlyReason = gettext("Unhandled exception: ") + exType;
					}

					friendlyReason = gettext("Game crashed: ") + friendlyReason;

					LPVOID memPtr = VirtualAllocEx(gameProcess, NULL, friendlyReason.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

					if (memPtr)
					{
						WriteProcessMemory(gameProcess, memPtr, friendlyReason.data(), friendlyReason.size() + 1, NULL);
					}

					auto func = GetFunc(gameProcess, "BeforeTerminateHandler");

					if (func)
					{
						HANDLE hThread = CreateRemoteThread(gameProcess, NULL, 0, func, memPtr, 0, NULL);

						if (hThread)
						{
							WaitForSingleObject(hThread, 7500);
							CloseHandle(hThread);
						}
					}
				}

				TerminateProcess(parentProcess, -2);
			}).detach();

			g_session["status"] = "crashed";

			UpdateSession(g_session);
		}

		static std::optional<std::wstring> crashId;
		static std::optional<std::wstring> crashIdError;

		static auto saveStr = gettext(L"Save information\nStores a file with crash information that you should copy and upload when asking for help.");

		static TASKDIALOG_BUTTON buttons[] = {
			{ 42, saveStr.c_str() }
		};

		static std::wstring tempSignature = fmt::sprintf(gettext(L"Crash signature: %s\nReport ID: ... [uploading]\nYou can press Ctrl-C to copy this message and paste it elsewhere."), crashHash);
		static bool isDisconnectMessage = false;
		isDisconnectMessage = false;

		if (crashometry.find("kill_network_msg") != crashometry.end() && crashometry.find("reload_game") == crashometry.end())
		{
			windowTitle = L"Disconnected";
			mainInstruction = L"O\x448\x438\x431\x43A\x430 (Error)";

			content = ToWide(crashometry["kill_network_msg"]);

			isDisconnectMessage = true;
		}

		static std::thread saveThread;

		static TASKDIALOGCONFIG taskDialogConfig = { 0 };
		taskDialogConfig.cbSize = sizeof(taskDialogConfig);
		taskDialogConfig.hInstance = GetModuleHandle(nullptr);
		taskDialogConfig.dwFlags = TDF_ENABLE_HYPERLINKS | TDF_EXPAND_FOOTER_AREA | TDF_SHOW_PROGRESS_BAR | TDF_CALLBACK_TIMER | TDF_USE_COMMAND_LINKS | TDF_EXPANDED_BY_DEFAULT;
		taskDialogConfig.dwCommonButtons = TDCBF_CLOSE_BUTTON;
		taskDialogConfig.cButtons = 1;
		taskDialogConfig.pButtons = buttons;
		taskDialogConfig.pszWindowTitle = windowTitle.c_str();
		taskDialogConfig.pszMainIcon = MAKEINTRESOURCEW(-7); // shield bar w/ error color
		taskDialogConfig.pszMainInstruction = mainInstruction.c_str();
		taskDialogConfig.pszContent = content.c_str();
		taskDialogConfig.pszExpandedInformation = tempSignature.c_str();
		taskDialogConfig.pfCallback = [](HWND hWnd, UINT type, WPARAM wParam, LPARAM lParam, LONG_PTR data)
		{
			if (type == TDN_HYPERLINK_CLICKED)
			{
				ShellExecute(nullptr, L"open", (LPCWSTR)lParam, nullptr, nullptr, SW_NORMAL);
			}
			else if (type == TDN_BUTTON_CLICKED)
			{
				if (wParam == 42)
				{
					SendMessage(hWnd, TDM_ENABLE_BUTTON, 42, 0);

					saveThread = std::thread([]()
					{
						GatherCrashInformation();
					});
				}
				else
				{
					return S_OK;
				}
			}
			else if (type == TDN_CREATED)
			{
				SendMessage(hWnd, TDM_ENABLE_BUTTON, IDCLOSE, 0);
				SendMessage(hWnd, TDM_SET_MARQUEE_PROGRESS_BAR, 1, 0);
				SendMessage(hWnd, TDM_SET_PROGRESS_BAR_MARQUEE, 1, 15);
				SendMessage(hWnd, TDM_UPDATE_ICON, TDIE_ICON_MAIN, (LPARAM)TD_ERROR_ICON);
			}
			else if (type == TDN_TIMER)
			{
				if (crashId)
				{
					if (!crashId->empty())
					{
						SendMessage(hWnd, TDM_SET_ELEMENT_TEXT, TDE_EXPANDED_INFORMATION, (WPARAM)va(gettext(L"Crash signature: %s\nReport ID: %s\nYou can press Ctrl-C to copy this message and paste it elsewhere."), crashHash.c_str(), crashId->c_str()));
					}
					else if (crashIdError && !crashIdError->empty())
					{
						SendMessage(hWnd, TDM_SET_ELEMENT_TEXT, TDE_EXPANDED_INFORMATION, (WPARAM)va(gettext(L"Crash signature: %s\n%s\nYou can press Ctrl-C to copy this message and paste it elsewhere."), crashHash.c_str(), crashIdError->c_str()));
					}

					SendMessage(hWnd, TDM_ENABLE_BUTTON, IDCLOSE, 1);
					SendMessage(hWnd, TDM_SET_MARQUEE_PROGRESS_BAR, 0, 0);
					SendMessage(hWnd, TDM_SET_PROGRESS_BAR_POS, 100, 0);
					SendMessage(hWnd, TDM_SET_PROGRESS_BAR_STATE, PBST_NORMAL, 0);

					if (crashId->empty())
					{
						SendMessage(hWnd, TDM_SET_PROGRESS_BAR_STATE, PBST_ERROR, 0);
					}

					crashId.reset();
				}
			}

			return S_FALSE;
		};

		// make the disconnect message less confusing
		if (isDisconnectMessage)
		{
			taskDialogConfig.dwFlags &= ~(TDF_USE_COMMAND_LINKS | TDF_EXPANDED_BY_DEFAULT);
			taskDialogConfig.pszMainIcon = TD_WARNING_ICON;

			saveStr = gettext(L"Save information");
			buttons[0].pszButtonText = saveStr.c_str();
		}

		OverloadCrashData(&taskDialogConfig);

		// don't upload the 'launched directly' error
		if (taskDialogConfig.pszContent && wcsstr(taskDialogConfig.pszContent, L"This application should be launched directly from the shell or a web browser."))
		{
			shouldUpload = false;
		}

		trace("Process crash captured. Crash dialog content:\n%s\n%s\n", ToNarrow(taskDialogConfig.pszMainInstruction), ToNarrow(taskDialogConfig.pszContent));

		g_dumpPath = *filePath;

		auto thread = std::thread([=]()
		{
			// Should not show taskdialog when in fxdk mode
			if (shouldTerminate && !launch::IsSDKGuest())
			{
				TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);
			}
		});

		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		bool uploadCrashes = true;
		bool bigMemoryDump = false;

		if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			bigMemoryDump = (GetPrivateProfileInt(L"Game", L"EnableFullMemoryDump", 0, fpath.c_str()) != 0);
			uploadCrashes = (GetPrivateProfileInt(L"Game", L"DisableCrashUpload", 0, fpath.c_str()) != 1);
		}

		if (bigMemoryDump && shouldTerminate)
		{
			STARTUPINFOW si = { 0 };
			si.cb = sizeof(si);

			PROCESS_INFORMATION pi;

			std::wstring dumpPath = *filePath;
			dumpPath.resize(dumpPath.size() - 4);  // strip .dmp
			dumpPath.append(TEXT("-full.dmp"));

			CreateProcessW(nullptr, const_cast<wchar_t*>(va(L"explorer /select,\"%s\"", dumpPath)), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}

		int timeout = 20000;

		// upload the actual minidump file as well
#if defined(GTA_FIVE)
		if (uploadCrashes && shouldUpload && HTTPUpload::SendMultipartPostRequest(L"https://crash-ingress.fivem.net/post", parameters, files, &timeout, &responseBody, &responseCode))
		{
			trace("Crash report service returned %s\n", ToNarrow(responseBody));
			crashId = responseBody;
		}
		else
		{
			if (shouldUpload)
			{
				crashIdError = fmt::sprintf(L"Error uploading: HTTP %d%s", responseCode, !responseBody.empty() ? L" (" + responseBody + L")" : L"");
			}
			else
			{
				crashIdError = L"Crash reporting is disabled for this crash type.";
			}

			crashId = L"";
		}
#else
		crashIdError = L"Crash reporting is disabled for this title.";
		crashId = L"";
#endif

		if (thread.joinable())
		{
			thread.join();
		}

		if (saveThread.joinable())
		{
			saveThread.join();
		}

		SetEvent(crashReport);

		_wunlink(MakeRelativeCitPath(L"data\\cache\\extra_dump_info.bin").c_str());
		_wunlink(MakeRelativeCitPath(L"data\\cache\\extra_dump_info2.bin").c_str());
	};

	CrashGenerationServer::OnClientExitedCallback exitCallback = [] (void*, const ClientInfo* info)
	{
	};

	CrashGenerationServer::OnClientUploadRequestCallback uploadCallback = [] (void*, DWORD)
	{

	};

	std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");

	std::wstring pipeName = L"\\\\.\\pipe\\CitizenFX_Dump";

	CrashGenerationServer server(pipeName, nullptr, connectCallback, nullptr, dumpCallback, nullptr, exitCallback, nullptr, uploadCallback, nullptr, true, &crashDirectory);
	if (server.Start())
	{
		SetEvent(inheritedHandleBit);

		OnStartSession();
	}

	WaitForSingleObject(parentProcess, INFINITE);
	WaitForSingleObject(crashReport, 15000);

	// at this point we can safely perform some cleanup tasks, no matter whether the game exited cleanly or crashed

	// revert NVSP disablement
#ifdef LAUNCHER_PERSONALITY_MAIN
	NVSP_ShutdownSafely();
	SteamInput_Reset();

	g_session["status"] = "exited";
	UpdateSession(g_session);

	_wunlink(MakeRelativeCitPath(L"data\\cache\\error-pickup").c_str());
	_wunlink(MakeRelativeCitPath(L"data\\cache\\session").c_str());
#endif

	// delete steam_appid.txt on last process exit to curb paranoia about MTL mod checks
	_wunlink(MakeRelativeGamePath(L"steam_appid.txt").c_str());
}
#endif

namespace google_breakpad
{
class AutoExceptionHandler
{
public:
	static LONG HandleException(EXCEPTION_POINTERS* exinfo)
	{
		return ExceptionHandler::HandleException(exinfo);
	}
};
}

void InitializeMiniDumpOverride()
{
	auto CoreSetExceptionOverride = (void (*)(LONG(*)(EXCEPTION_POINTERS*)))GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetExceptionOverride");

	if (CoreSetExceptionOverride)
	{
		CoreSetExceptionOverride(AutoExceptionHandler::HandleException);
	}
}

static ExceptionHandler* g_exceptionHandler;

extern "C" IMAGE_DOS_HEADER __ImageBase;
extern "C" BOOL WINAPI _CRT_INIT(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved);
#ifdef _M_AMD64
extern "C" void WINAPI __security_init_cookie();
#endif

static bool initialized = false;

extern "C" DLL_EXPORT void EarlyInitializeExceptionHandler()
{
	if (initialized)
	{
		return;
	}

#ifdef _M_AMD64
	__security_init_cookie();
#endif
	_CRT_INIT((HINSTANCE)&__ImageBase, DLL_PROCESS_ATTACH, nullptr);
	_CRT_INIT((HINSTANCE)&__ImageBase, DLL_THREAD_ATTACH, nullptr);
}

extern "C" DLL_EXPORT bool TerminateForException(PEXCEPTION_POINTERS exception)
{
	if (g_exceptionHandler)
	{
		return g_exceptionHandler->WriteMinidumpForException(exception);
	}

	return false;
}

extern "C" DLL_EXPORT bool InitializeExceptionHandler()
{
	if (initialized)
	{
		return false;
	}

	initialized = true;

	AllocateExceptionBuffer();

	// don't initialize when under a debugger, as debugger filtering is only done when execution gets to UnhandledExceptionFilter in basedll
	bool isDebugged = false;

	if (IsDebuggerPresent())
	{
		isDebugged = true;
	}

	std::wstring crashDirectory = MakeRelativeCitPath(L"crashes");
	CreateDirectory(crashDirectory.c_str(), nullptr);

	wchar_t* dumpServerBit = wcsstr(GetCommandLine(), L"-dumpserver");

	if (dumpServerBit)
	{
		wchar_t* parentPidBit = wcsstr(GetCommandLine(), L"-parentpid:");

#if defined(LAUNCHER_PERSONALITY_MAIN)
		InitializeDumpServer(wcstol(&dumpServerBit[12], nullptr, 10), wcstol(&parentPidBit[11], nullptr, 10));
#endif

		return true;
	}

	bool bigMemoryDump = false;

	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		bigMemoryDump = (GetPrivateProfileInt(L"Game", L"EnableFullMemoryDump", 0, fpath.c_str()) != 0);
	}

	auto mdType = (MiniDumpWithProcessThreadData | MiniDumpWithUnloadedModules | MiniDumpWithThreadInfo);

	if (bigMemoryDump)
	{
		mdType |= MiniDumpWithFullMemory;
	}

	CrashGenerationClient* client = new CrashGenerationClient(L"\\\\.\\pipe\\CitizenFX_Dump", (MINIDUMP_TYPE)mdType, new CustomClientInfo());

	if (!client->Register())
	{
		auto applicationName = MakeCfxSubProcess(L"DumpServer");

		// prepare initial structures
		STARTUPINFO startupInfo = { 0 };
		startupInfo.cb = sizeof(STARTUPINFO);

		PROCESS_INFORMATION processInfo = { 0 };

		// create an init handle
		SECURITY_ATTRIBUTES securityAttributes = { 0 };
		securityAttributes.bInheritHandle = TRUE;

		HANDLE initEvent = CreateEvent(&securityAttributes, TRUE, FALSE, nullptr);

		static HostSharedData<CfxState> hostData("CfxInitState");

		// create the command line including argument
		wchar_t commandLine[MAX_PATH * 8];
		if (_snwprintf(commandLine, _countof(commandLine), L"\"%s\" -dumpserver:%i -parentpid:%i", applicationName, (int)initEvent, hostData->GetInitialPid()) >= _countof(commandLine))
		{
			return false;
		}

		BOOL result = CreateProcess(applicationName, commandLine, nullptr, nullptr, TRUE, CREATE_BREAKAWAY_FROM_JOB, nullptr, nullptr, &startupInfo, &processInfo);

		if (result)
		{
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);
		}

		DWORD waitResult = WaitForSingleObject(initEvent, 
#ifdef _DEBUG
			1500
#else
			7500
#endif
		);

		if (!isDebugged)
		{
			if (!client->Register())
			{
				trace("Could not register with breakpad server.\n");
			}
		}
	}

	if (isDebugged)
	{
		return false;
	}

	g_exceptionHandler = new ExceptionHandler(
							L"",
							[](void* context, EXCEPTION_POINTERS* exinfo,
								MDRawAssertionInfo* assertion)
							{
								return true;
							},
							[] (const wchar_t* dump_path, const wchar_t* minidump_id, void* context, EXCEPTION_POINTERS* exinfo, MDRawAssertionInfo* assertion, bool succeeded)
							{
								return succeeded;
							},
							nullptr,
							ExceptionHandler::HANDLER_ALL,
							client
						);

	g_exceptionHandler->set_handle_debug_exceptions(true);

	// disable Windows' SetUnhandledExceptionFilter
	DWORD oldProtect;

	LPVOID unhandledFilters[] = { 
		GetProcAddress(GetModuleHandle(L"kernelbase.dll"), "SetUnhandledExceptionFilter"),
		GetProcAddress(GetModuleHandle(L"kernel32.dll"), "SetUnhandledExceptionFilter"),
	};

	for (auto unhandledFilter : unhandledFilters)
	{
		if (unhandledFilter)
		{
			VirtualProtect(unhandledFilter, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

#ifdef _M_AMD64
			*(uint8_t*)unhandledFilter = 0xC3;
#else
			*(uint32_t*)unhandledFilter = 0x900004C2;
#endif
		}
	}

	return false;
}
