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
#include <CoreConsole.h>
#include <ICoreGameInit.h>
#include <GameInit.h>
//New libs needed for saveSettings
#include <fstream>
#include <sstream>
#include "KnownFolders.h"
#include <ShlObj.h>

#include <json.hpp>

#include <CfxState.h>
#include <HostSharedData.h>

#include <network/uri.hpp>

#include <se/Security.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <SteamComponentAPI.h>

#include "GameInit.h"

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
			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "loadedSettings", "json": %s })", settingsStream.str()));
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
static bool g_connected;

static void ConnectTo(const std::string& hostnameStr)
{
	if (g_connected)
	{
		trace("Ignoring ConnectTo because we're already connecting/connected.\n");
		return;
	}

	g_connected = true;

	auto npa = net::PeerAddress::FromString(hostnameStr);

	if (npa)
	{
		nui::PostFrameMessage("mpMenu", R"({ "type": "connecting" })");

		netLibrary->ConnectToServer(npa.get());
	}
	else
	{
		trace("Could not resolve %s.\n", hostnameStr);
	}
}

static std::string g_pendingAuthPayload;

static void HandleAuthPayload(const std::string& payloadStr)
{
	if (nui::HasMainUI())
	{
		auto payloadJson = nlohmann::json(payloadStr).dump();

		nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "authPayload", "data": %s })", payloadJson));
	}
	else
	{
		g_pendingAuthPayload = payloadStr;
	}
}

static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* lib)
	{
		netLibrary = lib;

		netLibrary->OnConnectionError.Connect([] (const char* error)
		{
			g_connected = false;

			rapidjson::Document document;
			document.SetString(error, document.GetAllocator());

			rapidjson::StringBuffer sbuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

			document.Accept(writer);

			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectFailed", "message": %s })", sbuffer.GetString()));
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

			if (nui::HasMainUI())
			{
				nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectStatus", "data": %s })", sbuffer.GetString()));
			}
		});

		static std::function<void()> finishConnectCb;
		static bool disconnected;

		netLibrary->OnInterceptConnection.Connect([](const net::PeerAddress& peer, const std::function<void()>& cb)
		{
			if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
			{
				if (!disconnected)
				{
					netLibrary->OnConnectionProgress("Waiting for game to shut down...", 0, 100);

					finishConnectCb = cb;

					return false;
				}
			}
			else
			{
				disconnected = false;
			}

			return true;
		});

		Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([]()
		{
			disconnected = false;
		});

		Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
		{
			if (finishConnectCb)
			{
				auto cb = std::move(finishConnectCb);
				cb();
			}
			else
			{
				disconnected = true;
			}
		}, 5000);
	});

	OnKillNetwork.Connect([](const char*)
	{
		g_connected = false;
	});

	static ConsoleCommand connectCommand("connect", [](const std::string& server)
	{
		ConnectTo(server);
	});

	static ConsoleCommand disconnectCommand("disconnect", []()
	{
		if (netLibrary->GetConnectionState() != 0)
		{
			OnKillNetwork("Disconnected.");
			OnMsgConfirm();
		}
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
		else if (!_wcsicmp(type, L"cancelDefer"))
		{
			netLibrary->CancelDeferredConnection();

			g_connected = false;
		}
		else if (!_wcsicmp(type, L"changeName"))
		{
			std::string newusername = ToNarrow(arg);
			if (!newusername.empty()) {
				if (newusername.c_str() != netLibrary->GetPlayerName()) {
					netLibrary->SetPlayerName(newusername.c_str());
					trace(va("Changed player name to %s\n", newusername.c_str()));
					nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "setSettingsNick", "nickname": "%s" })", newusername));
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
		else if (!_wcsicmp(type, L"loadWarning"))
		{
			std::string warningMessage;

			if (Instance<ICoreGameInit>::Get()->GetData("warningMessage", &warningMessage))
			{
				if (!warningMessage.empty())
				{
					rapidjson::Document document;
					document.SetString(warningMessage.c_str(), document.GetAllocator());

					rapidjson::StringBuffer sbuffer;
					rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

					document.Accept(writer);

					nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "setWarningMessage", "message": %s })", sbuffer.GetString()));
				}

				Instance<ICoreGameInit>::Get()->SetData("warningMessage", "");
			}

			wchar_t computerName[256] = { 0 };
			DWORD len = _countof(computerName);
			GetComputerNameW(computerName, &len);

			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "setComputerName", "data": "%s" })", ToNarrow(computerName)));
		}
		else if (!_wcsicmp(type, L"checkNickname"))
		{
			if (!arg || !arg[0] || !netLibrary) return;
			const char* text = netLibrary->GetPlayerName();
			std::string newusername = ToNarrow(arg);

			if (text != newusername && !HasDefaultName()) // one's a string, two's a char, string meets char, string::operator== exists
			{
				trace("Loaded nickname: %s\n", newusername.c_str());
				netLibrary->SetPlayerName(newusername.c_str());
			}

			if (!g_pendingAuthPayload.empty())
			{
				auto pendingAuthPayload = g_pendingAuthPayload;

				g_pendingAuthPayload = "";

				HandleAuthPayload(pendingAuthPayload);
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

	OnGameFrame.Connect([]()
	{
		se::ScopedPrincipal scope(se::Principal{ "system.console" });
		Instance<console::Context>::Get()->ExecuteBuffer();
	});

	OnMsgConfirm.Connect([] ()
	{
		nui::SetMainUI(true);

		nui::CreateFrame("mpMenu", console::GetDefaultContext()->GetVariableManager()->FindEntryRaw("ui_url")->GetValue());
	});
});

#include <gameSkeleton.h>
#include <shellapi.h>

#include <nng.h>
#include <protocol/pipeline0/pull.h>
#include <protocol/pipeline0/push.h>

static void ProtocolRegister()
{
	LSTATUS result;

#define CHECK_STATUS(x) \
	result = (x); \
	if (result != ERROR_SUCCESS) { \
		trace("[Protocol Registration] " #x " failed: %x", result); \
		return; \
	}

	HKEY key;
	wchar_t path[MAX_PATH];
	wchar_t command[1024];

	GetModuleFileNameW(NULL, path, sizeof(path));
	swprintf_s(command, L"\"%s\" \"%%1\"", path);

	CHECK_STATUS(RegCreateKeyW(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\fivem", &key));
	CHECK_STATUS(RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)L"FiveM", 6 * 2));
	CHECK_STATUS(RegSetValueExW(key, L"URL Protocol", 0, REG_SZ, (BYTE*)L"", 1 * 2));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\FiveM.ProtocolHandler", &key));
	CHECK_STATUS(RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)L"FiveM", 6 * 2));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\FiveM", &key));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\FiveM\\Capabilities", &key));
	CHECK_STATUS(RegSetValueExW(key, L"ApplicationName", 0, REG_SZ, (BYTE*)L"FiveM", 6 * 2));
	CHECK_STATUS(RegSetValueExW(key, L"ApplicationDescription", 0, REG_SZ, (BYTE*)L"FiveM", 6 * 2));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\FiveM\\Capabilities\\URLAssociations", &key));
	CHECK_STATUS(RegSetValueExW(key, L"fivem", 0, REG_SZ, (BYTE*)L"FiveM.ProtocolHandler", 22 * 2));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\RegisteredApplications", &key));
	CHECK_STATUS(RegSetValueExW(key, L"FiveM", 0, REG_SZ, (BYTE*)L"Software\\FiveM\\Capabilities", 28 * 2));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Classes\\FiveM.ProtocolHandler\\shell\\open\\command", &key));
	CHECK_STATUS(RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)command, (wcslen(command) * sizeof(wchar_t)) + 2));
	CHECK_STATUS(RegCloseKey(key));
}

void Component_RunPreInit()
{
	static HostSharedData<CfxState> hostData("CfxInitState");

	if (hostData->IsMasterProcess())
	{
		ProtocolRegister();
	}

	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	static std::string connectHost;
	static std::string authPayload;

	for (int i = 1; i < argc; i++)
	{
		std::string arg = ToNarrow(argv[i]);

		if (arg.find("fivem:") == 0)
		{
			std::error_code ec;
			network::uri parsed = network::make_uri(arg, ec);

			if (!static_cast<bool>(ec))
			{
				if (!parsed.host().empty())
				{
					if (parsed.host().to_string() == "connect")
					{
						if (!parsed.path().empty())
						{
							connectHost = parsed.path().substr(1).to_string();
						}
					}
					else if (parsed.host().to_string() == "accept-auth")
					{
						if (!parsed.query().empty())
						{
							authPayload = parsed.query().to_string();
						}
					}
				}
			}

			break;
		}
	}

	LocalFree(argv);

	if (!connectHost.empty())
	{
		if (hostData->IsMasterProcess() || hostData->IsGameProcess())
		{
			rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
			{
				if (type == rage::InitFunctionType::INIT_CORE)
				{
					ConnectTo(connectHost);
					connectHost = "";
				}
			}, 999999);
		}
		else
		{
			nng_socket socket;
			nng_dialer dialer;

			nng_push0_open(&socket);
			nng_dial(socket, "ipc:///tmp/fivem_connect", &dialer, 0);
			nng_send(socket, const_cast<char*>(connectHost.c_str()), connectHost.size(), 0);

			if (!hostData->gamePid)
			{
				AllowSetForegroundWindow(hostData->initialPid);
			}
			else
			{
				AllowSetForegroundWindow(hostData->gamePid);
			}

			TerminateProcess(GetCurrentProcess(), 0);
		}
	}

	if (!authPayload.empty())
	{
		if (hostData->IsMasterProcess() || hostData->IsGameProcess())
		{
			rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
			{
				if (type == rage::InitFunctionType::INIT_CORE)
				{
					HandleAuthPayload(authPayload);
					authPayload = "";
				}
			}, 999999);
		}
		else
		{
			nng_socket socket;
			nng_dialer dialer;

			nng_push0_open(&socket);
			nng_dial(socket, "ipc:///tmp/fivem_auth", &dialer, 0);
			nng_send(socket, const_cast<char*>(authPayload.c_str()), authPayload.size(), 0);

			if (!hostData->gamePid)
			{
				AllowSetForegroundWindow(hostData->initialPid);
			}
			else
			{
				AllowSetForegroundWindow(hostData->gamePid);
			}

			TerminateProcess(GetCurrentProcess(), 0);
		}
	}
}

static InitFunction connectInitFunction([]()
{
	static nng_socket netSocket;
	static nng_listener listener;

	nng_pull0_open(&netSocket);
	nng_listen(netSocket, "ipc:///tmp/fivem_connect", &listener, 0);

	static nng_socket netAuthSocket;
	static nng_listener authListener;

	nng_pull0_open(&netAuthSocket);
	nng_listen(netAuthSocket, "ipc:///tmp/fivem_auth", &authListener, 0);

	OnGameFrame.Connect([]()
	{
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}

		char* buffer;
		size_t bufLen;

		int err;

		err = nng_recv(netSocket, &buffer, &bufLen, NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC);

		if (err == 0)
		{
			std::string connectMsg(buffer, buffer + bufLen);
			nng_free(buffer, bufLen);

			ConnectTo(connectMsg);

			SetForegroundWindow(FindWindow(L"grcWindow", nullptr));
		}

		err = nng_recv(netAuthSocket, &buffer, &bufLen, NNG_FLAG_NONBLOCK | NNG_FLAG_ALLOC);

		if (err == 0)
		{
			std::string msg(buffer, buffer + bufLen);
			nng_free(buffer, bufLen);

			HandleAuthPayload(msg);

			SetForegroundWindow(FindWindow(L"grcWindow", nullptr));
		}
	});
});
