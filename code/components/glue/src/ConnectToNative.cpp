/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <CfxLocale.h>
#include <CefOverlay.h>
#include <NetLibrary.h>
#include <strsafe.h>
#include <GlobalEvents.h>

#include <nutsnbolts.h>
#include <ConsoleHost.h>
#include <CoreConsole.h>
#include <ICoreGameInit.h>
#include <GameInit.h>
#include <ScriptEngine.h>
#include <ResourceManager.h>
#include <ResourceEventComponent.h>
//New libs needed for saveSettings
#include <fstream>
#include <sstream>
#include "KnownFolders.h"
#include <ShlObj.h>
#include <Shellapi.h>
#include <HttpClient.h>
#include <InputHook.h>
#include <RelativeDevice.h>
#include <VFSManager.h>

#include <json.hpp>

#include <CfxState.h>
#include <HostSharedData.h>

#include <skyr/url.hpp>

#include <se/Security.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <LauncherIPC.h>

#include <CrossBuildRuntime.h>

#include <SteamComponentAPI.h>

#include <MinMode.h>

#include "CfxState.h"
#include "GameInit.h"
#include "CnlEndpoint.h"
#include "PacketHandler.h"
#include "PaymentRequest.h"

#include "LinkProtocolIPC.h"

#ifdef GTA_FIVE
#include <ArchetypesCollector.h>
#endif

inline auto& GetEarlyGameFrame()
{
	auto& earlyGameFrame =
#if defined(HAS_EARLY_GAME_FRAME)
	OnEarlyGameFrame
#else
	OnGameFrame
#endif
	;

	return earlyGameFrame;
}

std::string g_lastConn;
static std::string g_connectNonce;

extern bool XBR_InterceptCardResponse(const nlohmann::json& j);
extern bool XBR_InterceptCancelDefer();

static LONG WINAPI TerminateInstantly(LPEXCEPTION_POINTERS pointers)
{
	if (pointers->ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT)
	{
		TerminateProcess(GetCurrentProcess(), 0xDEADCAFE);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

static void SaveBuildNumber(uint32_t build)
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		WritePrivateProfileString(L"Game", L"SavedBuildNumber", fmt::sprintf(L"%d", build).c_str(), fpath.c_str());
	}
}

static void SaveGameSettings(const std::wstring& poolIncreases, bool replaceExecutable)
{
	std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

	if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		WritePrivateProfileString(L"Game", L"PoolSizesIncrease", poolIncreases.c_str(), fpath.c_str());
		WritePrivateProfileString(L"Game", L"ReplaceExecutable", replaceExecutable ? L"1" : L"0", fpath.c_str());
	}
}

void RestartGameToOtherBuild(int build, int pureLevel, std::wstring poolSizesIncreaseSetting, bool replaceExecutable)
{
#if defined(GTA_FIVE) || defined(IS_RDR3)
	SECURITY_ATTRIBUTES securityAttributes = { 0 };
	securityAttributes.bInheritHandle = TRUE;

	HANDLE switchEvent = CreateEventW(&securityAttributes, TRUE, FALSE, NULL);

	static HostSharedData<CfxState> hostData("CfxInitState");

	auto cli = fmt::sprintf(L"\"%s\" %s %s %s -switchcl:%d \"%s://connect/%s\"",
	hostData->gameExePath,
	fmt::sprintf(L"-b%d", build),
	IsCL2() ? L"-cl2" : L"",
	pureLevel == 0 ? L"" : fmt::sprintf(L"-pure_%d", pureLevel),
	(uintptr_t)switchEvent,
	hostData->GetLinkProtocol(),
	ToWide(g_lastConn));

	// we won't launch the default build if we don't do this
	if (build == xbr::GetDefaultGameBuild())
	{
		SaveBuildNumber(xbr::GetDefaultGameBuild());
	}

	SaveGameSettings(poolSizesIncreaseSetting, replaceExecutable);

	trace("Switching from build %d to build %d...\n", xbr::GetRequestedGameBuild(), build);

	SIZE_T size = 0;
	InitializeProcThreadAttributeList(NULL, 1, 0, &size);

	std::vector<uint8_t> attListData(size);
	auto attList = (LPPROC_THREAD_ATTRIBUTE_LIST)attListData.data();

	assert(attList);

	InitializeProcThreadAttributeList(attList, 1, 0, &size);
	UpdateProcThreadAttribute(attList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, &switchEvent, sizeof(HANDLE), NULL, NULL);

	STARTUPINFOEXW si = { 0 };
	si.StartupInfo.cb = sizeof(si);
	si.lpAttributeList = attList;

	PROCESS_INFORMATION pi;

	if (!CreateProcessW(NULL, const_cast<wchar_t*>(cli.c_str()), NULL, NULL, TRUE, CREATE_BREAKAWAY_FROM_JOB | EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &si.StartupInfo, &pi))
	{
		trace("failed to exit: %d\n", GetLastError());
	}
	else
	{
		// to silence VS analyzers
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	ExitProcess(0x69);
#endif
}

extern void InitializeBuildSwitch(int build, int pureLevel, std::wstring poolSizesIncreaseSetting, bool replaceExecutable);

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

NetLibrary* netLibrary;
static bool g_connected;

static void UpdatePendingAuthPayload();

static void SetNickname(const std::string& name)
{
	if (!netLibrary)
	{
		NetLibrary::OnNetLibraryCreate.Connect([name](NetLibrary*)
		{
			SetNickname(name);
		}, INT32_MAX);

		return;
	}

	const char* text = netLibrary->GetPlayerName();

	if (text != name && !name.empty())
	{
		trace("Loaded nickname: %s\n", name);
		netLibrary->SetPlayerName(name.c_str());
	}

	UpdatePendingAuthPayload();
}

static void ConnectTo(const std::string& hostnameStr, bool fromUI = false, const std::string& connectParams = "")
{
	auto connectParamsReal = connectParams;
	static bool switched;

	if (wcsstr(GetCommandLineW(), L"-switchcl") && !switched)
	{
		connectParamsReal = "switchcl=true&" + connectParamsReal;
		switched = true;
	}

	if (!fromUI && !launch::IsSDKGuest())
	{
		if (nui::HasMainUI())
		{
			auto j = nlohmann::json::object({
				{ "type", "connectTo" },
				{ "hostnameStr", hostnameStr },
				{ "connectParams", connectParamsReal }
			});
			nui::PostFrameMessage("mpMenu", j.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));

			return;
		}
	}

	if (g_connected)
	{
		trace("Ignoring ConnectTo because we're already connecting/connected.\n");
		return;
	}

	g_connected = true;

	nui::PostFrameMessage("mpMenu", R"({ "type": "connecting" })");

	g_lastConn = hostnameStr;

	if (!hostnameStr.empty() && hostnameStr[0] == '-')
	{
		netLibrary->ConnectToServer("cfx.re/join/" + hostnameStr.substr(1));
	}
	else
	{
		netLibrary->ConnectToServer(hostnameStr);
	}
}

static std::string g_pendingAuthPayload;

static void HandleAuthPayload(const std::string& payloadStr)
{
	if (nui::HasMainUI())
	{
		auto payloadJson = nlohmann::json(payloadStr).dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);

		nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "authPayload", "data": %s })", payloadJson));
	}
	else
	{
		g_pendingAuthPayload = payloadStr;
	}
}

#include <LegitimacyAPI.h>

static std::string g_discourseClientId;
static std::string g_discourseUserToken;

static std::string g_cardConnectionToken;

struct ServerLink
{
	std::string rawIcon;
	std::string hostname;
	std::string url;
	nlohmann::json vars;
};

#include <wrl.h>
#include <psapi.h>
#include <propsys.h>
#include <propkey.h>
#include <propvarutil.h>
#include <botan/base64.h>

namespace WRL = Microsoft::WRL;

static WRL::ComPtr<IShellLink> MakeShellLink(const ServerLink& link)
{
	WRL::ComPtr<IShellLink> psl;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&psl));

	if (SUCCEEDED(hr))
	{
		std::wstring buildArgument;
		try
		{
			if (link.vars.is_object())
			{
				if (auto it = link.vars.find("sv_enforceGameBuild"); it != link.vars.end())
				{
					buildArgument = fmt::sprintf(L"-b%d ", atoi(it.value().get<std::string>().c_str()));
				}
			}
		}
		catch (...)
		{
		}

		static HostSharedData<CfxState> hostData("CfxInitState");

		wchar_t imageFileName[1024];

		auto hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, hostData->GetInitialPid());
		GetModuleFileNameEx(hProcess, NULL, imageFileName, std::size(imageFileName));

		psl->SetPath(imageFileName);
		psl->SetArguments(fmt::sprintf(L"%s%s://connect/%s", buildArgument, hostData->GetLinkProtocol(), ToWide(link.url)).c_str());

		WRL::ComPtr<IPropertyStore> pps;
		psl.As(&pps);

		PROPVARIANT propvar;
		hr = InitPropVariantFromString(ToWide(link.hostname).c_str(), &propvar);
		hr = pps->SetValue(PKEY_Title, propvar);
		hr = pps->Commit();
		PropVariantClear(&propvar);

		psl->SetIconLocation(imageFileName, -201);

		if (!link.rawIcon.empty())
		{
			auto iconPath = MakeRelativeCitPath(fmt::sprintf(L"data/cache/servers/%08x.ico", HashString(link.rawIcon.c_str())));
			
			FILE* f = _wfopen(iconPath.c_str(), L"wb");

			if (f)
			{
				auto data = Botan::base64_decode(link.rawIcon.substr(strlen("data:image/png;base64,")));

				uint8_t iconHeader[] = {
					0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
					0xff, 0xff, 0x16, 0x00, 0x00, 0x00
				};

				*(uint32_t*)&iconHeader[14] = data.size();

				fwrite(iconHeader, 1, sizeof(iconHeader), f);

				fwrite(data.data(), 1, data.size(), f);
				fclose(f);

				psl->SetIconLocation(iconPath.c_str(), 0);
			}
		}
	}

	return psl;
}

static void SetShellIcon(const std::string& rawIcon)
{
	WRL::ComPtr<ITaskbarList3> tbl;
	if (FAILED(CoCreateInstance(CLSID_TaskbarList,
		NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&tbl))))
	{
		return;
	}

	auto gameWindow = CoreGetGameWindow();

	if (!rawIcon.empty())
	{
		try
		{
			auto data = Botan::base64_decode(rawIcon);
			HICON hIcon = CreateIconFromResourceEx(data.data(), data.size(), TRUE, 0x30000, 16, 16, LR_DEFAULTCOLOR);
			tbl->SetOverlayIcon(gameWindow, hIcon, L"Server");
			DeleteObject(hIcon);
		}
		catch (...)
		{
		}
	}
	else
	{
		tbl->SetOverlayIcon(gameWindow, nullptr, nullptr);
	}
}

static void UpdateJumpList(const std::vector<ServerLink>& links)
{
	PWSTR aumid;
	GetCurrentProcessExplicitAppUserModelID(&aumid);

	WRL::ComPtr<ICustomDestinationList> pcdl;
	HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));

	if (FAILED(hr))
	{
		CoTaskMemFree(aumid);
		return;
	}

	pcdl->SetAppID(aumid);
	CoTaskMemFree(aumid);

	UINT cMinSlots;
	WRL::ComPtr<IObjectArray> poaRemoved;

	hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));

	if (FAILED(hr))
	{
		return;
	}

	{
		WRL::ComPtr<IObjectCollection> poc;
		hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));

		if (FAILED(hr))
		{
			return;
		}

		for (int i = 0; i < std::min(links.size(), size_t(cMinSlots)); i++)
		{
			auto shellLink = MakeShellLink(links[i]);

			poc->AddObject(shellLink.Get());
		}

		pcdl->AppendCategory(L"History", poc.Get());
	}

	pcdl->CommitList();
}

void DLL_IMPORT UiDone();

static void UpdatePendingAuthPayload()
{
	if (!g_pendingAuthPayload.empty())
	{
		auto pendingAuthPayload = g_pendingAuthPayload;
		g_pendingAuthPayload = "";

		HandleAuthPayload(pendingAuthPayload);
	}
}

static void DisconnectCmd()
{
	if (netLibrary->GetConnectionState() != 0)
	{
		OnKillNetwork("Disconnected.");
		OnMsgConfirm();
	}
}

extern void MarkNuiLoaded();

static std::function<void()> g_onYesCallback;

class PaymentRequestPacketHandler : public net::PacketHandler<net::packet::ServerPaymentRequest, HashRageString("msgPaymentRequest")>
{
public:
	template<typename T>
	bool Process(T& stream)
	{
		return ProcessPacket(stream, [](net::packet::ServerPaymentRequest& serverPaymentRequest)
		{
			try
			{
				auto json = nlohmann::json::parse(std::string(reinterpret_cast<const char*>(serverPaymentRequest.data.GetValue().data()), serverPaymentRequest.data.GetValue().size()));

				se::ScopedPrincipal scope(se::Principal{ "system.console" });
				console::GetDefaultContext()->GetVariableManager()->FindEntryRaw("warningMessageResult")->SetValue("0");
				console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "warningmessage", "PURCHASE REQUEST", fmt::sprintf("The server is requesting a purchase of %s for %s.", json.value("sku_name", ""), json.value("sku_price", "")), "Do you want to purchase this item?", "20" });

				g_onYesCallback = [json]()
				{
					std::map<std::string, std::string> postMap;
					postMap["data"] = json.value<std::string>("data", "");
					postMap["sig"] = json.value<std::string>("sig", "");
					postMap["clientId"] = g_discourseClientId;
					postMap["userToken"] = g_discourseUserToken;

					Instance<HttpClient>::Get()->DoPostRequest("https://keymaster.fivem.net/api/paymentAssign", postMap, [](bool success, const char* data, size_t length)
					{
						if (success)
						{
							auto res = nlohmann::json::parse(std::string(data, length));
							auto url = res.value("url", "");

							if (!url.empty())
							{
								if (url.find("http://") == 0 || url.find("https://") == 0)
								{
									ShellExecute(nullptr, L"open", ToWide(url).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
								}
							}
						}
					});
				};
			}
			catch (const std::exception& e)
			{

			}
		});
	}
};


static InitFunction initFunction([] ()
{
	static std::function<void()> backfillDoneEvent;
	static bool mpMenuExpectsBackfill;
	static bool disconnect;

	static ipc::Endpoint ep("launcherTalk", false);

	OnCriticalGameFrame.Connect([]()
	{
		ep.RunFrame();
	});

	OnGameFrame.Connect([]()
	{
		if (disconnect)
		{
			DisconnectCmd();
			disconnect = false;
		}

		ep.RunFrame();
	});

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([]()
	{
		ep.Call("loading");
	});

	InputHook::DeprecatedOnWndProc.Connect([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool& pass, LRESULT& lresult)
	{
		// we don't want the game to be informed about the system entering a low-power state
		if (msg == WM_POWERBROADCAST)
		{
			// if the system is resumed, we do want to try to manually disconnect
			if (wParam == PBT_APMRESUMEAUTOMATIC || wParam == PBT_APMRESUMESUSPEND)
			{
				disconnect = true;
			}

			pass = false;
			lresult = true;
		}
	});

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* lib)
	{
		netLibrary = lib;

		netLibrary->OnInfoBlobReceived.Connect([](std::string_view sv, const std::function<void()>& cb)
		{
			if (!nui::HasFrame("mpMenu") || !mpMenuExpectsBackfill)
			{
				cb();
				return;
			}

			try
			{
				auto info = nlohmann::json::parse(sv);
				std::string iconUri = "";
				std::string svLicenseKeyToken = "";

				if (auto it = info.find("icon"); it != info.end())
				{
					auto iconStr = it.value().get<std::string>();
					iconUri = "data:image/png;base64," + iconStr;

					SetShellIcon(iconStr);
				}

				if (auto it = info.find("vars"); it != info.end())
				{
					if (auto it2 = it.value().find("sv_licenseKeyToken"); it2 != it.value().end())
					{
						svLicenseKeyToken = it2.value().get<std::string>();
					}
				}

				auto outInfo = nlohmann::json::object({ { "icon", iconUri }, { "token", svLicenseKeyToken }, { "vars", info["vars"] } });
				auto outData = nlohmann::json::object({ { "nonce", g_connectNonce }, { "server", outInfo } }).dump();

				nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "backfillServerInfo", "data": %s })", outData));
				backfillDoneEvent = cb;
			}
			catch (...)
			{
				cb();
			}
		});

		netLibrary->OnConnectOKReceived.Connect([](NetAddress)
		{
			auto peerAddress = netLibrary->GetCurrentPeer().ToString();

			nui::PostRootMessage(fmt::sprintf(R"({ "type": "setServerAddress", "data": "%s" })", peerAddress));
		});

		netLibrary->OnRequestBuildSwitch.Connect([](int build, int pureLevel, std::wstring poolSizesIncreaseSetting, bool replaceExecutable)
		{
			InitializeBuildSwitch(build, pureLevel, std::move(poolSizesIncreaseSetting), replaceExecutable);
			g_connected = false;
		});

		netLibrary->OnConnectionErrorRichEvent.Connect([] (const std::string& errorOrig, const std::string& metaData)
		{
			std::string error = errorOrig;

			if ((strstr(error.c_str(), "steam") || strstr(error.c_str(), "Steam")) && !strstr(error.c_str(), ".ms/verify"))
			{
				if (auto steam = GetSteam())
				{
					if (steam->IsSteamRunning())
					{
						if (IClientEngine* steamClient = steam->GetPrivateClient())
						{
							InterfaceMapper steamUser(steamClient->GetIClientUser(steam->GetHSteamUser(), steam->GetHSteamPipe(), "CLIENTUSER_INTERFACE_VERSION001"));

							if (steamUser.IsValid())
							{
								uint64_t steamID = 0;
								steamUser.Invoke<void>("GetSteamID", &steamID);

								if ((steamID & 0xFFFFFFFF00000000) != 0)
								{
									error += "\nThis is a Steam authentication failure, but you are running Steam and it is signed in. The server owner can find more information in their server console.";
								}
							}
						}
					}
				}
			}

			console::Printf("no_console", "OnConnectionError: %s\n", error);

			g_connected = false;

			rapidjson::Document document;
			document.SetString(error.c_str(), document.GetAllocator());

			rapidjson::StringBuffer sbuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

			document.Accept(writer);

			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectFailed", "message": %s, "extra": %s })", sbuffer.GetString(), metaData));

			ep.Call("connectionError", error);
		});

		netLibrary->OnConnectionProgress.Connect([] (const std::string& message, int progress, int totalProgress, bool cancelable)
		{
			console::Printf("no_console", "OnConnectionProgress: %s\n", message);

			rapidjson::Document document;
			document.SetObject();
			document.AddMember("message", rapidjson::Value(message.c_str(), message.size(), document.GetAllocator()), document.GetAllocator());
			document.AddMember("count", progress, document.GetAllocator());
			document.AddMember("total", totalProgress, document.GetAllocator());
			document.AddMember("cancelable", cancelable, document.GetAllocator());

			rapidjson::StringBuffer sbuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

			document.Accept(writer);

			if (nui::HasMainUI())
			{
				nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectStatus", "data": %s })", sbuffer.GetString()));
			}

			ep.Call("connectionProgress", message, progress, totalProgress);
		});

		netLibrary->OnConnectionCardPresent.Connect([](const std::string& card, const std::string& token)
		{
			g_cardConnectionToken = token;

			rapidjson::Document document;
			document.SetObject();
			document.AddMember("card", rapidjson::Value(card.c_str(), card.size(), document.GetAllocator()), document.GetAllocator());

			rapidjson::StringBuffer sbuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

			document.Accept(writer);

			if (nui::HasMainUI())
			{
				nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectCard", "data": %s })", sbuffer.GetString()));
			}

			ep.Call("connectionError", std::string("Cards don't exist here yet!"));
		});

		netLibrary->OnStateChanged.Connect([](NetLibrary::ConnectionState currentState, NetLibrary::ConnectionState previousState)
		{
			ep.Call("connectionStateChanged", (int)currentState, (int)previousState);
		});

		static std::function<void()> finishConnectCb;
		static bool gameUnloaded;

		netLibrary->OnInterceptConnection.Connect([](const std::string& url, const std::function<void()>& cb)
		{
			if (Instance<ICoreGameInit>::Get()->GetGameLoaded() || Instance<ICoreGameInit>::Get()->HasVariable("killedGameEarly"))
			{
				if (!gameUnloaded)
				{
					netLibrary->OnConnectionProgress("Waiting for game to shut down...", 0, 100, true);

					finishConnectCb = cb;

					return false;
				}
			}
			else
			{
				gameUnloaded = false;
				ep.Call("unloading");
			}

			return true;
		});

		Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([]()
		{
			gameUnloaded = false;
			ep.Call("unloading");
		});

		Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
		{
			if (finishConnectCb && g_connected)
			{
				auto cb = std::move(finishConnectCb);
				cb();
			}
			else
			{
				gameUnloaded = true;
				ep.Call("unloaded");
			}
		}, 5000);

		lib->AddPacketHandler<PaymentRequestPacketHandler>(true);
	});

	OnMainGameFrame.Connect([]()
	{
		if (g_onYesCallback)
		{
			int result = atoi(console::GetDefaultContext()->GetVariableManager()->FindEntryRaw("warningMessageResult")->GetValue().c_str());

			if (result != 0)
			{
				if (result == 4)
				{
					g_onYesCallback();
				}

				g_onYesCallback = {};
			}
		}
	});

	OnKillNetwork.Connect([](const char*)
	{
		g_connected = false;

		SetShellIcon("");
	});

	if (launch::IsSDKGuest())
	{
		std::string sdkRootPath = getenv("CitizenFX_SDK_rootPath");

		if (sdkRootPath.empty() || sdkRootPath == "built-in")
		{
			vfs::Mount(new vfs::RelativeDevice(ToNarrow(MakeRelativeCitPath(L"citizen/sdk/sdk-root/"))), "sdk-root:/");
		}
		else
		{
			vfs::Mount(new vfs::RelativeDevice(sdkRootPath + "/"), "sdk-root:/");
		}

		fx::ScriptEngine::RegisterNativeHandler("SEND_SDK_MESSAGE", [](fx::ScriptContext& context)
		{
			ep.Call("sdk:message", std::string(context.GetArgument<const char*>(0)));
		});

		fx::ScriptEngine::RegisterNativeHandler("SEND_SDK_MESSAGE_TO_BACKEND", [](fx::ScriptContext& context)
		{
			ep.Call("sdk:backendMessage", std::string(context.GetArgument<const char*>(0)));
		});

		console::CoreAddPrintListener([](ConsoleChannel channel, const char* msg)
		{
			ep.Call("sdk:consoleMessage", channel, std::string(msg));
		});

		ep.Bind("sdk:clientEvent", [](const std::string& eventName, const std::string& payload)
		{
			fwRefContainer<fx::ResourceManager> resman = Instance<fx::ResourceManager>::Get();
			fwRefContainer<fx::ResourceEventManagerComponent> resevman = resman->GetComponent<fx::ResourceEventManagerComponent>();

			resevman->QueueEvent2(eventName, {}, payload);
		});

#ifdef GTA_FIVE
		OnRefreshArchetypesCollectionDone.Connect([]()
		{
			ep.Call("sdk:refreshArchetypesCollectionDone");
		});

		ep.Bind("sdk:refreshArchetypesCollection", []()
		{
			OnRefreshArchetypesCollection();
		});
#endif
	}

	static ConsoleCommand connectCommand("connect", [](const std::string& server)
	{
		ConnectTo(server);
	});

	ep.Bind("connectTo", [](const std::string& url)
	{
		ConnectTo(url);
	});

	ep.Bind("charInput", [](uint32_t ch)
	{
		bool p = true;
		LRESULT r;

		InputHook::DeprecatedOnWndProc(NULL, WM_CHAR, ch, 0, p, r);
	});

	ep.Bind("imeCommitText", [](const std::string& u8str, int rS, int rE, int p)
	{
		auto b = nui::GetFocusBrowser();

		if (b)
		{
			b->GetHost()->ImeCommitText(ToWide(u8str), CefRange(rS, rE), p);
		}
	});

	ep.Bind("imeSetComposition", [](const std::string& u8str, const std::vector<std::string>& underlines, int rS, int rE, int cS, int cE)
	{
		auto b = nui::GetFocusBrowser();

		if (b)
		{
			std::vector<CefCompositionUnderline> uls;

			for (auto& ul : underlines)
			{
				uls.push_back(*reinterpret_cast<const CefCompositionUnderline*>(ul.c_str()));
			}

			b->GetHost()->ImeSetComposition(ToWide(u8str), uls, CefRange(rS, rE), CefRange(cS, cE));
		}
	});

	ep.Bind("imeCancelComposition", []()
	{
		auto b = nui::GetFocusBrowser();

		if (b)
		{
			b->GetHost()->ImeCancelComposition();
		}
	});

	ep.Bind("resizeWindow", [](int w, int h)
	{
		auto wnd = CoreGetGameWindow();

		SetWindowPos(wnd, NULL, 0, 0, w, h, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_ASYNCWINDOWPOS);
	});

	ep.Bind("sdk:invokeNative", [](const std::string& nativeType, const std::string& argumentData)
	{
		if (nativeType == "sendCommand")
		{
			se::ScopedPrincipal ps{ se::Principal{"system.console"} };
			console::GetDefaultContext()->ExecuteSingleCommand(argumentData);
		}
	});

	ep.Bind("disconnect", []()
	{
		if (netLibrary->GetConnectionState() != 0)
		{
			fwRefContainer<fx::ResourceManager> resman = Instance<fx::ResourceManager>::Get();
			fwRefContainer<fx::ResourceEventManagerComponent> resevman = resman->GetComponent<fx::ResourceEventManagerComponent>();

			resevman->TriggerEvent2("disconnecting", {});

			OnKillNetwork("Disconnected.");
			OnMsgConfirm();
		}
	});

	static ConsoleCommand disconnectCommand("disconnect", []()
	{
		DisconnectCmd();
	});

	static std::string curChannel;

	wchar_t resultPath[1024];

	static std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
	GetPrivateProfileString(L"Game", L"UpdateChannel", L"production", resultPath, std::size(resultPath), fpath.c_str());

	curChannel = ToNarrow(resultPath);

	static ConVar<bool> uiPremium("ui_premium", ConVar_None, false);

	// ConVar_ScriptRestricted because update channel is often misused as a marker for other things
	static ConVar<std::string> uiUpdateChannel("ui_updateChannel", ConVar_ScriptRestricted, curChannel,
	[](internal::ConsoleVariableEntry<std::string>* convar)
	{
		if (convar->GetValue() != curChannel)
		{
			curChannel = convar->GetValue();

			WritePrivateProfileString(L"Game", L"UpdateChannel", ToWide(curChannel).c_str(), fpath.c_str());

			rapidjson::Document document;
			document.SetString("Restart the game to apply the update channel change.", document.GetAllocator());

			rapidjson::StringBuffer sbuffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

			document.Accept(writer);

			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "setWarningMessage", "message": %s })", sbuffer.GetString()));
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
		static std::string lastHostName;

		if (!_wcsicmp(type, L"getFavorites"))
		{
			UpdatePendingAuthPayload();
		}
		else if (!_wcsicmp(type, L"backfillEnable"))
		{
			mpMenuExpectsBackfill = true;
		}
		else if (!_wcsicmp(type, L"backfillDone"))
		{
			auto ev = std::move(backfillDoneEvent);

			if (ev)
			{
				ev();
			}
		}
		else if (!_wcsicmp(type, L"getMinModeInfo"))
		{
			static bool done = ([]
			{
#ifdef GTA_FIVE
				std::thread([]
				{
					UiDone();

					auto hWnd = CoreGetGameWindow();
					ShowWindow(hWnd, SW_SHOW);

					// game code locks it
					LockSetForegroundWindow(LSFW_UNLOCK);
					SetForegroundWindow(hWnd);
				})
				.detach();
#endif

				MarkNuiLoaded();

				return true;
			})();

			auto manifest = CoreGetMinModeManifest();

			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "setMinModeInfo", "enabled": %s, "data": %s })", manifest->IsEnabled() ? "true" : "false", manifest->GetRaw()));

			static bool initSwitched;

			if (wcsstr(GetCommandLineW(), L"-switchcl") && !initSwitched)
			{
				nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "setSwitchCl", "enabled": %s })", true));
				initSwitched = true;
			}
		}
		else if (!_wcsicmp(type, L"reconnect"))
		{
			ConnectTo(lastHostName, true);
		}
		else if (!_wcsicmp(type, L"connectTo"))
		{
			std::string hostName = ToNarrow(arg);

			try
			{
				auto j = nlohmann::json::parse(hostName);
				hostName = j[0].get<std::string>();

				if (j.size() >= 2)
				{
					g_connectNonce = j[1].get<std::string>();
				}
			}
			catch (...)
			{
			}

			lastHostName = hostName;
			ConnectTo(hostName, true);
		}
		else if (!_wcsicmp(type, L"cancelDefer"))
		{
			if (!XBR_InterceptCancelDefer())
			{
				netLibrary->CancelDeferredConnection();
			}
			netLibrary->Disconnect();

			g_connected = false;
		}
		else if (_wcsicmp(type, L"executeCommand") == 0)
		{
			if (!nui::HasMainUI())
			{
				return;
			}

			se::ScopedPrincipal principal{
				se::Principal{
				"system.console" }
			};
			console::GetDefaultContext()->ExecuteSingleCommand(ToNarrow(arg));
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
		else if (!_wcsicmp(type, L"setLocale"))
		{
			if (nui::HasMainUI())
			{
				CoreGetLocalization()->SetLocale(ToNarrow(arg));
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
			if (!arg || !arg[0])
			{
				trace("Failed to set nickname\n");
				return;
			}

			std::string newusername = ToNarrow(arg);
			SetNickname(newusername);
		}
		else if (!_wcsicmp(type, L"exit"))
		{
			// queue an ExitProcess on the next game frame
			GetEarlyGameFrame().Connect([]()
			{
				AddVectoredExceptionHandler(FALSE, TerminateInstantly);

				CefShutdown();

				TerminateProcess(GetCurrentProcess(), 0);
			});
		}
		else if (!_wcsicmp(type, L"setDiscourseIdentity"))
		{
			try
			{
				auto json = nlohmann::json::parse(ToNarrow(arg));

				g_discourseUserToken = json.value<std::string>("token", "");
				g_discourseClientId = json.value<std::string>("clientId", "");

				Instance<ICoreGameInit>::Get()->SetData("discourseUserToken", g_discourseUserToken);
				Instance<ICoreGameInit>::Get()->SetData("discourseClientId", g_discourseClientId);

				Instance<::HttpClient>::Get()->DoPostRequest(
					CNL_ENDPOINT "api/validate/discourse",
					{
						{ "entitlementId", ros::GetEntitlementSource() },
						{ "authToken", g_discourseUserToken },
						{ "clientId", g_discourseClientId },
					},
					[](bool success, const char* data, size_t size)
				{
					if (success)
					{
						std::string response{ data, size };

						bool hasEndUserPremium = false;

						try
						{
							auto json = nlohmann::json::parse(response);

							for (const auto& group : json["user"]["groups"])
							{
								auto name = group.value<std::string>("name", "");

								if (name == "staff" || name == "patreon_enduser")
								{
									hasEndUserPremium = true;
									break;
								}
							}
						}
						catch (const std::exception& e)
						{

						}

						if (hasEndUserPremium)
						{
							uiPremium.GetHelper()->SetRawValue(true);
							Instance<ICoreGameInit>::Get()->SetVariable("endUserPremium");
						}
					}
				});
			}
			catch (const std::exception& e)
			{
				trace("failed to set discourse identity: %s\n", e.what());
			}
		}
		else if (!_wcsicmp(type, L"submitCardResponse"))
		{
			try
			{
				auto json = nlohmann::json::parse(ToNarrow(arg));

				if (!XBR_InterceptCardResponse(json))
				{
					if (!g_cardConnectionToken.empty())
					{
						netLibrary->SubmitCardResponse(json["data"].dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace), g_cardConnectionToken);
					}
				}
			}
			catch (const std::exception& e)
			{
				trace("failed to set card response: %s\n", e.what());
			}
		}
		else if (!_wcsicmp(type, L"setLastServers"))
		{
			try
			{
				auto json = nlohmann::json::parse(ToNarrow(arg));

				int start = json.size() > 15 ? json.size() - 15 : 0;
				int end = json.size();

				std::vector<ServerLink> links;

				for (int i = end - 1; i >= start; i--)
				{
					if (json[i].is_null() || json[i]["hostname"].is_null() || json[i]["address"].is_null())
					{
						continue;
					}

					ServerLink l;
					json[i]["hostname"].get_to(l.hostname);

					if (!json[i]["rawIcon"].is_null())
					{
						json[i]["rawIcon"].get_to(l.rawIcon);
					}

					json[i]["address"].get_to(l.url);
					l.vars = json[i]["vars"];

					if (l.url.find("cfx.re/join/") == 0)
					{
						l.url = "-" + l.url.substr(12);
					}

					links.push_back(std::move(l));
				}

				UpdateJumpList(links);
			}
			catch (const std::exception & e)
			{
				trace("failed to set last servers: %s\n", e.what());
			}
		}
	});

	GetEarlyGameFrame().Connect([]()
	{
		static bool hi;

		if (!hi)
		{
			ep.Call("hi");
			hi = true;
		}

		se::ScopedPrincipal scope(se::Principal{ "system.console" });
		Instance<console::Context>::Get()->ExecuteBuffer();
	});

	OnMsgConfirm.Connect([] ()
	{
		ep.Call("disconnected");

		nui::SetMainUI(true);
		nui::SwitchContext("");

		nui::CreateFrame("mpMenu", console::GetDefaultContext()->GetVariableManager()->FindEntryRaw("ui_url")->GetValue());
	});
});

#ifndef GTA_NY
#include <gameSkeleton.h>
#endif
#include <shellapi.h>

static void ProtocolRegister(const wchar_t* name, const wchar_t* cls)
{
	LSTATUS result;

#define CHECK_STATUS(x) \
	result = (x); \
	if (result != ERROR_SUCCESS) { \
		trace("[Protocol Registration] " #x " failed: %x", result); \
		return; \
	}

	static HostSharedData<CfxState> hostData("CfxInitState");

	HKEY key = NULL;
	std::wstring command = fmt::sprintf(L"\"%s\" \"%%1\"", hostData->gameExePath);

	const auto create_key = [&key](std::wstring name)
	{
		return RegCreateKeyW(HKEY_CURRENT_USER, name.c_str(), &key);
	};

	const auto set_string = [&key](const wchar_t* name, std::wstring value)
	{
		return RegSetValueExW(key, name, 0, REG_SZ, (const BYTE*)value.c_str(), (value.size() + 1) * sizeof(wchar_t));
	};

	CHECK_STATUS(create_key(fmt::sprintf(L"SOFTWARE\\Classes\\%s", cls)));
	CHECK_STATUS(set_string(NULL, name));
	CHECK_STATUS(set_string(L"URL Protocol", L""));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(create_key(fmt::sprintf(L"SOFTWARE\\Classes\\%s.ProtocolHandler", name)));
	CHECK_STATUS(set_string(NULL, name));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(create_key(fmt::sprintf(L"SOFTWARE\\%s", name)));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(create_key(fmt::sprintf(L"SOFTWARE\\%s\\Capabilities", name)));
	CHECK_STATUS(set_string(L"ApplicationName", name));
	CHECK_STATUS(set_string(L"ApplicationDescription", name));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(create_key(fmt::sprintf(L"SOFTWARE\\%s\\Capabilities\\URLAssociations", name)));
	CHECK_STATUS(set_string(cls, fmt::sprintf(L"%s.ProtocolHandler", name)));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(create_key(L"SOFTWARE\\RegisteredApplications"));
	CHECK_STATUS(set_string(name, fmt::sprintf(L"Software\\%s\\Capabilities", name)));
	CHECK_STATUS(RegCloseKey(key));

	CHECK_STATUS(create_key(fmt::sprintf(L"SOFTWARE\\Classes\\%s.ProtocolHandler\\shell\\open\\command", name)));
	CHECK_STATUS(set_string(NULL, command));
	CHECK_STATUS(RegCloseKey(key));

	if (!IsWindows8Point1OrGreater())
	{
		// these are for compatibility on downlevel Windows systems
		CHECK_STATUS(create_key(fmt::sprintf(L"SOFTWARE\\Classes\\%s\\shell\\open\\command", cls)));
		CHECK_STATUS(set_string(NULL, command));
		CHECK_STATUS(RegCloseKey(key));
	}
}

void Component_RunPreInit()
{
	static HostSharedData<CfxState> hostData("CfxInitState");

#ifndef _DEBUG
	if (hostData->IsGameProcess())
#else
	if (hostData->IsMasterProcess())
#endif
	{
		ProtocolRegister(PRODUCT_NAME, hostData->GetLinkProtocol());
	}

	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

	static std::string connectHost;
	static std::string connectParams;
	static std::string authPayload;

	static auto protocolLinkStart = ToNarrow(hostData->GetLinkProtocol(L":"));

	for (int i = 1; i < argc; i++)
	{
		std::string arg = ToNarrow(argv[i]);

		if (arg.find(protocolLinkStart) == 0)
		{
			auto parsed = skyr::make_url(arg);

			if (parsed)
			{
				if (!parsed->host().empty())
				{
					if (parsed->host() == "connect")
					{
						if (!parsed->pathname().empty())
						{
							connectHost = parsed->pathname().substr(1);
							const auto& search = parsed->search_parameters();
							if (!search.empty())
							{
								connectParams = search.to_string();
							}
						}
					}
					else if (parsed->host() == "accept-auth")
					{
						if (!parsed->search().empty())
						{
							authPayload = parsed->search().substr(1);
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
// #TODOLIBERTY: ?
#ifndef GTA_NY
			rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
			{
				if (type == rage::InitFunctionType::INIT_CORE)
				{
					ConnectTo(connectHost, false, connectParams);
					connectHost = "";
					connectParams = "";
				}
			}, 999999);
#endif
		}
		else
		{
			auto j = nlohmann::json::object({ { "host", connectHost }, { "params", connectParams } });
			std::string connectMsg = j.dump(-1, ' ', false, nlohmann::detail::error_handler_t::strict);

			cfx::glue::LinkProtocolIPC::SendConnectTo(connectMsg);

			if (!hostData->gamePid)
			{
				AllowSetForegroundWindow(hostData->GetInitialPid());
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
// #TODOLIBERTY: ?
#ifndef GTA_NY
			rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
			{
				if (type == rage::InitFunctionType::INIT_CORE)
				{
					HandleAuthPayload(authPayload);
					authPayload = "";
				}
			}, 999999);
#endif
		}
		else
		{
			cfx::glue::LinkProtocolIPC::SendAuthPayload(authPayload);

			if (!hostData->gamePid)
			{
				AllowSetForegroundWindow(hostData->GetInitialPid());
			}
			else
			{
				AllowSetForegroundWindow(hostData->gamePid);
			}

			TerminateProcess(GetCurrentProcess(), 0);
		}
	}
}

#if __has_include(<gameSkeleton.h>)
static InitFunction buildSaverInitFunction([]() {
	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_BEFORE_MAP_LOADED)
		{
			SaveBuildNumber(xbr::GetRequestedGameBuild());
		}
	});
});
#endif

static InitFunction linkProtocolIPCInitFunction([]()
{
	// Only run LinkProtocolIPC in the game process
	if (!CfxState::Get()->IsGameProcess())
	{
		return;
	}

	cfx::glue::LinkProtocolIPC::Initialize();

	cfx::glue::LinkProtocolIPC::OnConnectTo.Connect([](const std::string_view& connectMsg)
	{
		auto connectData = nlohmann::json::parse(connectMsg);
		ConnectTo(connectData["host"], false, connectData["params"]);

		SetForegroundWindow(CoreGetGameWindow());
	});

	cfx::glue::LinkProtocolIPC::OnAuthPayload.Connect([](const std::string_view& authPayload)
	{
		HandleAuthPayload(std::string(authPayload));

		SetForegroundWindow(CoreGetGameWindow());
	});

	GetEarlyGameFrame().Connect([]()
	{
		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}

		cfx::glue::LinkProtocolIPC::ProcessMessages();
	});
});
