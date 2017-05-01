/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "SteamComponent.h"
#include "InterfaceMapper.h"
#include "SafeClientEngine.h"

#include <HostSharedData.h>
#include <CfxState.h>

#include <CfxSubProcess.h>

#include <fstream>
#include <sstream>
#include <thread>

class KeyValuesBuilder
{
private:
	std::stringstream m_buffer;

	inline void PackBytes(const void* bytes, size_t size)
	{
		m_buffer << std::string(reinterpret_cast<const char*>(bytes), size);
	}

	inline void PackDataType(uint8_t type)
	{
		PackBytes(&type, 1);
	}

	inline void PackNullTerminated(const char* string)
	{
		PackBytes(string, strlen(string) + 1);
	}

public:
	inline void PackString(const char* key, const char* value)
	{
		PackDataType(1);
		PackNullTerminated(key);
		PackNullTerminated(value);
	}

	inline void PackUint64(const char* key, uint64_t value)
	{
		PackDataType(7);
		PackNullTerminated(key);
		PackBytes(&value, sizeof(value));
	}

	inline void PackEnd()
	{
		PackDataType(8);
	}

	inline std::string GetString()
	{
		return m_buffer.str();
	}
};

SteamComponent::SteamComponent()
	: m_client(nullptr), m_clientEngine(nullptr), m_callbackIndex(0), m_parentAppID(218)
{

}

static void RunChildLauncher()
{
	// get the base executable path
	auto ourPath = MakeCfxSubProcess(L"SteamChild.exe");

	wchar_t ourDirectory[MAX_PATH];
	GetCurrentDirectory(sizeof(ourDirectory), ourDirectory);

	std::wstring commandLine = va(L"\"%s\" -steamparent:%d", ourPath, GetCurrentProcessId());

	// run the steam parent
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	PROCESS_INFORMATION pi;

	CreateProcess(ourPath, (wchar_t*)commandLine.c_str(), nullptr, nullptr, FALSE, 0, nullptr, ourDirectory, &si, &pi);

	// wait for it to finish
	WaitForSingleObject(pi.hProcess, 15000);

	// and close up afterwards
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

#include <base64.h>

void SteamComponent::Initialize()
{
	// launch the presence dummy dummy if needed
	static HostSharedData<CfxState> hostData("CfxInitState");

	if (hostData->IsMasterProcess())
	{
		SetEnvironmentVariable(L"SteamAppId", L"218");

		m_steamLoader.Initialize();

		if (!m_steamLoader.IsSteamRunning(false))
		{
			return;
		}

		// delete any existing steam_appid.txt
		_unlink("steam_appid.txt");

		// initialize the public Steam API
		InitializePublicAPI();

		// run the child launcher
		RunChildLauncher();

		// initialize the private Steam API
		InitializeClientAPI();

		// run the backend thread
		RunThread();

		// write back steam_appid.txt for later purposes
		FILE* f = fopen("steam_appid.txt", "w");

		if (f)
		{
			fprintf(f, "%d", m_parentAppID);
			fclose(f);
		}
	}
}

void SteamComponent::InitializePublicAPI()
{
	auto createInterface = m_steamLoader.GetCreateInterfaceFunc();

	// get a IClientEngine pointer
	int returnCode;
	ISteamClient* clientEngine = reinterpret_cast<ISteamClient*>(createInterface("SteamClient016", &returnCode));

	if (clientEngine)
	{
		// connect to the local Steam client
		HSteamPipe steamPipe = clientEngine->CreateSteamPipe();
		HSteamUser steamUser = clientEngine->ConnectToGlobalUser(steamPipe);

		// if this all worked, set the member variables
		m_client = clientEngine;
		m_steamPipe = steamPipe;
		m_steamUser = steamUser;
	}
}

void SteamComponent::InitializeClientAPI()
{
	auto createInterface = m_steamLoader.GetCreateInterfaceFunc();

	// get a IClientEngine pointer
	int returnCode;
	IClientEngine* clientEngine = reinterpret_cast<IClientEngine*>(createInterface("CLIENTENGINE_INTERFACE_VERSION003", &returnCode));

	// if not existent, try ClientEngine004 - these coexisted for a while
	if (!clientEngine)
	{
		clientEngine = reinterpret_cast<IClientEngine*>(createInterface("CLIENTENGINE_INTERFACE_VERSION004", &returnCode));
	}

	if (clientEngine)
	{
		// if this all worked, set the member variables
		m_clientEngine = CreateSafeClientEngine(clientEngine, m_steamPipe, m_steamUser);
	}
}

struct CallbackMsg_t
{
	HSteamUser m_hSteamUser;
	int m_iCallback;
	uint8_t* m_pubParam;
	int m_cubParam;
};

void SteamComponent::RunThread()
{
	std::thread runtimeThread([=] ()
	{
		SetThreadName(-1, "Steam Worker Thread");

		auto getCallback = m_steamLoader.GetProcAddress<bool(*)(HSteamPipe, CallbackMsg_t*)>("Steam_BGetCallback");
		auto freeLastCallback = m_steamLoader.GetProcAddress<void(*)(HSteamPipe)>("Steam_FreeLastCallback");

		while (true)
		{
			Sleep(50);

			uint32_t lastRichPresence = 0;
			CallbackMsg_t callbackMsg;

			while (getCallback(m_steamPipe, &callbackMsg))
			{
				std::vector<std::function<void(void*)>> toInvoke;

				m_callbackMutex.lock();

				auto range = m_userCallbacks.equal_range(callbackMsg.m_iCallback);

				for (auto& it = range.first; it != range.second; it++)
				{
					toInvoke.push_back(it->second.second);
				}

				m_callbackMutex.unlock();

				// invoke the callbacks
				for (auto& callback : toInvoke)
				{
					callback(callbackMsg.m_pubParam);
				}

				freeLastCallback(m_steamPipe);
			}

			if ((GetTickCount() - lastRichPresence) > 5000)
			{
				UpdateRichPresence();

				lastRichPresence = GetTickCount();
			}

		}
	});

	runtimeThread.detach();
}

int SteamComponent::RegisterSteamCallbackRaw(int callbackID, std::function<void(void*)> callback)
{
	m_callbackMutex.lock();

	int callbackIndex = m_callbackIndex;

	m_userCallbacks.insert(std::make_pair(callbackID, std::make_pair(callbackIndex, callback)));
	m_callbackIndex++;

	m_callbackMutex.unlock();

	return callbackIndex;
}

void SteamComponent::RemoveSteamCallback(int registeredID)
{
	m_callbackMutex.lock();

	for (auto& it = m_userCallbacks.begin(); it != m_userCallbacks.end(); it++)
	{
		if (it->second.first == registeredID)
		{
			m_userCallbacks.erase(it);
			break;
		}
	}

	m_callbackMutex.unlock();
}

#if defined(GTA_NY)
#define PARENT_APP_ID 12210
#define PRODUCT_DISPLAY_NAME "CitizenFX:IV \xF0\x9F\x92\x8A"
#elif defined(PAYNE)
#define PARENT_APP_ID 204100
#define PRODUCT_DISPLAY_NAME "CitizenPayne \xF0\x9F\x92\x8A"
#elif defined(GTA_FIVE)
#define PARENT_APP_ID 218
#define PRODUCT_DISPLAY_NAME "FiveM \xF0\x9F\x90\x8C" // snail
#else
#define PARENT_APP_ID 218
#define PRODUCT_DISPLAY_NAME "Unknown CitizenFX product"
#endif

static int SehRoutine(PEXCEPTION_POINTERS exception)
{
	if (exception->ExceptionRecord->ExceptionCode & 0x80000000)
	{
		return EXCEPTION_EXECUTE_HANDLER;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

template<typename T>
static auto SafeCall(const T& fn)
{
	__try
	{
		return fn();
	}
	__except (SehRoutine(GetExceptionInformation()))
	{
		return std::result_of_t<T()>();
	}
}

void SteamComponent::InitializePresence()
{
	trace("Initializing Steam parent: Initializing presence.\n");

	// get a local-only app ID to register our app at
	InterfaceMapper steamShortcutsInterface(m_clientEngine->GetIClientShortcuts(m_steamUser, m_steamPipe, "CLIENTSHORTCUTS_INTERFACE_VERSION001"));

	if (!steamShortcutsInterface.IsValid())
	{
		return;
	}

	uint32_t appID = steamShortcutsInterface.Invoke<uint32_t>("GetUniqueLocalAppId");

	// check for ownership of a suitable parent game to use for the CGameID instance
	InterfaceMapper steamUserInterface(m_clientEngine->GetIClientUser(m_steamUser, m_steamPipe, "CLIENTUSER_INTERFACE_VERSION001"));

	uint32_t parentAppID = 243750;

	// set the parent appid in the instance
	m_parentAppID = parentAppID;

	// create a fake app to hold our gameid
	uint64_t gameID = 0xA18F2DAB01000000 | parentAppID; // crc32 for 'kekking' + mod

	InterfaceMapper steamAppsInterface(m_clientEngine->GetIClientApps(m_steamUser, m_steamPipe, "CLIENTAPPS_INTERFACE_VERSION001"));

	// create the keyvalues string for the app
	KeyValuesBuilder builder;
	builder.PackString("name", PRODUCT_DISPLAY_NAME);
	builder.PackUint64("gameid", gameID);
	builder.PackString("installed", "1");
	builder.PackString("gamedir", "kekking");
	builder.PackString("serverbrowsername", "lovely!");
	builder.PackEnd();

	std::string str = builder.GetString();

	// add the app to steamclient's point of view
	bool configAdded = steamAppsInterface.Invoke<bool>("SetLocalAppConfig", appID, str.c_str(), (uint32_t)str.size());

	// if the configuration is valid, launch our child process
	if (configAdded)
	{
		std::string productName = PRODUCT_DISPLAY_NAME;

		std::wstring namePath = MakeRelativeCitPath(L"steamname.txt");

		if (GetFileAttributes(namePath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			std::wifstream nameFile(namePath);
			std::wstringstream nameStream;
			nameStream << nameFile.rdbuf();
			nameFile.close();

			productName = ToNarrow(nameStream.str());
		}

		// set our pipe appid
		InterfaceMapper steamUtils(m_clientEngine->GetIClientUtils(m_steamPipe, "CLIENTUTILS_INTERFACE_VERSION001"));

		steamUtils.Invoke<void>("SetAppIDForCurrentPipe", parentAppID, false);

		// get the base executable path
		auto ourPath = MakeCfxSubProcess(L"SteamChild.exe");

		wchar_t ourDirectory[MAX_PATH];
		GetCurrentDirectory(sizeof(ourDirectory), ourDirectory);

		static HostSharedData<CfxState> hostData("CfxInitState");

		std::wstring commandLine = va(L"\"%s\" -steamchild:%d", ourPath, hostData->initialPid);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

		auto steamAttempts = std::vector<std::function<bool()>>{
			// before 2017-02 VR updates
			[&]()
			{
				steamUserInterface.Invoke<bool>("SpawnProcess", converter.to_bytes(ourPath).c_str(), converter.to_bytes(commandLine).c_str(), 0, converter.to_bytes(ourDirectory).c_str(), gameID, parentAppID, productName.c_str(), 0);
				return true;
			},

			// 2017-02-0x~ VR updates
			[&]()
			{
				// removed: parent app ID explicit argument
				// added: last argument (bitfield, related to SteamVR)
				steamUserInterface.Invoke<bool>("SpawnProcess", converter.to_bytes(ourPath).c_str(), converter.to_bytes(commandLine).c_str(), 0, converter.to_bytes(ourDirectory).c_str(), gameID, productName.c_str(), 0, 0);
				return true;
			}
		};

		trace("Initializing Steam parent: Attempting to run processes.\n");

		bool passedSteamPresence = false;

		for (const auto& fn : steamAttempts)
		{
			if (SafeCall(fn))
			{
				passedSteamPresence = true;
				break;
			}
		}

		if (!passedSteamPresence)
		{
			trace("Valve API change? SpawnProcess for presence helper failed on all attempts!\n");
		}

		SetRichPresenceTemplate("In the menus");
	}
}

bool SteamComponent::RunPresenceDummy()
{
	bool exitProcess = false;

	// check if the command line contains the -steamchild parameter
	const wchar_t* steamChildPart = L"-steamchild:";
	wchar_t* commandLineMatch = wcsstr(GetCommandLine(), steamChildPart);

	trace("CitizenFX Steam child starting - command line: %s\n", GetCommandLineA());
	
	// if it does
	if (commandLineMatch)
	{
		// parse the sequence following -steamchild
		int parentPid = _wtoi(&commandLineMatch[wcslen(steamChildPart)]);

		trace("game parent PID: %d\n", parentPid);

		// open a handle to the parent process with SYNCHRONIZE rights
		HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, parentPid);

		// mark us as needing to exit
		exitProcess = true;

		// if we opened the process...
		if (processHandle != INVALID_HANDLE_VALUE)
		{
			// do we like the process?
			trace("waiting for process to exit...\n");

			// ... wait for it to exit and close the handle afterwards
			WaitForSingleObject(processHandle, INFINITE);

			DWORD exitCode;
			GetExitCodeProcess(processHandle, &exitCode);

			CloseHandle(processHandle);

			// log it
			trace("process exited with %d!\n", exitCode);
		}
	}
	else
	{
		steamChildPart = L"-steamparent:";
		commandLineMatch = wcsstr(GetCommandLine(), steamChildPart);

		if (commandLineMatch)
		{
			trace("Initializing Steam parent.\n");

			SetEnvironmentVariable(L"SteamAppId", nullptr);

			m_steamLoader.Initialize();

			if (m_steamLoader.IsSteamRunning(false))
			{
				trace("Initializing Steam parent: Steam's running.\n");

				_unlink("steam_appid.txt");

				InitializePublicAPI();
				InitializeClientAPI();
				InitializePresence();
			}

			exitProcess = true;
		}
	}

	return exitProcess;
}

ISteamClient* SteamComponent::GetPublicClient()
{
	return m_client;
}

IClientEngine* SteamComponent::GetPrivateClient()
{
	return m_clientEngine;
}

HSteamUser SteamComponent::GetHSteamUser()
{
	return m_steamUser;
}

HSteamPipe SteamComponent::GetHSteamPipe()
{
	return m_steamPipe;
}

bool SteamComponent::IsSteamRunning()
{
	return m_steamLoader.IsSteamRunning(false);
}

int SteamComponent::GetParentAppID()
{
	return m_parentAppID;
}

void SteamComponent::SetConnectValue(const std::string& text)
{
	if (m_clientEngine)
	{
		InterfaceMapper steamFriendsInterface(m_clientEngine->GetIClientFriends(m_steamUser, m_steamPipe, "CLIENTFRIENDS_INTERFACE_VERSION001"));

		if (steamFriendsInterface.IsValid())
		{
			steamFriendsInterface.Invoke<bool>("SetRichPresence", 218, "status", text.c_str());
		}
	}
}

void SteamComponent::UpdateRichPresence()
{
	if (m_clientEngine)
	{
		InterfaceMapper steamFriendsInterface(m_clientEngine->GetIClientFriends(m_steamUser, m_steamPipe, "CLIENTFRIENDS_INTERFACE_VERSION001"));

		if (steamFriendsInterface.IsValid())
		{
			std::string formattedRichPresence = fmt::format(m_richPresenceTemplate,
				m_richPresenceValues[0],
				m_richPresenceValues[1],
				m_richPresenceValues[2],
				m_richPresenceValues[3],
				m_richPresenceValues[4],
				m_richPresenceValues[5],
				m_richPresenceValues[6],
				m_richPresenceValues[7]
			);

			steamFriendsInterface.Invoke<bool>("SetRichPresence", 218, "status", formattedRichPresence.c_str());
		}
	}
}

void SteamComponent::SetRichPresenceTemplate(const std::string& text)
{
	m_richPresenceTemplate = text;
}

void SteamComponent::SetRichPresenceValue(int idx, const std::string& value)
{
	assert(idx >= 0 && idx < _countof(m_richPresenceValues));

	m_richPresenceValues[idx] = value;
}

static SteamComponent steamComponent;

static InitFunction initFunction([] ()
{
	steamComponent.Initialize();

	Instance<ISteamComponent>::Set(&steamComponent);
});

void Component_RunPreInit()
{
	if (steamComponent.RunPresenceDummy())
	{
		ExitProcess(0);
	}
}