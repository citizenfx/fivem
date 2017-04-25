/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <CefOverlay.h>
#include <NetLibrary.h>
#include <strsafe.h>
#include <GlobalEvents.h>
#include <nutsnbolts.h>
#include <ConsoleHost.h>
//New libs needed for saveSettings
#include <fstream>
#include <sstream>
#include "KnownFolders.h"
#include <ShlObj.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <SteamComponentAPI.h>

static LONG WINAPI TerminateInstantly(LPEXCEPTION_POINTERS pointers)
{
	if (pointers->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT)
	{
		TerminateProcess(GetCurrentProcess(), 0xDEADCAFE);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

void saveSettings(const wchar_t *json) {
	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath))) {
		// create the directory if not existent
		std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
		CreateDirectory(cfxPath.c_str(), nullptr);
		// open and read the profile file
		std::wstring settingsPath = cfxPath + L"\\settings.json";
		std::ofstream settingsFile(settingsPath);
		//trace(va("Saving settings data %s\n", json));
		settingsFile << ToNarrow(json);
		settingsFile.close();
		CoTaskMemFree(appDataPath);
	}
}

void loadSettings() {
	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath))) {
		// create the directory if not existent
		std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
		CreateDirectory(cfxPath.c_str(), nullptr);

		// open and read the profile file
		std::wstring settingsPath = cfxPath + L"\\settings.json";
		if (FILE* profileFile = _wfopen(settingsPath.c_str(), L"rb"))
		{
			std::ifstream settingsFile(settingsPath);

			std::stringstream settingsStream;
			settingsStream << settingsFile.rdbuf();
			settingsFile.close();

			//trace(va("Loaded JSON settings %s\n", json.c_str()));
			nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'loadedSettings', json: %s }, '*');", settingsStream.str()));
		}
		
		CoTaskMemFree(appDataPath);
	}
}

inline ISteamComponent* GetSteam()
{
	auto steamComponent = Instance<ISteamComponent>::Get();

	// if Steam isn't running, return an error
	if (!steamComponent->IsSteamRunning())
	{
		steamComponent->Initialize();

		if (!steamComponent->IsSteamRunning())
		{
			return nullptr;
		}
	}

	return steamComponent;
}

inline bool HasDefaultName()
{
	auto steamComponent = GetSteam();

	if (steamComponent)
	{
		IClientEngine* steamClient = steamComponent->GetPrivateClient();

		InterfaceMapper steamFriends(steamClient->GetIClientFriends(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTFRIENDS_INTERFACE_VERSION001"));

		if (steamFriends.IsValid())
		{
			return true;
		}
	}

	return false;
}

static NetLibrary* netLibrary;

static void ConnectTo(const std::string& hostnameStr)
{
	static char hostname[256];

	StringCbCopyA(hostname, sizeof(hostname), hostnameStr.c_str());

	std::string port = std::string(hostname);
	std::string ip = std::string(hostname);
	ip = ip.substr(0, ip.find_last_of(":"));
	port = port.substr(port.find_last_of(":") + 1);
	const char* portnum = port.c_str();

	if (port.empty())
	{
		portnum = "30120";
	}

	netLibrary->ConnectToServer(ip.c_str(), atoi(portnum));

	nui::ExecuteRootScript("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connecting' }, '*');");
}

static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* lib)
	{
		netLibrary = lib;

		netLibrary->OnConnectionError.Connect([] (const char* error)
		{
			rapidjson::Document document;
			document.SetString(error, document.GetAllocator());

			rapidjson::StringBuffer sbuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

			document.Accept(writer);

			nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connectFailed', message: %s }, '*');", sbuffer.GetString()));
		});

		netLibrary->OnConnectionProgress.Connect([] (const std::string& message, int progress, int totalProgress)
		{
			rapidjson::Document document;
			document.SetObject();
			document.AddMember("message", rapidjson::Value(message.c_str(), message.size(), document.GetAllocator()), document.GetAllocator());
			document.AddMember("count", progress, document.GetAllocator());
			document.AddMember("total", totalProgress, document.GetAllocator());

			rapidjson::StringBuffer sbuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

			document.Accept(writer);

			nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connectStatus', data: %s }, '*');", sbuffer.GetString()));
		});
	});

	ConHost::OnInvokeNative.Connect([](const char* type, const char* arg)
	{
		if (!_stricmp(type, "connectTo"))
		{
			ConnectTo(arg);
		}
	});

	nui::OnInvokeNative.Connect([](const wchar_t* type, const wchar_t* arg)
	{
		if (!_wcsicmp(type, L"connectTo"))
		{
			std::wstring hostnameStrW = arg;
			std::string hostnameStr(hostnameStrW.begin(), hostnameStrW.end());

			ConnectTo(hostnameStr);
		}
		else if (!_wcsicmp(type, L"changeName"))
		{
			std::string newusername = ToNarrow(arg);
			if (!newusername.empty()) {
				if (newusername.c_str() != netLibrary->GetPlayerName()) {
					netLibrary->SetPlayerName(newusername.c_str());
					trace(va("Changed player name to %s\n", newusername.c_str()));
					nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'setSettingsNick', nickname: '%s' }, '*');", newusername.c_str()));
				}
			}
		}
		else if (!_wcsicmp(type, L"loadSettings"))
		{
			loadSettings();
			trace("Settings loaded!\n");
		}
		else if (!_wcsicmp(type, L"saveSettings"))
		{
			saveSettings(arg);
			trace("Settings saved!\n");
		}
		else if (!_wcsicmp(type, L"checkNickname"))
		{
			if (!arg || !arg[0]) return;
			const char* text = netLibrary->GetPlayerName();
			std::string newusername = ToNarrow(arg);

			if (text != newusername && !HasDefaultName()) // one's a string, two's a char, string meets char, string::operator== exists
			{
				trace("Loaded nickname: %s\n", newusername.c_str());
				netLibrary->SetPlayerName(newusername.c_str());
			}
		}
		else if (!_wcsicmp(type, L"exit"))
		{
			// queue an ExitProcess on the next game frame
			OnGameFrame.Connect([] ()
			{
				AddVectoredExceptionHandler(FALSE, TerminateInstantly);

				CefShutdown();

				TerminateProcess(GetCurrentProcess(), 0);
			});
		}
	});

	OnMsgConfirm.Connect([] ()
	{
		nui::SetMainUI(true);

		nui::CreateFrame("mpMenu", "nui://game/ui/app/index.html");
	});
});