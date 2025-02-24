/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#pragma comment(lib, "crypt32.lib")

#include <wincrypt.h>
#include <shlobj.h>

#include <array>
#include <mutex>

#include <Error.h>

#include <Hooking.Patterns.h>
#include <psapi.h>

#include <json.hpp>
#include <cpr/cpr.h>

#include <botan/base64.h>
#include <boost/property_tree/xml_parser.hpp>
#include <sstream>

#include <CL2LaunchMode.h>

#include <CfxLocale.h>
#include <UUIState.h>
#include <HostSharedData.h>

#include "ROSErrors.h"
#include <boost/algorithm/string/replace.hpp>

#include "CfxReleaseInfo.h"

constexpr const auto kROSBlobEmailOffset = 8; // 0x8
constexpr const auto kROSBlobTicketOffset = 2800; // 0xAF0
constexpr const auto kROSBlobSessionTicketOffset = 3312; // 0xCF0
constexpr const auto kROSBlobAccountIdOffset = 3824; // 0xEF0
constexpr const auto kROSBlobUsernameOffset = 3751; // 0xEA7
constexpr const auto kROSBlobSessionKeyOffset = 4480; // 0x1180

template<typename... Ts>
static auto PostAutoLogin(Ts&&... args)
{
	return cpr::Post(
		cpr::Header{
			{ "Content-Type", "application/json; charset=utf-8" },
			{ "Accept", "application/json" },
			{ "X-Requested-With", "XMLHttpRequest" } },
		cpr::UserAgent{ fmt::sprintf("CitizenFX/1 (rel. %d)", cfx::GetPlatformRelease()) },
		std::forward<Ts>(args)...
	);
}

static bool TryFindError(const std::string& errorString, std::string* outMessage)
{
	if (auto it = g_rosErrors.find("Errors_" + errorString); it != g_rosErrors.end())
	{
		auto msg = it->second;
		boost::algorithm::replace_all(msg, "<a>", "");
		boost::algorithm::replace_all(msg, "</a>", "");

		*outMessage = msg;

		return true;
	}

	return false;
}

static std::string MapMessage(const nlohmann::json& j)
{
	std::string message = "Unknown error.";

	if (auto it = j.find("Message"); it != j.end() && it.value().is_string())
	{
		message = it.value().get<std::string>();
	}
	else if (auto it = j.find("Error"); it != j.end() && it.value().is_object())
	{
		if (auto errorMessage = it.value().find("ErrorMessage"); errorMessage != it.value().end() && errorMessage.value().is_string())
		{
			std::string errorString = errorMessage.value();
			std::string errorStringGeneral = errorString.substr(0, errorString.find_first_of('.'));

			if (!TryFindError(errorString, &message))
			{
				TryFindError(errorStringGeneral, &message);
			}
		}
	}

	return message;
}

// this structure is passed via HostSharedData so should have a linear layout
struct ExternalROSBlob
{
	uint8_t data[16384];
	uint8_t steamData[64 * 1024];
	size_t steamSize;

	// epicData is a copy of GetCommandLine result, so can't exceed the maximum length of a command line
	// (https://devblogs.microsoft.com/oldnewthing/20031210-00/?p=41553, https://archive.today/nab64)
	wchar_t epicData[UNICODE_STRING_MAX_CHARS];
	size_t epicSize;

	uint32_t steamAppId;
	bool valid;
	bool tried;
	bool triedEpic;
	bool inUI = false;

	ExternalROSBlob()
	{
		valid = false;
		tried = false;
		triedEpic = false;
		steamAppId = 0;
		memset(data, 0, sizeof(data));

		memset(steamData, 0, sizeof(steamData));
		steamSize = 0;

		memset(epicData, 0, sizeof(epicData));
		epicSize = 0;
	}
};

std::string GetAuthSessionTicket(uint32_t appID);

std::string GetExternalSteamTicket()
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");

	if (blob->steamSize)
	{
		return GetAuthSessionTicket(271590);
	}

	return "";
}

std::string g_rosEmail;

std::string GetROSEmail()
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");

	if (!blob->valid)
	{
		return g_rosEmail;
	}

	auto accountBlob = blob->data;
	return (const char*)&accountBlob[kROSBlobEmailOffset];
}

#if defined(IS_RDR3) || defined(GTA_FIVE) || defined(GTA_NY)
static uint8_t* accountBlob;

static std::vector<DWORD> GetMTLPids()
{
	static DWORD pids[16384];

	DWORD len;
	if (!EnumProcesses(pids, sizeof(pids), &len))
	{
		return {};
	}

	std::vector<DWORD> rv;

	auto numProcs = len / sizeof(DWORD);

	for (int i = 0; i < numProcs; i++)
	{
		auto hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pids[i]);

		if (hProcess)
		{
			wchar_t path[MAX_PATH];
			if (GetProcessImageFileNameW(hProcess, path, std::size(path)))
			{
				auto lastEnd = wcsrchr(path, L'\\');

				if (lastEnd)
				{
					if (!_wcsicmp(lastEnd, L"\\launcher.exe"))
					{
						// TODO: NT->DOS path name
						/*lastEnd[0] = '\0';
						wcscat(path, L"\\offline.pak");

						if (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES)
						{*/
							rv.push_back(pids[i]);
						//}
					}
				}
			}

			CloseHandle(hProcess);
		}
	}

	return rv;
}

static bool InitAccountRemote();

bool GetMTLSessionInfo(std::string& ticket, std::string& sessionTicket, std::array<uint8_t, 16>& sessionKey, uint64_t& accountId)
{
	if (!accountBlob)
	{
#if defined(GTA_FIVE) || defined(GTA_NY)
		if (!InitAccountRemote())
#endif
		{

			return false;
		}

		if (!accountBlob)
		{
			return false;
		}
	}

	ticket = std::string((const char*)&accountBlob[kROSBlobTicketOffset]);
	sessionTicket = std::string((const char*)&accountBlob[kROSBlobSessionTicketOffset]);
	memcpy(sessionKey.data(), &accountBlob[kROSBlobSessionKeyOffset], sessionKey.size());
	accountId = *(uint64_t*)&accountBlob[kROSBlobAccountIdOffset];

	return true;
}

void RunLauncher(const wchar_t* toolName, bool instantWait);

static bool InitAccountSteam()
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");

	if (!blob->tried)
	{
#ifdef IS_RDR3
		if (!getenv("CitizenFX_ToolMode"))
#endif
		{
			static HostSharedData<UpdaterUIState> uuiState("CfxUUIState");

			uuiState->SetText(1, gettext("Signing in with Steam"));
			uuiState->SetProgress(100.0);
			uuiState->Open();

			RunLauncher(L"ros:steam", true);			

			uuiState->Close();
		}
	}

	if (blob->valid)
	{
		accountBlob = blob->data;
		trace("External ROS says it's signed in: %s\n", (const char*)&accountBlob[kROSBlobEmailOffset]);
	}

	return blob->valid;
}

static bool InitAccountEOS()
{
	if (GetFileAttributesW(MakeRelativeGamePath(L"EOSSDK-Win64-Shipping.dll").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");

	if (!blob->triedEpic)
	{
#ifdef IS_RDR3
		if (!getenv("CitizenFX_ToolMode"))
#endif
		{
			static HostSharedData<UpdaterUIState> uuiState("CfxUUIState");

			uuiState->SetText(1, gettext("Signing in with Epic"));
			uuiState->SetProgress(100.0);
			uuiState->Open();

			RunLauncher(L"ros:epic", true);

			uuiState->Close();
		}
	}

	if (blob->valid)
	{
		accountBlob = blob->data;
		trace("Epic ROS says it's signed in: %s\n", (const char*)&accountBlob[kROSBlobEmailOffset]);
	}

	return blob->valid;
}

HANDLE OpenProcessByName(const std::wstring& name, DWORD desiredAccess)
{
	std::vector<DWORD> processes(16384);
	DWORD cbNeeded = 0;
	EnumProcesses(&processes[0], processes.size() * sizeof(DWORD), &cbNeeded);

	auto numProcs = cbNeeded / sizeof(DWORD);

	for (size_t i = 0; i < numProcs; i++)
	{
		HANDLE handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processes[i]);

		if (handle)
		{
			wchar_t filename[2048];
			if (GetProcessImageFileNameW(handle, filename, std::size(filename)))
			{
				if (wcsstr(filename, name.c_str()))
				{
					CloseHandle(handle);

					return OpenProcess(desiredAccess, FALSE, processes[i]);
				}
			}

			CloseHandle(handle);
		}
	}

	return INVALID_HANDLE_VALUE;
}

#include <shellapi.h>
#include <winternl.h>

#pragma comment(lib, "ntdll.lib")

extern bool RunSteamAuthUi(const nlohmann::json& json);
extern bool RunEpicAuthUi(const nlohmann::json& json);
std::string g_tpaId;
std::string g_tpaToken;
std::string g_rosData2;

bool LoadOwnershipTicket();

static nlohmann::json LoadStoredTpaData()
{
	nlohmann::json j;

	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
	{
		// create the directory if not existent
		std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
		CreateDirectory(cfxPath.c_str(), nullptr);

		// open and read the profile file
		std::wstring profilePath = cfxPath + L"\\ros_auth.json";

		if (!LoadOwnershipTicket())
		{
			std::error_code err;
			std::filesystem::remove(profilePath, err);

			return j;
		}

		if (FILE* profileFile = _wfopen(profilePath.c_str(), L"rb"))
		{
			std::vector<uint8_t> profileFileData;
			int pos;

			// get the file length
			fseek(profileFile, 0, SEEK_END);
			pos = ftell(profileFile);
			fseek(profileFile, 0, SEEK_SET);

			// resize the buffer
			profileFileData.resize(pos);

			// read the file and close it
			fread(&profileFileData[0], 1, pos, profileFile);

			fclose(profileFile);

			// decrypt the stored data - setup blob
			DATA_BLOB cryptBlob;
			cryptBlob.pbData = &profileFileData[0];
			cryptBlob.cbData = profileFileData.size();

			DATA_BLOB outBlob;

			// call DPAPI
			if (CryptUnprotectData(&cryptBlob, nullptr, nullptr, nullptr, nullptr, 0, &outBlob))
			{
				// parse the profile list
				try
				{
					j = nlohmann::json::parse(std::string(reinterpret_cast<char*>(outBlob.pbData), outBlob.cbData));
				}
				catch (std::exception& e)
				{
				}

				// free the out data
				LocalFree(outBlob.pbData);
			}
		}

		CoTaskMemFree(appDataPath);
	}

	return j;
}

static void SaveTpaData(const nlohmann::json& list)
{
	auto savedList = list.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);

	// encrypt the actual string
	DATA_BLOB cryptBlob;
	cryptBlob.pbData = reinterpret_cast<uint8_t*>(const_cast<char*>(savedList.c_str()));
	cryptBlob.cbData = savedList.size();

	DATA_BLOB outBlob;

	if (CryptProtectData(&cryptBlob, nullptr, nullptr, nullptr, nullptr, 0, &outBlob))
	{
		PWSTR appDataPath;
		if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
		{
			// create the directory if not existent
			std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
			CreateDirectory(cfxPath.c_str(), nullptr);

			// open and read the profile file
			std::wstring profilePath = cfxPath + L"\\ros_auth.json";

			if (FILE* profileFile = _wfopen(profilePath.c_str(), L"wb"))
			{
				fwrite(outBlob.pbData, 1, outBlob.cbData, profileFile);
				fclose(profileFile);
			}

			CoTaskMemFree(appDataPath);
		}

		LocalFree(outBlob.pbData);
	}
}

static nlohmann::json GetTpaTokens()
{
	auto doc = LoadStoredTpaData();

	if (doc.is_object() && doc["tpaTokens"].is_object())
	{
		return doc["tpaTokens"];
	}

	return nlohmann::json::object();
}

static void AddTpaToken(const std::string& rockstarId, const std::string& token)
{
	auto doc = LoadStoredTpaData();

	if (!doc.is_object())
	{
		doc = nlohmann::json::object();
	}

	if (!doc["tpaTokens"].is_object())
	{
		doc["tpaTokens"] = nlohmann::json::object();
	}

	doc["tpaTokens"][rockstarId] = token;

	SaveTpaData(doc);
}

#include <eos-sdk/eos_sdk.h>
#include <eos-sdk/eos_auth.h>

void ValidateEpic(int parentPid)
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");
	blob->triedEpic = true;

	// find a process named EpicGamesLauncher.exe
	auto egl = OpenProcessByName(L"EpicGamesLauncher.exe", SYNCHRONIZE);

	if (egl == INVALID_HANDLE_VALUE)
	{
		return;
	}

	if (!egl)
	{
		MessageBoxW(NULL, L"Could not read data from the Epic Games Launcher.", L"RedM", MB_OK | MB_ICONSTOP);
		return;
	}

	CloseHandle(egl);

	// try finding MTL (or the launcher stub, for Five)
	auto launchedProcName =
#ifndef GTA_FIVE
	L"Launcher\\Launcher.exe"
#else
	L"PlayGTAV.exe"
#endif
	;

	auto mtl = OpenProcessByName(launchedProcName, SYNCHRONIZE);

	std::wstring epicName = L"Heather";

#ifdef GTA_FIVE
	epicName = L"0584d2013f0149a791e7b9bad0eec102%3A6e563a2c0f5f46e3b4e88b5f4ed50cca%3A9d2d0eb64d5c44529cece33fe2a46482";
#endif

	if (mtl != INVALID_HANDLE_VALUE)
	{
#ifdef IS_RDR3
		MessageBoxW(NULL, L"If you're trying to launch an Epic Games version of RDR2, please make sure the Rockstar Games Launcher is closed. If not, remove EOSSDK-Win64-Shipping.dll from your RDR2 folder.", L"RedM", MB_OK | MB_ICONSTOP);
#endif
		return;
	}

	std::wstring silentVar = L"";
#ifdef IS_RDR3
	silentVar = L"&silent=true";
#endif

	// try to launch RDR-MTL from EGL
	ShellExecuteW(NULL, L"open", va(L"com.epicgames.launcher://apps/%s?action=launch%s", epicName, silentVar), NULL, NULL, SW_SHOWNORMAL);

	// wait for MTL to open
	int iterations = 0;

	mtl = INVALID_HANDLE_VALUE;

	while (mtl == INVALID_HANDLE_VALUE)
	{
		if (iterations > (60000 / 500))
		{
#ifdef IS_RDR3
			MessageBoxW(NULL, L"If you're trying to launch an Epic Games version of RDR2, please make sure that you can launch it using the Epic Games Launcher first.", L"RedM", MB_OK | MB_ICONSTOP);
#endif
			return;
		}

		Sleep(500);

		mtl = OpenProcessByName(launchedProcName, PROCESS_VM_READ | PROCESS_QUERY_INFORMATION | PROCESS_TERMINATE | PROCESS_SUSPEND_RESUME);
	}

	if (!mtl)
	{
#ifdef IS_RDR3
		FatalError("Epic Games Launcher launched the Rockstar Games Launcher in a way we couldn't access. Aborting! If you are running it as administrator, please launch it as regular user.");
#endif

		return;
	}

	// halt!
	DebugActiveProcess(GetProcessId(mtl));

	auto bye = [mtl]()
	{
		DebugActiveProcessStop(GetProcessId(mtl));
		TerminateProcess(mtl, 0);

#ifdef GTA_FIVE
		auto gameStub = OpenProcessByName(L"PlayGTAV.exe", PROCESS_TERMINATE);

		if (gameStub)
		{
			TerminateProcess(gameStub, 0);
		}
#endif
	};

	// get command line
	PROCESS_BASIC_INFORMATION pbi;
	NtQueryInformationProcess(mtl, ProcessBasicInformation, &pbi, sizeof(pbi), NULL);

	PVOID pPEB = pbi.PebBaseAddress;
	PVOID rtlUserProcParamsAddress;
	UNICODE_STRING commandLine;

	ReadProcessMemory(mtl, (PCHAR)pPEB + offsetof(PEB, ProcessParameters), &rtlUserProcParamsAddress, sizeof(PVOID), NULL);
	ReadProcessMemory(mtl, (PCHAR)rtlUserProcParamsAddress + offsetof(RTL_USER_PROCESS_PARAMETERS, CommandLine), &commandLine, sizeof(commandLine), NULL);

	std::vector<wchar_t> buffer((commandLine.Length / 2) + 1);
	ReadProcessMemory(mtl, commandLine.Buffer, &buffer[0], commandLine.Length, NULL);

	std::wstring commandLineString = &buffer[0];

	int argc = 0;
	auto argv = CommandLineToArgvW(commandLineString.c_str(), &argc);

	std::wstring epicPassword;

	for (size_t i = 0; i < argc; i++)
	{
		std::wstring_view arg = argv[i];

		if (arg.find(L"-AUTH_PASSWORD=") == 0)
		{
			epicPassword = arg.substr(15);
		}
	}

	LocalFree(argv);

	if (epicPassword.empty())
	{
		FatalError("Didn't get an Epic exchange token.");
	}

	auto eosDll = LoadLibraryW(MakeRelativeGamePath(L"EOSSDK-Win64-Shipping.dll").c_str());

	if (!eosDll)
	{
#ifdef IS_RDR3
		FatalError("No Epic DLL.");
#else
		return;
#endif
	}

	EOS_InitializeOptions eio = { 0 };
	eio.ApiVersion = 2; //EOS_INITIALIZE_API_LATEST;
#ifdef IS_RDR3
	eio.ProductName = "RDR2";
#else
	eio.ProductName = "GTAV";
#endif
	eio.ProductVersion = "1.0";

	auto _EOS_Initialize = (decltype(&EOS_Initialize))GetProcAddress(eosDll, "EOS_Initialize");
	_EOS_Initialize(&eio);

	EOS_Platform_Options epo = { 0 };
	epo.ApiVersion = 5; //EOS_PLATFORM_OPTIONS_API_LATEST;
#ifdef IS_RDR3
	epo.ProductId = "1893447b7ea5459b96191a7458075768";
	epo.SandboxId = "b30b6d1b4dfd4dcc93b5490be5e094e5";
	epo.ClientCredentials.ClientId = "6b3b277c50a74855ab9beff2d62428aa";
	epo.ClientCredentials.ClientSecret = "4h8MBChGbrV2woGgr2fdaYn2iWtHdf9F"; // omg this is SO secret, wow
	epo.DeploymentId = "a59e364fb7f04ea4b0ca52e91281b9dc";
#else
	epo.ProductId = "68d2cc08f9a94b8fb51af4f5cfa6d41b";
	epo.SandboxId = "0584d2013f0149a791e7b9bad0eec102";
	epo.ClientCredentials.ClientId = "c19208b6eec5423d951186aee0ad7b9c";
	epo.ClientCredentials.ClientSecret = "YaEL5raCmwNB4TuZf5AU"; // omg this is SO secret, wow
	epo.DeploymentId = "f8cb4ad3fab34f4ca902480ae7875a82";
#endif
	epo.Flags = EOS_PF_DISABLE_OVERLAY;

	auto _EOS_Platform_Create = (decltype(&EOS_Platform_Create))GetProcAddress(eosDll, "EOS_Platform_Create");
	auto epicPlatform = _EOS_Platform_Create(&epo);
	assert(epicPlatform);

	auto _EOS_Platform_GetAuthInterface = (decltype(&EOS_Platform_GetAuthInterface))GetProcAddress(eosDll, "EOS_Platform_GetAuthInterface");
	static auto epicAuth = _EOS_Platform_GetAuthInterface(epicPlatform);

	auto nepicPassword = ToNarrow(epicPassword);

	EOS_Auth_Credentials ecs = { 0 };
	ecs.ApiVersion = 1; //EOS_AUTH_CREDENTIALS_API_LATEST;
	ecs.Type = EOS_ELoginCredentialType::EOS_LCT_ExchangeCode;
	ecs.Token = nepicPassword.c_str();

	EOS_Auth_LoginOptions eao = { 0 };
	eao.ApiVersion = 1;//EOS_AUTH_LOGIN_API_LATEST;
	//eao.ScopeFlags = 0;
	eao.Credentials = &ecs;

	static HANDLE hWait = CreateEvent(NULL, FALSE, FALSE, NULL);
	static EOS_Auth_Token* eosToken;

	static auto _EOS_Auth_CopyUserAuthToken = (decltype(&EOS_Auth_CopyUserAuthToken))GetProcAddress(eosDll, "EOS_Auth_CopyUserAuthToken");
	static auto _EOS_EpicAccountId_ToString = (decltype(&EOS_EpicAccountId_ToString))GetProcAddress(eosDll, "EOS_EpicAccountId_ToString");

	std::string s;
	nlohmann::json j;
	cpr::Response r;

	auto _EOS_Auth_Login = (decltype(&EOS_Auth_Login))GetProcAddress(eosDll, "EOS_Auth_Login");
	static std::string accountId;
	_EOS_Auth_Login(epicAuth, &eao, &hWait, [](const EOS_Auth_LoginCallbackInfo* info)
	{
		if (info->ResultCode != EOS_EResult::EOS_Success)
		{
			FatalError("EOS_Auth_Login failed: %d", (int)info->ResultCode);
		}

		EOS_Auth_CopyUserAuthTokenOptions cao = { 0 };
		cao.ApiVersion = 1;

		_EOS_Auth_CopyUserAuthToken(epicAuth, &cao, info->LocalUserId, &eosToken);

		SetEvent(hWait);

		char accountIdBuf[128];
		int32_t accountIdLength = std::size(accountIdBuf);

		_EOS_EpicAccountId_ToString(info->LocalUserId, accountIdBuf, &accountIdLength);

		accountId = accountIdBuf;
	});

	auto _EOS_Platform_Tick = (decltype(&EOS_Platform_Tick))GetProcAddress(eosDll, "EOS_Platform_Tick");

	while (WaitForSingleObject(hWait, 50) == WAIT_TIMEOUT)
	{
		_EOS_Platform_Tick(epicPlatform);
	}

	s = eosToken->AccessToken;

	blob->inUI = true;

	if (!RunEpicAuthUi(nlohmann::json::object({ { "EpicAccountId", accountId },
		{ "EpicPlayerName", "uhh" },
		{ "epicAccessToken", s }, { "TpaTokens", GetTpaTokens() } })))
	{
		blob->inUI = false;
		bye();

#ifdef IS_RDR3
		FatalError("Error during Epic ROS signin\n");
#endif

		return;
	}

	blob->inUI = false;

	AddTpaToken(g_tpaId, g_tpaToken);
	
	j = nlohmann::json::parse(g_rosData2);

	auto sessionKey = Botan::base64_decode(j.value("SessionKey", ""));

	std::istringstream stream(j.value("Xml", ""));

	boost::property_tree::ptree tree;
	boost::property_tree::read_xml(stream, tree);

	auto tick = j.value("Ticket", "");

	*(uint64_t*)&blob->data[kROSBlobAccountIdOffset] = j.value("RockstarId", uint64_t(0));
	strcpy((char*)&blob->data[kROSBlobTicketOffset], tick.c_str());
	strcpy((char*)&blob->data[kROSBlobSessionTicketOffset], j.value("SessionTicket", "").c_str());
	memcpy(&blob->data[kROSBlobSessionKeyOffset], sessionKey.data(), sessionKey.size());
	strcpy((char*)&blob->data[kROSBlobEmailOffset], j.value("Email", "").c_str());
	strcpy((char*)&blob->data[kROSBlobUsernameOffset], j.value("Nickname", "").c_str());

	j = nlohmann::json::object({
#ifdef IS_RDR3
		{ "title", "rdr2" },
#else
		{ "title", "gta5" },
#endif
		{ "env", "prod" },
		{ "epicPlayerJwt", s },
		{ "playerTicket", tick },
		{ "version", 11 },
	});

	r = PostAutoLogin(
	cpr::Url{ "https://rgl.rockstargames.com/api/launcher/bindepicaccount" },
	cpr::Header{
	{
		{ "Authorization", fmt::sprintf("SCAUTH val=\"%s\"", tick) },
	} },
	cpr::Body{
	j.dump() });

	if (r.error)
	{
		bye();

#ifdef IS_RDR3
		FatalError("Error during binding of ROS to Epic: %s", r.error.message);
#endif
		return;
	}

	StringCbCopyW(blob->epicData, sizeof(blob->epicData), commandLineString.c_str());
	blob->epicSize = commandLineString.size();
	blob->valid = true;

	// resume until we can exit
	HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"CitizenFX_GTA5_ClearedForLaunch_Epic");

	if (hEvent != INVALID_HANDLE_VALUE)
	{
		SetEvent(hEvent);
		CloseHandle(hEvent);
	}

	HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, parentPid);
	WaitForSingleObject(hProcess, 30000);

	// terminate
	bye();
}

void ValidateSteam(int parentPid)
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");
	blob->tried = true;

	uint32_t appId =
#ifdef GTA_FIVE
	271590
#elif defined(IS_RDR3)
	1174180
#elif defined(GTA_NY)
	12210
#else
	0
#endif
	;

	auto appName =
#ifdef GTA_FIVE
	"gta5"
#elif defined(IS_RDR3)
	"rdr2"
#elif defined(GTA_NY)
	"gta4"
#else
	""
#endif
	;

	nlohmann::json j;
	cpr::Response r;
	std::string s;

	s = GetAuthSessionTicket(appId);

	if (s.empty())
	{
#if defined(IS_RDR3)
		appId = 1404210; // RDO
		appName = "rdr2_rdo";
		s = GetAuthSessionTicket(appId);

		if (s.empty())
#endif
		{
			return;
		}
	}

	blob->inUI = true;

	if (!RunSteamAuthUi(nlohmann::json::object({ { "SteamAppId", int32_t(appId) },
		{ "SteamAuthTicket", s }, { "TpaTokens", GetTpaTokens() } })))
	{
#ifdef IS_RDR3
		FatalError("Error during Steam ROS signin\n");
#endif

		blob->inUI = false;
		return;
	}

	blob->inUI = false;

	AddTpaToken(g_tpaId, g_tpaToken);

	j = nlohmann::json::parse(g_rosData2);

	auto sessionKey = Botan::base64_decode(j.value("SessionKey", ""));
	auto tick = j.value("Ticket", "");

	*(uint64_t*)&blob->data[kROSBlobAccountIdOffset] = j.value("RockstarId", uint64_t(0));
	strcpy((char*)&blob->data[kROSBlobTicketOffset], tick.c_str());
	strcpy((char*)&blob->data[kROSBlobSessionTicketOffset], j.value("SessionTicket", "").c_str());
	memcpy(&blob->data[kROSBlobSessionKeyOffset], sessionKey.data(), sessionKey.size());
	strcpy((char*)&blob->data[kROSBlobEmailOffset], j.value("Email", "").c_str());
	strcpy((char*)&blob->data[kROSBlobUsernameOffset], j.value("Nickname", "").c_str());

	j = nlohmann::json::object({
		{ "title", appName },
		{ "env", "prod" },
		{ "steamAuthTicket", s },
		{ "steamAppId", appId },
		{ "playerTicket", tick },
		{ "version", 11 },
	});

	r = PostAutoLogin(
		cpr::Url{ "https://rgl.rockstargames.com/api/launcher/bindsteamaccount" },
		cpr::Header{
			{"Authorization", fmt::sprintf("SCAUTH val=\"%s\"", tick) },
		},
		cpr::Body{
			j.dump()
		});

	if (r.error)
	{
#ifdef IS_RDR3
		FatalError("Error during binding of ROS to Steam: %s", r.error.message);
#endif

		return;
	}

	j = nlohmann::json::parse(r.text);

	memcpy(blob->steamData, s.data(), s.size());
	blob->steamSize = s.size();
	blob->steamAppId = appId;

	blob->valid = true;
}

static bool InitAccountMTL()
{
#if defined(_M_AMD64)
	auto pids = GetMTLPids();

	if (pids.empty())
	{
#if defined(IS_RDR3)
		MessageBoxW(NULL, L"You have to run the Rockstar Games Launcher, Steam, or the Epic Games Launcher (depending on where you purchased the game) to be able to run RedM.", L"RedM", MB_OK | MB_ICONSTOP);
#endif

		return false;
	}

	HANDLE hProcess = NULL;
	HMODULE scModule = NULL;

	for (DWORD pid : pids)
	{
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

		if (!hProcess)
		{
			continue;
		}

		HMODULE hMods[1024];
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
		{
			for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
			{
				wchar_t szModName[MAX_PATH];

				// Get the full path to the module's file.

				if (GetModuleFileNameExW(hProcess, hMods[i], szModName,
					std::size(szModName)))
				{
					auto lastEnd = wcsrchr(szModName, L'\\');

					if (lastEnd)
					{
						if (_wcsicmp(lastEnd, L"\\socialclub.dll") == 0)
						{
							scModule = hMods[i];
							break;
						}
					}
				}
			}
		}

		if (scModule)
		{
			break;
		}

		CloseHandle(hProcess);
	}

	if (!hProcess)
	{
#if defined(IS_RDR3)
		MessageBoxW(NULL, L"Could not read data from the Rockstar Games Launcher. If you are running it as administrator, please launch it as regular user.", L"RedM", MB_OK | MB_ICONSTOP);
#endif

		return false;
	}
	
	if (!scModule)
	{
#if defined(IS_RDR3)
		FatalError("MTL didn't have SC SDK loaded.");
#endif

		return false;
	}

	SIZE_T nr;
	uint8_t buffer[0x1000];
	ReadProcessMemory(hProcess, scModule, buffer, sizeof(buffer), &nr);

	IMAGE_DOS_HEADER* mz = (IMAGE_DOS_HEADER*)buffer;
	assert(mz->e_magic == IMAGE_DOS_SIGNATURE);

	IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)&buffer[mz->e_lfanew];
	auto len = nt->OptionalHeader.SizeOfCode - 0x1000;

	std::vector<uint8_t> exeCode(len);
	ReadProcessMemory(hProcess, (char*)scModule + 0x1000, exeCode.data(), len, &nr);

	auto pl = hook::range_pattern((uintptr_t)exeCode.data(), (uintptr_t)exeCode.data() + exeCode.size(), "84 C0 75 19 FF C7 83 FF 01 7C D8").count_hint(1);

	if (pl.size() < 1)
	{
		return false;
	}

	auto p = pl.get(0).get<uint8_t>(-35);
	auto memOff = *(uint32_t*)p;

	// rebase to MODULE-RELATIVE offset
	auto origOff = p - exeCode.data() + (uint8_t*)scModule + 0x1000;
	auto bufOff = origOff + memOff + 4;

	accountBlob = new uint8_t[16384];
	ReadProcessMemory(hProcess, bufOff, accountBlob, 16384, &nr);

	if (!isalnum(accountBlob[8]))
	{
#if defined(IS_RDR3)
		FatalError("No account blob info. Make sure to sign in to the Rockstar Games Launcher.");
#endif

		return false;
	}

	trace("MTL says it's signed in: %s\n", (const char*)&accountBlob[kROSBlobEmailOffset]);

	return true;
#else
	// #TODOLIBERTY
	return false;
#endif
}

void PreInitGameSpec()
{
#if defined(IS_RDR3)
	static bool accountSetUp;

	if (accountSetUp)
	{
		return;
	}

	accountSetUp = true;

	if (wcsstr(GetCommandLineW(), L"ros:steam") != nullptr || wcsstr(GetCommandLineW(), L"ros:epic") != nullptr || wcsstr(GetCommandLineW(), L"--type=") != nullptr)
	{
		return;
	}

	if (!InitAccountSteam() && !InitAccountEOS() && !InitAccountMTL())
	{
		TerminateProcess(GetCurrentProcess(), 0);
	}
#endif
}

static bool InitAccountRemote()
{
	return InitAccountSteam() || InitAccountMTL() || InitAccountEOS();
}

#ifdef IS_RDR3
#include <ICoreGameInit.h>

static HookFunction hookFunction([]()
{
	Instance<ICoreGameInit>::Get()->SetData("rosUserName", (const char*)&accountBlob[kROSBlobUsernameOffset]);
	Instance<ICoreGameInit>::Get()->SetData("rosUserEmail", (const char*)&accountBlob[kROSBlobEmailOffset]);
});
#endif
#else
void ValidateEpic(int)
{
}

void ValidateSteam(int)
{
}

void PreInitGameSpec()
{
}
#endif

uint64_t ROSGetDummyAccountID()
{
#if defined(IS_RDR3)
	return *(uint64_t*)&accountBlob[kROSBlobAccountIdOffset];
#else
	static std::once_flag gotAccountId;
	static uint32_t accountId;

	std::call_once(gotAccountId, [] ()
	{
		PWSTR appdataPath = nullptr;
		SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

		CreateDirectory(va(L"%s\\CitizenFX", appdataPath), nullptr);

		FILE* f = _wfopen(va(L"%s\\CitizenFX\\ros_id%s.dat", appdataPath, IsCL2() ? L"CL2" : L""), L"rb");

		auto generateNewId = [&] ()
		{
			// generate a random id
			HCRYPTPROV provider;
			if (!CryptAcquireContext(&provider, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
			{
				FatalError("CryptAcquireContext failed (ros:five ID generation)");
			}

			// do call
			if (!CryptGenRandom(provider, sizeof(accountId), reinterpret_cast<BYTE*>(&accountId)))
			{
				FatalError("CryptGenRandom failed (ros:five ID generation)");
			}

			// release
			CryptReleaseContext(provider, 0);

			// remove top bit
			accountId &= 0x7FFFFFFF;

			// verify if ID isn't null
			if (accountId == 0)
			{
				FatalError("ros:five ID generation generated a null ID!");
			}

			// write id
			f = _wfopen(va(L"%s\\CitizenFX\\ros_id%s.dat", appdataPath, IsCL2() ? L"CL2" : L""), L"wb");

			if (!f)
			{
				FatalError("Could not open AppData\\CitizenFX\\ros_id.dat for writing!");
			}

			fwrite(&accountId, 1, sizeof(accountId), f);
		};

		if (!f)
		{
			generateNewId();
		}
		else
		{
			fread(&accountId, 1, sizeof(accountId), f);

			if (accountId == 0)
			{
				fclose(f);
				f = nullptr;

				_wunlink(va(L"%s\\CitizenFX\\ros_id%s.dat", appdataPath, IsCL2() ? L"CL2" : L""));

				generateNewId();
			}
		}

		if (f)
		{
			fclose(f);
		}

		CoTaskMemFree(appdataPath);
	});

	return accountId;
#endif
}

extern "C" DLL_EXPORT uint64_t GetAccountID()
{
	return ROSGetDummyAccountID();
}

#include <shlobj.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

bool LoadAccountData(std::string& str)
{
    // make path
    wchar_t* appdataPath;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

    CreateDirectory(va(L"%s\\CitizenFX", appdataPath), nullptr);

    // open?
    FILE* f = _wfopen(va(L"%s\\CitizenFX\\ros_auth.dat", appdataPath), L"rb");

    if (!f)
    {
        CoTaskMemFree(appdataPath);
        return false;
    }

    // seek
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    fseek(f, 0, SEEK_SET);

    // read
    std::vector<char> data(length + 1);
    fread(&data[0], 1, length, f);

    fclose(f);

    // hm
    str = &data[0];

    CoTaskMemFree(appdataPath);

    return true;
}

bool LoadAccountData(boost::property_tree::ptree& tree)
{
    std::string str;

    if (!LoadAccountData(str))
    {
        return false;
    }

    std::stringstream stream(str);

    boost::property_tree::read_xml(stream, tree);

    if (tree.get("Response.Status", 0) == 0)
    {
        return false;
    }
    else
    {
        std::string ticket = tree.get<std::string>("Response.Ticket");
        int posixTime = tree.get<int>("Response.PosixTime");
        int secsUntilExpiration = tree.get<int>("Response.SecsUntilExpiration");

        if (time(nullptr) < (posixTime + secsUntilExpiration))
        {
            return true;
        }
    }

    return false;
}

void SaveAccountData(const std::string& data)
{
    // make path
    wchar_t* appdataPath;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

    CreateDirectory(va(L"%s\\CitizenFX", appdataPath), nullptr);

    // open?
    FILE* f = _wfopen(va(L"%s\\CitizenFX\\ros_auth.dat", appdataPath), L"wb");

    CoTaskMemFree(appdataPath);

    if (!f)
    {
        return;
    }

    fwrite(data.c_str(), 1, data.size(), f);
    fclose(f);
}

#include <CrossBuildRuntime.h>

static HMODULE WINAPI LoadLibraryExWStub(
LPCWSTR lpLibFileName,
HANDLE hFile,
DWORD dwFlags)
{
	if (dwFlags == LOAD_WITH_ALTERED_SEARCH_PATH)
	{
		if (wcsstr(lpLibFileName, L"\\steam_api64.dll"))
		{
			lpLibFileName = va(L"%s", MakeRelativeCitPath(L"steam_api64.dll"));
		}
		else if (wcsstr(lpLibFileName, L"\\EOSSDK-Win64-Shipping.dll"))
		{
			lpLibFileName = va(L"%s", MakeRelativeGamePath(L"EOSSDK-Win64-Shipping.dll"));
		}
	}

	return LoadLibraryExW(lpLibFileName, hFile, dwFlags);
}

static LPSTR GetCommandLineAStub()
{
	static std::string cli = ([]
	{
		HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");
		return fmt::sprintf("%s -useEpic %s", GetCommandLineA(), ToNarrow(blob->epicData));
	})();

	return cli.data();
}

static HookFunction hookFunctionSteamBlob([]()
{
// move the game to Steam mode if needed
#if defined(IS_RDR3)
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");

	if (!getenv("CitizenFX_ToolMode") && xbr::IsGameBuildOrGreater<1355>())
	{
		if (blob->steamAppId)
		{
			// hook LoadLibraryExW to be able to find steam_api64.dll
			hook::iat("kernel32.dll", LoadLibraryExWStub, "LoadLibraryExW");

			auto location = hook::get_pattern<char>("75 21 48 89 59 48 48 89 59 50 48 89 59", -4);
			auto useSteam = hook::get_address<int*>(location);
			auto steamAppId = hook::get_address<char**>(location + 0x31) + 1;

			*useSteam = 1;
			*steamAppId = strdup(va("%d", blob->steamAppId));

			// Steam language takes precedence over the -rglLanguage CLI argument (if the Steam API returned a known language)
			// This is inconsistent so we'll treat this like any other platform, ignoring the Steam language.
			hook::put<uint8_t>(hook::get_pattern("44 38 25 ? ? ? ? 74 15 48 8B", 7), 0xEB);
		}
		else if (blob->epicSize)
		{
			hook::iat("kernel32.dll", LoadLibraryExWStub, "LoadLibraryExW");
			hook::iat("kernel32.dll", GetCommandLineAStub, "GetCommandLineA");
		}
	}
#endif
});

bool InLegitimacyUI()
{
	HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");
	return blob->inUI;
}
