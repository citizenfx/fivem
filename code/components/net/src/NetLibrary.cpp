/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetLibrary.h"
#include <base64.h>
#include "ICoreGameInit.h"
#include <mutex>
#include <mmsystem.h>
#include <yaml-cpp/yaml.h>
#include <SteamComponentAPI.h>
#include <LegitimacyAPI.h>
#include <IteratorView.h>

#include <json.hpp>

#include <Error.h>

fwEvent<const std::string&> OnRichPresenceSetTemplate;
fwEvent<int, const std::string&> OnRichPresenceSetValue;

std::unique_ptr<NetLibraryImplBase> CreateNetLibraryImplV2(INetLibraryInherit* base);

#define TIMEOUT_DATA_SIZE 16

static uint32_t g_runFrameTicks[TIMEOUT_DATA_SIZE];
static uint32_t g_receiveDataTicks[TIMEOUT_DATA_SIZE];
static uint32_t g_sendDataTicks[TIMEOUT_DATA_SIZE];

static void AddTimeoutTick(uint32_t* timeoutList)
{
	memmove(&timeoutList[0], &timeoutList[1], (TIMEOUT_DATA_SIZE - 1) * sizeof(uint32_t));
	timeoutList[TIMEOUT_DATA_SIZE - 1] = timeGetTime();
}

static std::string CollectTimeoutInfo()
{
	uint32_t begin = timeGetTime();

	auto gatherInfo = [begin](uint32_t* list) -> std::string
	{
		std::stringstream ss;

		for (int i = 0; i < TIMEOUT_DATA_SIZE; i++)
		{
			ss << fmt::sprintf("-%d ", begin - list[i]);
		}

		return ss.str();
	};

	return fmt::sprintf(
		"DEBUG INFO FOR TIMEOUTS:\nrun frame: %s\nreceive data: %s\nsend data: %s",
		gatherInfo(g_runFrameTicks),
		gatherInfo(g_receiveDataTicks),
		gatherInfo(g_sendDataTicks)
	);
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

void NetLibrary::AddReceiveTick()
{
	AddTimeoutTick(g_receiveDataTicks);
}

void NetLibrary::AddSendTick()
{
	AddTimeoutTick(g_sendDataTicks);
}

static uint32_t m_tempGuid = GetTickCount();

uint16_t NetLibrary::GetServerNetID()
{
	return m_serverNetID;
}

uint16_t NetLibrary::GetServerSlotID()
{
	return m_serverSlotID;
}

uint16_t NetLibrary::GetHostNetID()
{
	return m_hostNetID;
}

void NetLibrary::HandleConnected(int serverNetID, int hostNetID, int hostBase, int slotID, uint64_t serverTime)
{
	m_serverNetID = serverNetID;
	m_hostNetID = hostNetID;
	m_hostBase = hostBase;
	m_serverSlotID = slotID;
	m_serverTime = serverTime;

	trace("connectOK, our id %d (slot %d), host id %d\n", m_serverNetID, m_serverSlotID, m_hostNetID);

	OnConnectOKReceived(m_currentServer);

	m_connectionState = CS_CONNECTED;
}

bool NetLibrary::GetOutgoingPacket(RoutingPacket& packet)
{
	return m_outgoingPackets.try_pop(packet);
}

bool NetLibrary::WaitForRoutedPacket(uint32_t timeout)
{
	{
		std::lock_guard<std::mutex> guard(m_incomingPacketMutex);

		if (!m_incomingPackets.empty())
		{
			return true;
		}
	}

	WaitForSingleObject(m_receiveEvent, timeout);

	{
		std::lock_guard<std::mutex> guard(m_incomingPacketMutex);

		return (!m_incomingPackets.empty());
	}
}

void NetLibrary::EnqueueRoutedPacket(uint16_t netID, const std::string& packet)
{
	{
		std::lock_guard<std::mutex> guard(m_incomingPacketMutex);

		RoutingPacket routePacket;
		routePacket.netID = netID;
		routePacket.payload = std::move(packet);
		routePacket.genTime = timeGetTime();

		m_incomingPackets.push(std::move(routePacket));
	}

	SetEvent(m_receiveEvent);
}

bool NetLibrary::DequeueRoutedPacket(char* buffer, size_t* length, uint16_t* netID)
{
	{
		std::lock_guard<std::mutex> guard(m_incomingPacketMutex);

		if (m_incomingPackets.empty())
		{
			return false;
		}

		auto packet = m_incomingPackets.front();
		m_incomingPackets.pop();

		memcpy(buffer, packet.payload.c_str(), packet.payload.size());
		*netID = packet.netID;
		*length = packet.payload.size();

		// store metrics
		auto timeval = (timeGetTime() - packet.genTime);

		m_metricSink->OnRouteDelayResult(timeval);
	}

	ResetEvent(m_receiveEvent);

	return true;
}

void NetLibrary::RoutePacket(const char* buffer, size_t length, uint16_t netID)
{
	RoutingPacket routePacket;
	routePacket.netID = netID;
	routePacket.payload = std::string(buffer, length);

	m_outgoingPackets.push(routePacket);
}

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		  8192
#define	BIG_INFO_VALUE		8192

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char *Info_ValueForKey(const char *s, const char *key)
{
	char	pkey[BIG_INFO_KEY];
	static	char value[2][BIG_INFO_VALUE];	// use two buffers so compares
											// work without stomping on each other
	static	int	valueindex = 0;
	char	*o;

	if (!s || !key)
	{
		return "";
	}

	if (strlen(s) >= BIG_INFO_STRING)
	{
		return "";
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			*o++ = *s++;
		}
		*o = 0;

		if (!_stricmp(key, pkey))
			return value[valueindex];

		if (!*s)
			break;
		s++;
	}

	return "";
}


#define Q_IsColorString( p )  ( ( p ) && *( p ) == '^' && *( ( p ) + 1 ) && isdigit( *( ( p ) + 1 ) ) ) // ^[0-9]

void StripColors(const char* in, char* out, int max)
{
	max--; // \0
	int current = 0;
	while (*in != 0 && current < max)
	{
		if (!Q_IsColorString(in))
		{
			*out = *in;
			out++;
			current++;
		}
		else
		{
			*in++;
		}
		*in++;
	}
	*out = '\0';
}

void NetLibrary::ProcessOOB(const NetAddress& from, const char* oob, size_t length)
{
	if (from == m_currentServer)
	{
		if (!_strnicmp(oob, "infoResponse", 12))
		{
			const char* infoString = &oob[13];

			m_infoString = infoString;

			{
				auto steam = GetSteam();

				char hostname[256] = { 0 };
				strncpy(hostname, Info_ValueForKey(infoString, "hostname"), 255);

				char cleaned[256];

				StripColors(hostname, cleaned, 256);

#ifdef GTA_FIVE
				SetWindowText(FindWindow(L"grcWindow", nullptr), va(L"FiveM - %s", ToWide(cleaned)));
#endif

				auto richPresenceSetTemplate = [&](const auto& tpl)
				{
					OnRichPresenceSetTemplate(tpl);

					if (steam)
					{
						steam->SetRichPresenceTemplate(tpl);
					}
				};

				auto richPresenceSetValue = [&](int idx, const std::string& val)
				{
					OnRichPresenceSetValue(idx, val);

					if (steam)
					{
						steam->SetRichPresenceValue(idx, val);
					}
				};

				richPresenceSetTemplate("{0}\n{1}");

				richPresenceSetValue(0, fmt::sprintf(
					"%s%s",
					std::string(cleaned).substr(0, 110),
					(strlen(cleaned) > 110) ? "..." : ""
				));

				richPresenceSetValue(1, "Connecting...");
			}

			// until map reloading is in existence
			std::string thisWorld = Info_ValueForKey(infoString, "world");

			if (thisWorld.empty())
			{
				thisWorld = "gta5";
			}

			static std::string lastWorld = thisWorld;

			if (lastWorld != thisWorld && Instance<ICoreGameInit>::Get()->GetGameLoaded())
			{
				GlobalError("Was loaded in world %s, but this server is world %s. Restart the game to join.", lastWorld, thisWorld);
				return;
			}

			lastWorld = thisWorld;

			// finalize connecting
			m_connectionState = CS_CONNECTING;
			m_lastConnect = 0;
			m_connectAttempts = 0;
		}
		else if (!_strnicmp(oob, "error", 5))
		{
			if (from != m_currentServer)
			{
				trace("Received 'error' request was not from the host\n");
				return;
			}

			if (length >= 6)
			{
				const char* errorStr = &oob[6];
				auto errText = std::string(errorStr, length - 6);

				if (strstr(errorStr, "Timed out") != nullptr)
				{
					errText += fmt::sprintf("\n%s", CollectTimeoutInfo());
				}

				GlobalError("%s", errText);
			}
		}
	}
}

void NetLibrary::SetHost(uint16_t netID, uint32_t base)
{
	m_hostNetID = netID;
	m_hostBase = base;
}

void NetLibrary::SetBase(uint32_t base)
{
	m_serverBase = base;
}

uint32_t NetLibrary::GetHostBase()
{
	return m_hostBase;
}

void NetLibrary::SetMetricSink(fwRefContainer<INetMetricSink>& sink)
{
	m_metricSink = sink;
}

void NetLibrary::HandleReliableCommand(uint32_t msgType, const char* buf, size_t length)
{
	auto range = m_reliableHandlers.equal_range(msgType);

	for (auto& handlerPair : fx::GetIteratorView(range))
	{
		auto [handler, runOnMainFrame] = handlerPair.second;

		if (runOnMainFrame)
		{
			auto server = m_currentServerPeer;
			net::Buffer netBuf(reinterpret_cast<const uint8_t*>(buf), length);

			m_mainFrameQueue.push([this, netBuf, handler, server]()
			{
				if (server != m_currentServerPeer)
				{
					trace("Ignored a network packet enqueued before reconnection.\n");
					return;
				}

				handler(reinterpret_cast<const char*>(netBuf.GetBuffer()), netBuf.GetLength());
			});
		}
		else
		{
			handler(buf, length);
		}
	}
}

RoutingPacket::RoutingPacket()
{
	//genTime = timeGetTime();
	genTime = 0;
}

void NetLibrary::SendReliableCommand(const char* type, const char* buffer, size_t length)
{
	m_impl->SendReliableCommand(HashRageString(type), buffer, length);
}

static std::string g_disconnectReason;

static std::mutex g_netFrameMutex;

inline uint64_t GetGUID()
{
	auto steamComponent = GetSteam();

	if (steamComponent)
	{
		IClientEngine* steamClient = steamComponent->GetPrivateClient();

		InterfaceMapper steamUser(steamClient->GetIClientUser(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTUSER_INTERFACE_VERSION001"));

		if (steamUser.IsValid())
		{
			uint64_t steamID;
			steamUser.Invoke<void>("GetSteamID", &steamID);

			return steamID;
		}
	}

	return (uint64_t)(0x210000100000000 | m_tempGuid);
}

void NetLibrary::RunMainFrame()
{
	std::function<void()> cb;

	while (m_mainFrameQueue.try_pop(cb))
	{
		cb();
	}
}

void NetLibrary::RunFrame()
{
	if (!g_netFrameMutex.try_lock())
	{
		return;
	}

	AddTimeoutTick(g_runFrameTicks);

	if (m_connectionState != m_lastConnectionState)
	{
		OnStateChanged(m_connectionState, m_lastConnectionState);

		m_lastConnectionState = m_connectionState;
	}

	if (m_impl)
	{
		m_impl->RunFrame();
	}

	switch (m_connectionState)
	{
		case CS_INITRECEIVED:
			// change connection state to CS_DOWNLOADING
			m_connectionState = CS_DOWNLOADING;

			// trigger task event
			OnConnectionProgress("Downloading content", 0, 1);
			OnInitReceived(m_currentServer);

			break;

		case CS_DOWNLOADCOMPLETE:
			m_connectionState = CS_FETCHING;
			m_lastConnect = 0;
			m_connectAttempts = 0;

			OnConnectionProgress("Downloading completed", 1, 1);

			break;

		case CS_FETCHING:
			if ((GetTickCount() - m_lastConnect) > 5000)
			{
				SendOutOfBand(m_currentServer, "getinfo xyz");

				m_lastConnect = GetTickCount();

				m_connectAttempts++;

				// advertise status
				auto specStatus = (m_connectAttempts > 1) ? fmt::sprintf(" (attempt %d)", m_connectAttempts) : "";

				OnConnectionProgress(fmt::sprintf("Fetching info from server...%s", specStatus), 1, 1);
			}

			if (m_connectAttempts > 3)
			{
				g_disconnectReason = "Fetching info timed out.";
				FinalizeDisconnect();

				OnConnectionTimedOut();

				GlobalError("Failed to getinfo server after 3 attempts.");
			}
			break;

		case CS_CONNECTING:
			if ((GetTickCount() - m_lastConnect) > 5000)
			{
				m_impl->SendConnect(fmt::sprintf("token=%s&guid=%llu", m_token, (uint64_t)GetGUID()));

				m_lastConnect = GetTickCount();

				m_connectAttempts++;

				// advertise status
				auto specStatus = (m_connectAttempts > 1) ? fmt::sprintf(" (attempt %d)", m_connectAttempts) : "";

				OnConnectionProgress(fmt::sprintf("Connecting to server...%s", specStatus), 1, 1);
			}

			if (m_connectAttempts > 3)
			{
				g_disconnectReason = "Connection timed out.";
				FinalizeDisconnect();

				OnConnectionTimedOut();

				GlobalError("Failed to connect to server after 3 attempts.");
			}

			break;

		case CS_ACTIVE:
			if (m_impl->HasTimedOut())
			{
				g_disconnectReason = "Connection timed out.";

				OnConnectionTimedOut();

				GlobalError("Server connection timed out after 15 seconds.\n%s", CollectTimeoutInfo());

				m_connectionState = CS_IDLE;
				m_currentServer = NetAddress();
			}

			break;
	}

	g_netFrameMutex.unlock();
}

void NetLibrary::Death()
{
	g_netFrameMutex.unlock();
}

void NetLibrary::Resurrection()
{
	g_netFrameMutex.lock();
}

static void tohex(unsigned char* in, size_t insz, char* out, size_t outsz)
{
    unsigned char* pin = in;
    const char* hex = "0123456789ABCDEF";
    char* pout = out;
    for (; pin < in + insz; pout += 2, pin++)
    {
        pout[0] = hex[(*pin >> 4) & 0xF];
        pout[1] = hex[*pin & 0xF];
        if (pout + 3 - out > outsz)
        {
            break;
        }
    }
    pout[0] = 0;
}

typedef uint32 HAuthTicket;
const HAuthTicket k_HAuthTicketInvalid = 0;

struct GetAuthSessionTicketResponse_t
{
	enum { k_iCallback = 100 + 63 };
	HAuthTicket m_hAuthTicket;
	int m_eResult;
};

void NetLibrary::ConnectToServer(const net::PeerAddress& address)
{
	if (m_connectionState != CS_IDLE)
	{
		Disconnect("Connecting to another server.");
		FinalizeDisconnect();
	}

	// late-initialize error state in ICoreGameInit
	// this happens here so it only tries capturing if connection was attempted
	static struct ErrorState 
	{
		ErrorState(NetLibrary* lib)
		{
			Instance<ICoreGameInit>::Get()->OnTriggerError.Connect([=] (const std::string& errorMessage)
			{
				if (lib->m_connectionState != CS_ACTIVE)
				{
					lib->OnConnectionError(errorMessage.c_str());

					lib->m_connectionState = CS_IDLE;

					return false;
				}
				else if (lib->m_connectionState != CS_IDLE)
				{
					auto nlPos = errorMessage.find_first_of('\n');

					if (nlPos == std::string::npos || nlPos > 100)
					{
						nlPos = 100;
					}

					lib->Disconnect(errorMessage.substr(0, nlPos).c_str());

					if (!Instance<ICoreGameInit>::Get()->GetGameLoaded())
					{
						lib->FinalizeDisconnect();
					}
				}

				return true;
			});
		}
	} es(this);

	m_currentServer = NetAddress(address.GetSocketAddress());
	m_currentServerPeer = address;
	m_connectionState = CS_INITING;

	AddCrashometry("last_server", "%s", address.ToString());

	if (m_impl)
	{
		m_impl->Reset();
	}

	m_outSequence = 0;

	static fwMap<fwString, fwString> postMap;
	postMap["method"] = "initConnect";
	postMap["name"] = GetPlayerName();
	postMap["protocol"] = va("%d", NETWORK_PROTOCOL);

	static std::function<void()> performRequest;

	postMap["guid"] = va("%lld", GetGUID());

	static bool isLegacyDeferral;
	isLegacyDeferral = false;

	static fwAction<bool, const char*, size_t> handleAuthResult;
	handleAuthResult = [=] (bool result, const char* connDataStr, size_t size) mutable
	{
		if (m_connectionState != CS_INITING)
		{
			return;
		}

		std::string connData(connDataStr, size);

		if (!result)
		{
			// TODO: add UI output
			m_connectionState = CS_IDLE;

			//nui::ExecuteRootScript("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connectFailed', message: 'General handshake failure.' }, '*');");
			OnConnectionError(va("Failed handshake to server %s:%d%s%s.", m_currentServer.GetAddress(), m_currentServer.GetPort(), connData.length() > 0 ? " - " : "", connData));

			return;
		}
		else if (!isLegacyDeferral)
		{
			OnConnectionError(va("Failed handshake to server %s:%d - it closed the connection while deferring.", m_currentServer.GetAddress(), m_currentServer.GetPort()));
		}
	};

	static std::function<bool(const std::string&)> handleAuthResultData;
	handleAuthResultData = [=](const std::string& chunk)
	{
		// FIXME: for now, assume the chunk will always be a full JSON message
		// this will not always be the case, but for initial prototyping this'll work...
		if (m_connectionState != CS_INITING)
		{
			return false;
		}

		std::string connData(chunk);

		try
		{
			auto node = YAML::Load(connData);

			if (node["token"].IsDefined())
			{
				m_token = node["token"].as<std::string>();
				Instance<ICoreGameInit>::Get()->SetData("connectionToken", m_token);
			}

			if (node["defer"].IsDefined())
			{
				if (node["deferVersion"].IsDefined())
				{
					// new deferral system
					OnConnectionProgress(node["message"].as<std::string>(), 133, 133);

					return true;
				}

				isLegacyDeferral = true;

				OnConnectionProgress(node["status"].as<std::string>(), 133, 133);

				static fwMap<fwString, fwString> newMap;
				newMap["method"] = "getDeferState";
				newMap["guid"] = va("%lld", GetGUID());
				newMap["token"] = m_token;

				HttpRequestOptions options;
				options.streamingCallback = handleAuthResultData;
				m_handshakeRequest = m_httpClient->DoPostRequest(fmt::sprintf("http://%s/client", address.ToString()), m_httpClient->BuildPostString(newMap), options, handleAuthResult);

				return true;
			}

			m_handshakeRequest = {};

			if (node["error"].IsDefined())
			{
				OnConnectionError(node["error"].as<std::string>().c_str());

				m_connectionState = CS_IDLE;

				return true;
			}

			if (!node["sH"].IsDefined())
			{
				OnConnectionError("Invalid server response from initConnect (missing JSON data), is this server running a broken resource?");
				m_connectionState = CS_IDLE;
				return true;
			}
			else
			{
				Instance<ICoreGameInit>::Get()->ShAllowed = node["sH"].as<bool>(true);
			}

			m_httpClient->DoGetRequest(fmt::sprintf("https://runtime.fivem.net/policy/shdisable?server=%s_%d", address.GetHost(), address.GetPort()), [=](bool success, const char* data, size_t length)
			{
				if (success)
				{
					if (std::string(data, length).find("yes") != std::string::npos)
					{
						Instance<ICoreGameInit>::Get()->ShAllowed = false;
					}
				}
			});

			Instance<ICoreGameInit>::Get()->EnhancedHostSupport = (node["enhancedHostSupport"].IsDefined() && node["enhancedHostSupport"].as<bool>(false));
			Instance<ICoreGameInit>::Get()->OneSyncEnabled = (node["onesync"].IsDefined() && node["onesync"].as<bool>(false));

			m_serverProtocol = node["protocol"].as<uint32_t>();

			auto steam = GetSteam();

			if (steam)
			{
				steam->SetConnectValue(fmt::sprintf("+connect %s:%d", m_currentServer.GetAddress(), m_currentServer.GetPort()));
			}

			if (Instance<ICoreGameInit>::Get()->OneSyncEnabled)
			{
				auto oneSyncFailure = [this]()
				{
					OnConnectionError("OneSync is not whitelisted for this server, or requesting whitelist status failed. You'll have to wait a little while longer!");
					m_connectionState = CS_IDLE;
				};

				auto oneSyncSuccess = [this]()
				{
					m_connectionState = CS_INITRECEIVED;
				};

				m_httpClient->DoGetRequest(fmt::sprintf("http://%s:%d/info.json", address.GetHost(), address.GetPort()), [=](bool success, const char* data, size_t size)
				{
					using json = nlohmann::json;

					if (success)
					{
						try
						{
							json info = json::parse(data, data + size);

							if (info.is_object() && info["vars"].is_object())
							{
								auto val = info["vars"].value("sv_licenseKeyToken", "");

								if (!val.empty())
								{
									m_httpClient->DoGetRequest(fmt::sprintf("https://policy-live.fivem.net/api/policy/%s/onesync", val), [=](bool success, const char* data, size_t size)
									{
										if (success)
										{
											if (std::string(data, size).find("yes") != std::string::npos)
											{
												oneSyncSuccess();

												return;
											}
										}

										oneSyncFailure();
									});

									return;
								}
							}
						}
						catch (std::exception& e)
						{
							trace("1s policy - get failed for %s\n", e.what());
						}

						oneSyncFailure();
					}
				});
			}
			else
			{
				m_connectionState = CS_INITRECEIVED;
			}

			if (node["netlibVersion"].as<int>(1) == 2)
			{
				m_impl = CreateNetLibraryImplV2(this);
			}
			else
			{
				OnConnectionError("Legacy servers are incompatible with this version of FiveM. Please tell the server owner to the server to the latest FXServer build. See https://fivem.net/ for more info.");
				m_connectionState = CS_IDLE;
				return true;
			}
		}
		catch (YAML::Exception& e)
		{
			OnConnectionError(e.what());
			m_connectionState = CS_IDLE;
		}

		return true;
	};

	performRequest = [=]()
	{
		OnConnectionProgress("Handshaking with server...", 0, 100);

		HttpRequestOptions options;
		options.streamingCallback = handleAuthResultData;

		m_handshakeRequest = m_httpClient->DoPostRequest(fmt::sprintf("http://%s/client", address.ToString()), m_httpClient->BuildPostString(postMap), options, handleAuthResult);
	};

	m_httpClient->DoGetRequest(fmt::sprintf("https://runtime.fivem.net/blacklist/%s_%d", address.GetHost(), address.GetPort()), [=](bool success, const char* data, size_t length)
	{
		if (success)
		{
			FatalError("This server has been blocked from the FiveM platform. Stated reason: %sIf you manage this server and you feel this is not justified, please contact your Technical Account Manager.", std::string(data, length));
		}
	});

	m_httpClient->DoGetRequest(fmt::sprintf("https://runtime.fivem.net/blacklist/%s", address.GetHost()), [=](bool success, const char* data, size_t length)
	{
		if (success)
		{
			FatalError("This server has been blocked from the FiveM platform. Stated reason: %sIf you manage this server and you feel this is not justified, please contact your Technical Account Manager.", std::string(data, length));
		}
	});

	auto continueRequest = [=]()
	{
		auto steamComponent = GetSteam();

		if (steamComponent)
		{
			static uint32_t ticketLength;
			static uint8_t ticketBuffer[4096];

			static int lastCallback = -1;

			IClientEngine* steamClient = steamComponent->GetPrivateClient();

			InterfaceMapper steamUtils(steamClient->GetIClientUtils(steamComponent->GetHSteamPipe(), "CLIENTUTILS_INTERFACE_VERSION001"));
			InterfaceMapper steamUser(steamClient->GetIClientUser(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTUSER_INTERFACE_VERSION001"));

			if (steamUser.IsValid())
			{
				auto removeCallback = []()
				{
					if (lastCallback != -1)
					{
						GetSteam()->RemoveSteamCallback(lastCallback);
						lastCallback = -1;
					}
				};

				removeCallback();

				lastCallback = steamComponent->RegisterSteamCallback<GetAuthSessionTicketResponse_t>([=](GetAuthSessionTicketResponse_t* response)
				{
					removeCallback();

					if (response->m_eResult != 1) // k_EResultOK
					{
						OnConnectionError(va("Failed to obtain Steam ticket, EResult %d.", response->m_eResult));
					}
					else
					{
						// encode the ticket buffer
						char outHex[16384];
						tohex(ticketBuffer, ticketLength, outHex, sizeof(outHex));

						postMap["authTicket"] = outHex;

						performRequest();
					}
				});

				int appID = steamUtils.Invoke<int>("GetAppID");

				trace("Getting auth ticket for pipe appID %d - should be 218.\n", appID);

				steamUser.Invoke<int>("GetAuthSessionTicket", ticketBuffer, (int)sizeof(ticketBuffer), &ticketLength);

				OnConnectionProgress("Obtaining Steam ticket...", 0, 100);
			}
			else
			{
				performRequest();
			}
		}
		else
		{
			performRequest();
		}
	};

	auto initiateRequest = [this, address, continueRequest]()
	{
		OnConnectionProgress("Requesting authentication ticket...", 0, 100);

		m_httpClient->DoPostRequest("https://lambda.fivem.net/api/ticket/create", { { "token", ros::GetEntitlementSource() }, { "server", address.ToString() }, { "guid", fmt::sprintf("%lld", GetGUID()) } }, [=](bool success, const char* data, size_t dataLen)
		{
			if (success)
			{
				auto node = YAML::Load(std::string(data, dataLen));

				if (node["error"].IsDefined())
				{
					OnConnectionError(va("%s", node["error"].as<std::string>()));

					m_connectionState = CS_IDLE;

					return;
				}
				else if (node["ticket"].IsDefined())
				{
					postMap["cfxTicket"] = node["ticket"].as<std::string>();
				}
			}

			continueRequest();
		});
	};

	if (OnInterceptConnection(address, initiateRequest))
	{
		initiateRequest();
	}
}

void NetLibrary::CancelDeferredConnection()
{
	if (m_handshakeRequest)
	{
		m_handshakeRequest->Abort();
		m_handshakeRequest = {};
	}

	if (m_connectionState == CS_INITING)
	{
		m_connectionState = CS_IDLE;
	}
}

void NetLibrary::Disconnect(const char* reason)
{
	g_disconnectReason = reason;

	OnAttemptDisconnect(reason);
	//GameInit::KillNetwork((const wchar_t*)1);
}

static std::mutex g_disconnectionMutex;

void NetLibrary::FinalizeDisconnect()
{
	std::unique_lock<std::mutex> lock(g_disconnectionMutex);

	if (m_connectionState == CS_CONNECTING || m_connectionState == CS_ACTIVE)
	{
		SendReliableCommand("msgIQuit", g_disconnectReason.c_str(), g_disconnectReason.length() + 1);

		m_impl->Flush();

		OnFinalizeDisconnect(m_currentServer);

		m_connectionState = CS_IDLE;
		m_currentServer = NetAddress();
	}
}

void NetLibrary::CreateResources()
{
	m_httpClient = Instance<HttpClient>::Get();
}

void NetLibrary::SendOutOfBand(const NetAddress& address, const char* format, ...)
{
	static char buffer[32768];

	*(int*)buffer = -1;

	va_list ap;
	va_start(ap, format);
	int length = _vsnprintf(&buffer[4], 32764, format, ap);
	va_end(ap);

	if (length >= 32764)
	{
		GlobalError("Attempted to overrun string in call to SendOutOfBand()!");
	}

	buffer[32767] = '\0';

	SendData(address, buffer, strlen(buffer));
}

const char* NetLibrary::GetPlayerName()
{
	/*
	ProfileManager* profileManager = Instance<ProfileManager>::Get();
	fwRefContainer<Profile> profile = profileManager->GetPrimaryProfile();*/
	if (!m_playerName.empty()) return m_playerName.c_str();

	auto steamComponent = GetSteam();

	if (steamComponent)
	{
		IClientEngine* steamClient = steamComponent->GetPrivateClient();

		if (steamClient)
		{
			InterfaceMapper steamFriends(steamClient->GetIClientFriends(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTFRIENDS_INTERFACE_VERSION001"));

			if (steamFriends.IsValid())
			{
				// TODO: name changing
				static std::string personaName = steamFriends.Invoke<const char*>("GetPersonaName");

				return personaName.c_str();
			}
		}
	}

	const char* returnName = nullptr;
	/*
	if (profile.GetRef())
	{
		returnName = profile->GetDisplayName();
	}
	else
	{
		static char computerName[64];
		DWORD nameSize = sizeof(computerName);
		GetComputerNameA(computerName, &nameSize);

		returnName = computerName;
	}*/
	returnName = getenv("USERNAME");
	if (returnName == nullptr || !returnName[0]) {
		static char computerName[64];
		DWORD nameSize = sizeof(computerName);
		GetComputerNameA(computerName, &nameSize);
		returnName = computerName;
	}
	if (returnName == nullptr || !returnName[0]) {
		returnName = "Unknown Solderer";
	}
	return returnName;
}

void NetLibrary::SetPlayerName(const char* name)
{
	m_playerName = name;
}

void NetLibrary::SendData(const NetAddress& address, const char* data, size_t length)
{
	m_impl->SendData(address, data, length);
}

void NetLibrary::AddReliableHandler(const char* type, const ReliableHandlerType& function, bool runOnMainThreadOnly /* = false */)
{
	uint32_t hash = HashRageString(type);

	m_reliableHandlers.insert({ hash, { function, runOnMainThreadOnly } });
}

void NetLibrary::DownloadsComplete()
{
	if (m_connectionState == CS_DOWNLOADING)
	{
		m_connectionState = CS_DOWNLOADCOMPLETE;
	}
}

bool NetLibrary::ProcessPreGameTick()
{
	if (m_connectionState != CS_ACTIVE && m_connectionState != CS_CONNECTED && m_connectionState != CS_IDLE)
	{
		RunFrame();

		return false;
	}

	return true;
}

void NetLibrary::SendNetEvent(const std::string& eventName, const std::string& jsonString, int i)
{
	const char* cmdType = "msgNetEvent";

	if (i == -1)
	{
		i = UINT16_MAX;
	}
	else if (i == -2)
	{
		cmdType = "msgServerEvent";
	}

	size_t eventNameLength = eventName.length();

	NetBuffer buffer(100000);

	if (i >= 0)
	{
		buffer.Write<uint16_t>(i);
	}

	buffer.Write<uint16_t>(eventNameLength + 1);
	buffer.Write(eventName.c_str(), eventNameLength + 1);

	buffer.Write(jsonString.c_str(), jsonString.size());
	
	SendReliableCommand(cmdType, buffer.GetBuffer(), buffer.GetCurLength());
}

/*void NetLibrary::AddReliableHandler(const char* type, ReliableHandlerType function)
{
	netLibrary.AddReliableHandlerImpl(type, function);
}*/

NetLibrary::NetLibrary()
	: m_serverNetID(0), m_serverBase(0), m_hostBase(0), m_hostNetID(0), m_connectionState(CS_IDLE), m_lastConnectionState(CS_IDLE),
	  m_lastConnect(0), m_impl(nullptr)

{
	m_receiveEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

__declspec(dllexport) fwEvent<NetLibrary*> NetLibrary::OnNetLibraryCreate;
__declspec(dllexport) fwEvent<const std::function<void(uint32_t, const char*, int)>&> NetLibrary::OnBuildMessage;

NetLibrary* NetLibrary::Create()
{
	auto lib = new NetLibrary();

	lib->CreateResources();

	lib->AddReliableHandler("msgIHost", [=] (const char* buf, size_t len)
	{
		NetBuffer buffer(buf, len);

		uint16_t hostNetID = buffer.Read<uint16_t>();
		uint32_t hostBase = buffer.Read<uint32_t>();

		lib->SetHost(hostNetID, hostBase);
	});

	OnNetLibraryCreate(lib);

	return lib;
}
