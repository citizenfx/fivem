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
//New libs needed for saveSettings
#include <fstream>
#include "KnownFolders.h"
#include <ShlObj.h>

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
		std::wofstream settingsFile(settingsPath);
		settingsFile << json;
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
			std::string json;
			settingsFile >> json;
			settingsFile.close();
			nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'loadedSettings', settings: '%s' }, '*');", json));
		}
		
		CoTaskMemFree(appDataPath);
	}
}

static InitFunction initFunction([] ()
{
	static NetLibrary* netLibrary;

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* lib)
	{
		netLibrary = lib;

		netLibrary->OnConnectionError.Connect([] (const char* error)
		{
			nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connectFailed', message: '%s' }, '*');", error));
		});
	});

	nui::OnInvokeNative.Connect([](const wchar_t* type, const wchar_t* arg)
	{
		if (!_wcsicmp(type, L"connectTo"))
		{
			std::wstring hostnameStrW = arg;
			std::string hostnameStr(hostnameStrW.begin(), hostnameStrW.end());

			static char hostname[256];

			StringCbCopyA(hostname, sizeof(hostname), hostnameStr.c_str());

			std::string port = std::string(hostname);
			std::string ip = std::string(hostname);
			ip = ip.substr(0, ip.find(":"));
			port = port.substr(port.find(":") + 1);
			const char* portnum = port.c_str();

			if (port.empty())
			{
				portnum = "30120";
			}

			netLibrary->ConnectToServer(ip.c_str(), atoi(portnum));

			nui::ExecuteRootScript("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connecting' }, '*');");
		}
		else if (!_wcsicmp(type, L"changeName"))
		{
			std::wstring newusernameStrW = arg;
			std::string newusername(newusernameStrW.begin(), newusernameStrW.end());
			if (!newusername.empty()) {
				netLibrary->SetPlayerName(newusername.c_str());
				saveSettings(arg);
				trace(va("Changed player name to %s\n", newusername.c_str()));
			}
		}
		else if (!_wcsicmp(type, L"loadSettings"))
		{
			loadSettings();
			trace("Settings loaded\n!");
		}
		else if (!_wcsicmp(type, L"checkNickname"))
		{
			if (arg == L"") return;
			const char* text = netLibrary->GetPlayerName();
			size_t size = strlen(text) + 1;
			wchar_t* wa = new wchar_t[size];
			mbstowcs(wa, text, size);
			if (arg != wa)
			{
				trace(va("Changed nickname (was new) to %s", text));
				netLibrary->SetPlayerName(text);
			}
		}
		else if (!_wcsicmp(type, L"exit"))
		{
			// queue an ExitProcess on the next game frame
			OnGameFrame.Connect([] ()
			{
				AddVectoredExceptionHandler(FALSE, TerminateInstantly);

				CefShutdown();

				ExitProcess(0);
			});
		}
	});

	OnMsgConfirm.Connect([] ()
	{
		nui::SetMainUI(true);

		nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");
	});
});