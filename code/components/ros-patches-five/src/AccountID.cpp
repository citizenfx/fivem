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

#include <HostSharedData.h>

struct ExternalROSBlob
{
	uint8_t data[16384];
	bool valid;
	bool tried;

	ExternalROSBlob()
	{
		valid = false;
		tried = false;
		memset(data, 0, sizeof(data));
	}
};

bool IsSteamTicket()
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");

	return blob->tried && blob->valid;
}

#if defined(IS_RDR3)
static uint8_t* accountBlob;

static DWORD GetMTLPid()
{
	static DWORD pids[16384];

	DWORD len;
	if (!EnumProcesses(pids, sizeof(pids), &len))
	{
		return -1;
	}

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
							CloseHandle(hProcess);
							return pids[i];
						//}
					}
				}
			}

			CloseHandle(hProcess);
		}
	}

	return -1;
}

bool GetMTLSessionInfo(std::string& ticket, std::string& sessionTicket, std::array<uint8_t, 16>& sessionKey)
{
	if (!accountBlob)
	{
		return false;
	}

	ticket = std::string((const char*)&accountBlob[2800]);
	sessionTicket = std::string((const char*)&accountBlob[3312]);
	memcpy(sessionKey.data(), &accountBlob[0x10D8], sessionKey.size());

	return true;
}

void RunLauncher(const wchar_t* toolName, bool instantWait);

static bool InitAccountSteam()
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");

	if (!blob->tried)
	{
		RunLauncher(L"ros:steam", true);
	}

	if (blob->valid)
	{
		accountBlob = blob->data;
		trace("Steam ROS says it's signed in: %s\n", (const char*)&accountBlob[8]);
	}

	return blob->valid;
}

std::string GetAuthSessionTicket(uint32_t appID);

void ValidateSteam(int parentPid)
{
	static HostSharedData<ExternalROSBlob> blob("Cfx_ExtRosBlob");
	blob->tried = true;

	std::string s = GetAuthSessionTicket(1174180);

	if (s.empty())
	{
		return;
	}

	auto j = nlohmann::json::object({
		{ "appName", "rdr2" },
		{ "platform", "pcros" },
		{ "authTicket", s }
	});

	auto r = cpr::Post(
		cpr::Url{ "https://ros.citizenfx.internal/scui/v2/api/rdr2/autologinsteam" },
		cpr::Header{
			{
				{"Content-Type", "application/json; charset=utf-8"},
				{"Accept", "application/json"},
				{"Host", "prod.ros.rockstargames.com"},
				{"X-Requested-With", "XMLHttpRequest"}
			}
		},
		cpr::Body{
			j.dump()
		},
		cpr::VerifySsl{ false });

	if (r.error)
	{
		FatalError("Error during auto-signin with ROS using Steam: %s", r.error.message);
	}

	j = nlohmann::json::parse(r.text);

	if (!j["Status"].get<bool>())
	{
		FatalError("Error during Steam ROS signin: %s %s", j.value("Error", ""), j.value("Message", ""));
	}

	auto sessionKey = Botan::base64_decode(j.value("SessionKey", ""));

	std::istringstream stream(j.value("Xml", ""));

	boost::property_tree::ptree tree;
	boost::property_tree::read_xml(stream, tree);

	auto tick = j.value("Ticket", "");

	*(uint64_t*)&blob->data[3816] = j.value("RockstarId", uint64_t(0));
	//*(uint64_t*)&blob->data[3816] = j.value("PlayerAccountId", uint64_t(0));
	strcpy((char*)&blob->data[2800], j.value("Ticket", "").c_str());
	strcpy((char*)&blob->data[3312], tree.get<std::string>("Response.SessionTicket").c_str());
	memcpy(&blob->data[0x10D8], sessionKey.data(), sessionKey.size());
	strcpy((char*)&blob->data[8], j.value("Email", "").c_str());

	j = nlohmann::json::object({
		{ "title", "rdr2" },
		{ "env", "prod" },
		{ "steamAuthTicket", s },
		{ "steamAppId", 1174180 },
		{ "playerTicket", tick },
		{ "version", 11 },
	});

	r = cpr::Post(
		cpr::Url{ "https://ros.citizenfx.internal/scui/v2/api/rdr2/bindsteamaccount" },
		cpr::Header{
			{
				{"Content-Type", "application/json; charset=utf-8"},
				{"Accept", "application/json"},
				{"Host", "prod.ros.rockstargames.com"},
				{"X-Requested-With", "XMLHttpRequest"},
				{"Authorization", fmt::sprintf("SCAUTH val=\"%s\"", tick) },
			}
		},
		cpr::Body{
			j.dump()
		},
		cpr::VerifySsl{ false });

	if (r.error)
	{
		FatalError("Error during binding of ROS to Steam: %s", r.error.message);
	}

	j = nlohmann::json::parse(r.text);

	blob->valid = true;
}

static bool InitAccountMTL()
{
	auto pid = GetMTLPid();

	if (pid == -1)
	{
		MessageBoxW(NULL, L"Currently, you have to run the Rockstar Games Launcher to be able to run this product.", L"RedM", MB_OK | MB_ICONSTOP);

		return false;
	}

	auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

	if (!hProcess)
	{
		MessageBoxW(NULL, L"Could not read data from the Rockstar Games Launcher. If you are running it as administrator, please launch it as regular user.", L"RedM", MB_OK | MB_ICONSTOP);

		return false;
	}

	HMODULE hMods[1024];
	DWORD cbNeeded;

	HMODULE scModule = NULL;

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
					if (_wcsicmp(lastEnd, L"\\socialclub.dll"))
					{
						scModule = hMods[i];
						break;
					}
				}
			}
		}
	}

	if (!scModule)
	{
		FatalError("MTL didn't have SC SDK loaded.");
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

	auto p = hook::range_pattern((uintptr_t)exeCode.data(), (uintptr_t)exeCode.data() + exeCode.size(), "84 C0 75 19 FF C7 83 FF 01 7C D8").count(1).get(0).get<uint8_t>(-35);
	auto memOff = *(uint32_t*)p;

	// rebase to MODULE-RELATIVE offset
	auto origOff = p - exeCode.data() + (uint8_t*)scModule + 0x1000;
	auto bufOff = origOff + memOff + 4;

	accountBlob = new uint8_t[16384];
	ReadProcessMemory(hProcess, bufOff, accountBlob, 16384, &nr);

	if (!isalnum(accountBlob[8]))
	{
		FatalError("No account blob info?");
	}

	trace("MTL says it's signed in: %s\n", (const char*)&accountBlob[8]);

	return true;
}

static InitFunction initFunction([]()
{
	if (wcsstr(GetCommandLineW(), L"ros:steam") != nullptr)
	{
		return;
	}

	if (!InitAccountSteam() && !InitAccountMTL())
	{
		TerminateProcess(GetCurrentProcess(), 0);
	}
});

#include <ICoreGameInit.h>

static HookFunction hookFunction([]()
{
	Instance<ICoreGameInit>::Get()->SetData("rosUserName", (const char*)&accountBlob[8]);
});
#else
void ValidateSteam(int)
{
}
#endif

uint64_t ROSGetDummyAccountID()
{
#if defined(IS_RDR3)
	return *(uint64_t*)&accountBlob[3816];
#else
	static std::once_flag gotAccountId;
	static uint32_t accountId;

	std::call_once(gotAccountId, [] ()
	{
		PWSTR appdataPath = nullptr;
		SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appdataPath);

		CreateDirectory(va(L"%s\\CitizenFX", appdataPath), nullptr);

		FILE* f = _wfopen(va(L"%s\\CitizenFX\\ros_id.dat", appdataPath), L"rb");

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
			f = _wfopen(va(L"%s\\CitizenFX\\ros_id.dat", appdataPath), L"wb");

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

				_wunlink(va(L"%s\\CitizenFX\\ros_id.dat", appdataPath));

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
