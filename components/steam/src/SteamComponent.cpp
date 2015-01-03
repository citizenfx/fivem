/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "SteamComponent.h"
#include "InterfaceMapper.h"

#include <sstream>

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

void SteamComponent::Initialize()
{
	// check if this is the presence dummy
	if (RunPresenceDummy())
	{
		ExitProcess(0);
	}

	m_steamLoader.Initialize();

	if (!m_steamLoader.IsSteamRunning())
	{
		return;
	}

	auto createInterface = m_steamLoader.GetCreateInterfaceFunc();

	// get a IClientEngine pointer
	int returnCode;
	IClientEngine* clientEngine = reinterpret_cast<IClientEngine*>(createInterface("CLIENTENGINE_INTERFACE_VERSION003", &returnCode));

	if (!clientEngine)
	{
		return;
	}

	// connect to the local Steam client
	HSteamPipe steamPipe = clientEngine->CreateSteamPipe();
	HSteamUser steamUser = clientEngine->ConnectToGlobalUser(steamPipe);

	// if this all worked, set the member variables
	m_clientEngine = clientEngine;
	m_steamPipe = steamPipe;
	m_steamUser = steamUser;

	// initialize the presence component
	InitializePresence();
}

#if defined(GTA_NY)
#define PARENT_APP_ID 12210
#define PRODUCT_DISPLAY_NAME "CitizenFX:IV \xF0\x9F\x92\x8A"
#elif defined(PAYNE)
#define PARENT_APP_ID 204100
#define PRODUCT_DISPLAY_NAME "CitizenPayne \xF0\x9F\x92\x8A"
#elif defined(GTA_FIVE)
#define PARENT_APP_ID 271590
#define PRODUCT_DISPLAY_NAME "FiveM \xF0\x9F\x92\x8A - GTA V, modified"
#else
#define PARENT_APP_ID 218
#define PRODUCT_DISPLAY_NAME "Unknown CitizenFX product"
#endif

void SteamComponent::InitializePresence()
{
	// get a local-only app ID to register our app at
	InterfaceMapper steamShortcutsInterface(m_clientEngine->GetIClientShortcuts(m_steamUser, m_steamPipe, "CLIENTSHORTCUTS_INTERFACE_VERSION001"));

	uint32_t appID = steamShortcutsInterface.Invoke<uint32_t>("GetUniqueLocalAppId");

	// check for ownership of a suitable parent game to use for the CGameID instance
	InterfaceMapper steamUserInterface(m_clientEngine->GetIClientUser(m_steamUser, m_steamPipe, "CLIENTUSER_INTERFACE_VERSION001"));

	uint32_t parentAppID = PARENT_APP_ID;

	uint32_t timeCreated = 0;
	bool success = steamUserInterface.Invoke<bool>("GetAppOwnershipInfo", parentAppID, &timeCreated, 0, 0);

	if (!success || timeCreated == 0)
	{
		parentAppID = 218;
	}

	// create a fake app to hold our gameid
	uint64_t gameID = 0xA18F2DAB01000000 | parentAppID; // crc32 for 'kekking' + mod

	InterfaceMapper steamAppsInterface(m_clientEngine->GetIClientApps(m_steamUser, m_steamPipe, "CLIENTAPPS_INTERFACE_VERSION001"));

	// create the keyvalues string for the app
	KeyValuesBuilder builder;
	builder.PackString("name", "lovely!");
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
		// get the base executable path
		char ourPath[MAX_PATH];
		GetModuleFileNameA(GetModuleHandle(nullptr), ourPath, _countof(ourPath));

		char ourDirectory[MAX_PATH];
		GetCurrentDirectoryA(sizeof(ourDirectory), ourDirectory);

		// create an empty VAC blob
		void* blob = new char[0];

		steamUserInterface.Invoke<bool>("SpawnProcess", blob, 0, ourPath, va("\"%s\" -steamchild:%d", ourPath, GetCurrentProcessId()), 0, ourDirectory, gameID, parentAppID, PRODUCT_DISPLAY_NAME, 0);
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

			CloseHandle(processHandle);

			// log it
			trace("process exited!\n");
		}
	}

	return exitProcess;
}

static SteamComponent steamComponent;

static InitFunction initFunction([] ()
{
	steamComponent.Initialize();
});