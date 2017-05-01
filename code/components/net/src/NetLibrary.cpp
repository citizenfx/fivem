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

#include <Error.h>

std::unique_ptr<NetLibraryImplBase> CreateNetLibraryImplV1(INetLibraryInherit* base);
std::unique_ptr<NetLibraryImplBase> CreateNetLibraryImplV2(INetLibraryInherit* base);

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

static uint32_t m_tempGuid = GetTickCount();

uint16_t NetLibrary::GetServerNetID()
{
	return m_serverNetID;
}

uint16_t NetLibrary::GetHostNetID()
{
	return m_hostNetID;
}

void NetLibrary::HandleConnected(int serverNetID, int hostNetID, int hostBase)
{
	m_serverNetID = serverNetID;
	m_hostNetID = hostNetID;
	m_hostBase = hostBase;

	trace("connectOK, our id %d, host id %d\n", m_serverNetID, m_hostNetID);

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

				if (steam)
				{
					char hostname[256] = { 0 };
					strncpy(hostname, Info_ValueForKey(infoString, "hostname"), 255);

					char cleaned[256];

					StripColors(hostname, cleaned, 256);

					steam->SetRichPresenceTemplate("{0}\n\n{2} on {3} with {1}");
					steam->SetRichPresenceValue(0, std::string(cleaned).substr(0, 64) + "...");
					steam->SetRichPresenceValue(1, "Connecting...");
					steam->SetRichPresenceValue(2, Info_ValueForKey(infoString, "gametype"));
					steam->SetRichPresenceValue(3, Info_ValueForKey(infoString, "mapname"));
				}
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

				GlobalError("%s", std::string(errorStr, length - 6));
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

	std::for_each(range.first, range.second, [&] (std::pair<uint32_t, ReliableHandlerType> handler)
	{
		handler.second(buf, length);
	});
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

void NetLibrary::RunFrame()
{
	if (!g_netFrameMutex.try_lock())
	{
		return;
	}

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
				FinalizeDisconnect();

				OnConnectionTimedOut();

				GlobalError("Server connection timed out after 15 seconds.");
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

void NetLibrary::ConnectToServer(const char* hostname, uint16_t port)
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
					lib->FinalizeDisconnect();
				}

				return true;
			});
		}
	} es(this);

	m_connectionState = CS_INITING;
	m_currentServer = NetAddress(hostname, port);

	if (m_impl)
	{
		m_impl->Reset();
	}

	m_outSequence = 0;

	wchar_t wideHostname[256];
	mbstowcs(wideHostname, hostname, _countof(wideHostname) - 1);

	wideHostname[255] = L'\0';

	fwWString wideHostnameStr = wideHostname;

	static fwMap<fwString, fwString> postMap;
	postMap["method"] = "initConnect";
	postMap["name"] = GetPlayerName();
	postMap["protocol"] = va("%d", NETWORK_PROTOCOL);

	static std::function<void()> performRequest;

	uint16_t capturePort = port;

	postMap["guid"] = va("%lld", GetGUID());

	static fwAction<bool, const char*, size_t> handleAuthResult;
	handleAuthResult = [=] (bool result, const char* connDataStr, size_t size) mutable
	{
		OnConnectionProgress("Handshaking...", 0, 100);

		std::string connData(connDataStr, size);

		if (!result)
		{
			// TODO: add UI output
			m_connectionState = CS_IDLE;

			//nui::ExecuteRootScript("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connectFailed', message: 'General handshake failure.' }, '*');");
			OnConnectionError(va("Failed handshake to server %s:%d%s%s.", m_currentServer.GetAddress(), m_currentServer.GetPort(), connData.length() > 0 ? " - " : "", connData));

			return;
		}

		try
		{
			auto node = YAML::Load(connData);

			if (node["error"].IsDefined())
			{
				// FIXME: single quotes
				//nui::ExecuteRootScript(va("citFrames[\"mpMenu\"].contentWindow.postMessage({ type: 'connectFailed', message: '%s' }, '*');", node["error"].as<std::string>().c_str()));
				OnConnectionError(node["error"].as<std::string>().c_str());

				m_connectionState = CS_IDLE;

				return;
			}

			if (!node["sH"].IsDefined())
			{
				// Server did not send a scripts setting: old server or rival project
				OnConnectionError("Legacy servers are incompatible with this version of FiveM. Update the server to the latest files from fivem.net");
				m_connectionState = CS_IDLE;
				return;
			}
			else Instance<ICoreGameInit>::Get()->ShAllowed = node["sH"].as<bool>(true);

			Instance<ICoreGameInit>::Get()->EnhancedHostSupport = (node["enhancedHostSupport"].IsDefined() && node["enhancedHostSupport"].as<bool>(false));

			m_token = node["token"].as<std::string>();

			m_serverProtocol = node["protocol"].as<uint32_t>();

			auto steam = GetSteam();

			if (steam)
			{
				steam->SetConnectValue(fmt::sprintf("+connect %s:%d", m_currentServer.GetAddress(), m_currentServer.GetPort()));
			}

			m_connectionState = CS_INITRECEIVED;

			if (node["netlibVersion"].as<int>(1) == 2)
			{
				m_impl = CreateNetLibraryImplV2(this);
			}
			else
			{
				m_impl = CreateNetLibraryImplV1(this);
			}
		}
		catch (YAML::Exception&)
		{
			m_connectionState = CS_IDLE;
		}
	};

	performRequest = [=]()
	{
		m_httpClient->DoPostRequest(wideHostname, port, L"/client", postMap, handleAuthResult);
	};


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
	
}

void NetLibrary::Disconnect(const char* reason)
{
	g_disconnectReason = reason;

	OnAttemptDisconnect(reason);
	//GameInit::KillNetwork((const wchar_t*)1);
}

void NetLibrary::FinalizeDisconnect()
{
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

void NetLibrary::AddReliableHandler(const char* type, ReliableHandlerType function)
{
	uint32_t hash = HashRageString(type);

	m_reliableHandlers.insert(std::make_pair(hash, function));
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