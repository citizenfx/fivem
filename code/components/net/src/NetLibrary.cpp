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
#include <SteamComponentAPI.h>
#include <LegitimacyAPI.h>
#include <IteratorView.h>
#include <optional>
#include <random>
#include <skyr/url.hpp>
#include <ResumeComponent.h>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <experimental/coroutine>
#include <pplawait.h>
#include <ppltasks.h>

#include <CrossBuildRuntime.h>
#include <PureModeState.h>
#include <CoreConsole.h>

#include <CfxLocale.h>

#include <json.hpp>

using json = nlohmann::json;

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
		double total = 0;
		double average = 0;
		double variance = 0;

		// gather tick delta
		size_t ticks[TIMEOUT_DATA_SIZE];

		for (int i = 1; i < TIMEOUT_DATA_SIZE; i++)
		{
			ticks[i - 1] = list[i] - list[i - 1];
		}

		ticks[TIMEOUT_DATA_SIZE - 1] = timeGetTime() - list[TIMEOUT_DATA_SIZE - 1];

		// calculate total/average/variance
		for (auto val : ticks)
		{
			total += val;
		}

		average = total / std::size(ticks);

		// variance
		total = 0;

		for (auto val : ticks)
		{
			total += pow(abs(val - average), 2);
		}

		variance = sqrt(total / std::size(ticks));

		// actual count
		return fmt::sprintf("%.2f ±%.2f ~%d", average, variance, ticks[TIMEOUT_DATA_SIZE - 1]);
	};

	return fmt::sprintf(
		gettext("**Timeout info**: game=%s, recv=%s, send=%s\n"),
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

	// if private client is unavailable, panic out
	// (usually caused by inaccurate Steam client DLL emulation/wrappers)
	if (!steamComponent->GetPrivateClient())
	{
		return nullptr;
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

	m_reconnectAttempts = 0;
	m_lastReconnect = 0;

	trace("^2Received connectOK: ServerID %d, SlotID %d, HostID %d\n", m_serverNetID, m_serverSlotID, m_hostNetID);

	OnConnectOKReceived(m_currentServer);

	if (m_connectionState != CS_ACTIVE)
	{
		m_connectionState = CS_CONNECTED;
	}
	else
	{
		Instance<ICoreGameInit>::Get()->ClearVariable("networkTimedOut");
	}
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

				static char hostname[8192] = { 0 };
				strncpy(hostname, Info_ValueForKey(infoString, "hostname"), 8191);

				static char cleaned[8192];

				StripColors(hostname, cleaned, 8192);

#if defined(GTA_FIVE) || defined(GTA_NY)
				SetWindowText(CoreGetGameWindow(), va(
#ifdef GTA_FIVE
					L"FiveM® by Cfx.re"
#elif defined(GTA_NY)
					L"LibertyM™ by Cfx.re"
#endif
					L" - %s", ToWide(cleaned)));
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

			m_lastReconnect = 0;
			m_reconnectAttempts = 0;
		}
		else if (!_strnicmp(oob, "error", 5))
		{
			if (m_disconnecting)
			{
				return;
			}

			if (from != m_currentServer)
			{
				trace("Received 'error' request was not from the host\n");
				return;
			}

			if (length >= 6)
			{
				const char* errorStr = &oob[6];
				auto errText = std::string(errorStr, length - 6);
				auto errHeading = "Disconnected by server";

				if (strstr(errorStr, "Timed out") != nullptr || strstr(errorStr, "timed out") != nullptr)
				{
					errHeading = "Timed out";
					errText += fmt::sprintf("\n\n---\n\n%s", CollectTimeoutInfo());
					errText += "\n[Reconnect](cfx.re://reconnect)";
				}

				if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
				{
					GlobalError("[md]# %s\n%s", errHeading, errText);
				}
				else
				{
					m_mainFrameQueue.push([errText]()
					{
						Instance<ICoreGameInit>::Get()->KillNetwork(ToWide(fmt::sprintf("Disconnected by server: %s", errText)).c_str());
					});
				}
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
	if (auto impl = GetImpl())
	{
		impl->SendReliableCommand(HashRageString(type), buffer, length);
	}
}

void NetLibrary::SendUnreliableCommand(const char* type, const char* buffer, size_t length)
{
	if (auto impl = GetImpl())
	{
		impl->SendUnreliableCommand(HashRageString(type), buffer, length);
	}
}

static std::string g_disconnectReason;

static std::mutex g_netFrameMutex;

inline uint64_t GetGUID()
{
	return (uint64_t)(0x210000100000000 | m_tempGuid);
}

uint64_t NetLibrary::GetGUID()
{
	return ::GetGUID();
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

	if (auto impl = GetImpl())
	{
		impl->RunFrame();
	}

	switch (m_connectionState)
	{
		case CS_INITRECEIVED:
			// change connection state to CS_DOWNLOADING
			m_connectionState = CS_DOWNLOADING;

			// trigger task event
			OnConnectionProgress("Downloading content", 0, 1, false);
			OnInitReceived(m_currentServer);

			break;

		case CS_DOWNLOADCOMPLETE:
			m_connectionState = CS_FETCHING;
			m_lastConnect = 0;
			m_connectAttempts = 0;

			OnConnectionProgress("Downloading completed", 1, 1, false);

			break;

		case CS_FETCHING:
			if ((GetTickCount() - m_lastConnect) > 5000)
			{
				SendOutOfBand(m_currentServer, "getinfo xyz");

				m_lastConnect = GetTickCount();

				m_connectAttempts++;

				// advertise status
				auto specStatus = (m_connectAttempts > 1) ? fmt::sprintf(" (attempt %d)", m_connectAttempts) : "";

				OnConnectionProgress(fmt::sprintf("Fetching info from server...%s", specStatus), 1, 1, true);
			}

			if (m_connectAttempts > 3)
			{
				Disconnect("Fetching info timed out.");

				OnConnectionTimedOut();

				GlobalError("[md]%s", fmt::sprintf(gettext("# Couldn't connect\nFailed to get info from server (tried 3 times).\n\n---\n\nIf you are the server owner, are you sure you are allowing UDP packets to and from the server?")));
			}
			break;

		case CS_CONNECTING:
			if ((GetTickCount() - m_lastConnect) > 5000 && GetImpl()->IsDisconnected())
			{
				GetImpl()->SendConnect(m_token, fmt::sprintf("token=%s&guid=%llu", m_token, (uint64_t)GetGUID()));

				m_lastConnect = GetTickCount();

				m_connectAttempts++;

				// advertise status
				auto specStatus = (m_connectAttempts > 1) ? fmt::sprintf(" (attempt %d)", m_connectAttempts) : "";

				OnConnectionProgress(fmt::sprintf("Connecting to server...%s", specStatus), 1, 1, false);
			}

			if (m_connectAttempts > 3)
			{
				Disconnect("Connection timed out.");

				OnConnectionTimedOut();

				GlobalError("Failed to connect to server after 3 attempts.");
			}

			break;

		case CS_ACTIVE:
			if (GetImpl()->HasTimedOut())
			{
				g_disconnectReason = "Connection timed out.";

				OnConnectionTimedOut();

				GlobalError("[md]%s", fmt::sprintf(gettext("# Timed out\nClient -> server connection timed out. Please try again later.\n\n---\n\n%s\n[Reconnect](cfx.re://reconnect)"), CollectTimeoutInfo()));

				m_connectionState = CS_IDLE;
				m_currentServer = NetAddress();
			}
			else
			{
				m_lastReconnect = GetTickCount() - 2500;
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
        if (size_t(pout + 3 - out) > outsz)
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

static concurrency::task<std::optional<std::string>> ResolveUrl(const std::string& rootUrl)
{
	try
	{
		auto uri = skyr::make_url(rootUrl);

		if (uri && !uri->protocol().empty())
		{
			if (uri->protocol() == "fivem:")
			{
				// this whatwg url spec is very 'special' and doesn't allow you to ever make a new url and set protocol to any 'special' scheme
				// such as 'http' or 'https' or 'file'
				// and compared to cpp-uri the uri_builder was removed too
				// so we do super verbose making a record

				skyr::url_record record;
				record.scheme = "https";

				skyr::url newUri{ std::move(record) };
				newUri.set_port(uri->port().empty() ? atoi(uri->port().c_str()) : 30120);
				newUri.set_pathname("/");

				*uri = newUri;
			}

			if (uri->protocol() == "http:" || uri->protocol() == "https:")
			{
				co_return uri->href();
			}
		}
	}
	catch (const std::exception& e)
	{
		
	}

	if (rootUrl.find(".cfx.re") != std::string::npos && rootUrl.find("https:") == std::string::npos)
	{
		co_return co_await ResolveUrl(fmt::sprintf("https://%s/", rootUrl));
	}
	
	// if it doesn't contain a . or a : it might be a join URL
	// (or if it is a join URL, it is a join URL)
	if (rootUrl.find_first_of(".:") == std::string::npos || rootUrl.find("cfx.re/join") != std::string::npos)
	{
		concurrency::task_completion_event<std::optional<std::string>> tce;

		HttpRequestOptions ro;
		ro.responseHeaders = std::make_shared<HttpHeaderList>();

		// prefix cfx.re/join if we can
		auto joinRootUrl = rootUrl;

		if (joinRootUrl.find("cfx.re/join") == std::string::npos)
		{
			joinRootUrl = "cfx.re/join/" + rootUrl;
		}

		Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("https://%s", joinRootUrl), ro, [ro, tce](bool success, const char* data, size_t callback)
		{
			if (success)
			{
				const auto& rh = *ro.responseHeaders;
				
				if (auto it = rh.find("X-CitizenFX-Url"); it != rh.end() && it->second != "https://private-placeholder.cfx.re/")
				{
					auto url = it->second;

					auto taskRef = [tce, url]() -> concurrency::task<void>
					{
						tce.set(co_await ResolveUrl(url));
					};

					taskRef();

					return;
				}
			}

			tce.set({});
		});

		auto joinUrlBit = co_await concurrency::task<std::optional<std::string>>{ tce };

		if (joinUrlBit)
		{
			co_return joinUrlBit;
		}
	}

	auto peerAddress = net::PeerAddress::FromString(rootUrl);
	
	if (peerAddress)
	{
		// same as above, we need a record
		skyr::url_record record;
		record.scheme = "http";

		skyr::url newUri{ std::move(record) };
		newUri.set_host(peerAddress->ToString());
		newUri.set_pathname("/");

		co_return newUri.href();
	}

	co_return {};
}

void NetLibrary::OnConnectionError(const std::string& errorString, const std::string& metaData /* = "{}" */)
{
	OnConnectionErrorEvent(errorString.c_str());
	OnConnectionErrorRichEvent(errorString, metaData);
}

// hack for NetLibraryImplV2
int g_serverVersion;

concurrency::task<void> NetLibrary::ConnectToServer(const std::string& rootUrl)
{
	m_disconnecting = false;

	std::string ruRef = rootUrl;

	// increment the GUID so servers won't race to remove us
	m_tempGuid++;

	auto urlRef = co_await ResolveUrl(ruRef);

	if (!urlRef)
	{
		OnConnectionError(fmt::sprintf("Couldn't resolve URL %s.", ruRef), json::object({
			{ "fault", "either" },
			{ "status", true },
			{ "action", "#ErrorAction_TryAgainCheckStatus" },
		}).dump());

		co_return;
	}

	auto url = *urlRef;

	if (m_connectionState != CS_IDLE)
	{
		Disconnect("Connecting to another server.");
	}

	// late-initialize error state in ICoreGameInit
	// this happens here so it only tries capturing if connection was attempted
	static struct ErrorState 
	{
		ErrorState(NetLibrary* lib)
		{
			Instance<ICoreGameInit>::Get()->OnTriggerError.Connect([=] (const std::string& errorMessage)
			{
				std::string richError = (lib->m_richError.empty()) ? "{}" : lib->m_richError;
				lib->m_richError = "";

				if (lib->m_connectionState != CS_ACTIVE)
				{
					lib->OnConnectionError(errorMessage.c_str(), richError);

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
						lib->Disconnect();
					}
				}

				return true;
			});
		}
	} es(this);

	m_connectionState = CS_INITING;
	m_currentServerUrl = url;

	AddCrashometry("last_server_url", "%s", url);

	if (auto impl = GetImpl())
	{
		impl->Reset();
	}

	m_outSequence = 0;

	static fwMap<fwString, fwString> postMap;
	postMap["method"] = "initConnect";
	postMap["name"] = GetPlayerName();
	postMap["protocol"] = va("%d", NETWORK_PROTOCOL);
	postMap["gameBuild"] = fmt::sprintf("%d", xbr::GetGameBuild());

#if defined(IS_RDR3)
	postMap["gameName"] = "rdr3";
#elif defined(GTA_FIVE)
	postMap["gameName"] = "gta5";
#elif defined(GTA_NY)
	postMap["gameName"] = "gta4";
#endif

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

			OnConnectionError(fmt::sprintf("Failed handshake to server %s%s%s.", url, connData.length() > 0 ? " - " : "", connData), json::object({
						{ "fault", "server" },
						{ "action", "#ErrorAction_TryAgainContactOwner" },
						})
			.dump());

			return;
		}
		else if (!isLegacyDeferral && !Instance<ICoreGameInit>::Get()->OneSyncEnabled)
		{
			OnConnectionError(fmt::sprintf("Failed handshake to server %s - it closed the connection while deferring.", url), json::object({
						{ "fault", "server" },
						{ "action", "#ErrorAction_TryAgainContactOwner" },
						})
			.dump());
		}
	};

	static struct Stream
		: public std::streambuf
	{
		Stream()
			: read(0)
		{
			memset(buffer, 0, sizeof(buffer));
		}

		void Reset()
		{
			setg(egptr(), egptr(), egptr());

			queue.clear();
			read = 0;
		}

		size_t Tell()
		{
			return read - (egptr() - gptr());
		}

		void Seek(size_t pos)
		{
			if (pos < queue.size())
			{
				read = pos;

				// reset to ensure underflow gets called again
				setg(egptr(), egptr(), egptr());
			}
		}

		void Push(std::string_view data)
		{
			size_t oldSize = queue.size();
			queue.resize(oldSize + data.size());

			std::copy(data.begin(), data.end(), queue.begin() + oldSize);
		}

		int underflow()
		{
			if (gptr() == egptr())
			{
				size_t rlen = std::min(sizeof(buffer), queue.size() - read);
				std::copy(queue.begin() + read, queue.begin() + read + rlen, buffer);

				read += rlen;

				setg(buffer, buffer, buffer + rlen);
			}

			return gptr() == egptr()
				? std::char_traits<char>::eof()
				: std::char_traits<char>::to_int_type(*gptr());
		}

	private:
		std::deque<char> queue;
		size_t read;

		char buffer[1024];
	} stream;

	stream.Reset();

	g_serverVersion = 0;

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
		stream.Push(connData);

		std::istream is(&stream);

		while (true)
		{
			try
			{
				json node;

				auto start = stream.Tell();

				try
				{
					is >> node;
				}
				catch (std::exception& e)
				{
					stream.Seek(start);
					return true;
				}

				if (!node["token"].is_null())
				{
					m_token = node["token"].get<std::string>();
					Instance<ICoreGameInit>::Get()->SetData("connectionToken", m_token);
				}

				if (!node["defer"].is_null())
				{
					if (!node["deferVersion"].is_null())
					{
						// new deferral system
						if (!node["message"].is_null())
						{
							OnConnectionProgress(node["message"].get<std::string>(), 5, 100, true);
						}
						else if (!node["card"].is_null())
						{
							OnConnectionCardPresent(node["card"].get<std::string>(), node["token"].get<std::string>());
						}

						continue;
					}

					isLegacyDeferral = true;

					OnConnectionProgress(node["status"].get<std::string>(), 5, 100, true);

					static fwMap<fwString, fwString> newMap;
					newMap["method"] = "getDeferState";
					newMap["guid"] = va("%lld", GetGUID());
					newMap["token"] = m_token;

					HttpRequestOptions options;
					options.streamingCallback = handleAuthResultData;
					m_handshakeRequest = m_httpClient->DoPostRequest(fmt::sprintf("%sclient", url), m_httpClient->BuildPostString(newMap), options, handleAuthResult);

					continue;
				}

				m_handshakeRequest = {};

				if (!node["error"].is_null())
				{
					OnConnectionError(fmt::sprintf("Connection rejected by server: %s", node["error"].get<std::string>()), json::object({
						{ "fault", "server" },
						{ "action", "#ErrorAction_SeeDetailsContactOwner" },
					})
					.dump());

					m_connectionState = CS_IDLE;

					return true;
				}

				if (node["sH"].is_null())
				{
					OnConnectionError("Invalid server response from initConnect (missing JSON data), is this server running a broken resource?");
					m_connectionState = CS_IDLE;
					return true;
				}
				else
				{
					Instance<ICoreGameInit>::Get()->ShAllowed = node.value("sH", true);
				}

#if defined(IS_RDR3)
				if (node["gamename"].is_null() || node["gamename"].get<std::string>() != "rdr3")
				{
					OnConnectionError("This server is not compatible with RedM, as it's for FiveM. Please join an actual RedM server instead.");
					m_connectionState = CS_IDLE;
					return true;
				}
#endif

				auto bitVersion = (!node["bitVersion"].is_null() ? node["bitVersion"].get<uint64_t>() : 0);
				auto rawEndpoints = (node.find("endpoints") != node.end()) ? node["endpoints"] : json{};

				auto continueAfterEndpoints = [=, capNode = node](const json& capEndpointsJson)
				{
					// copy to a non-const `json` so operator[] won't use the read-only version asserting on missing key
					auto node = capNode;
					auto endpointsJson = capEndpointsJson;

					try
					{
						// gather endpoints
						std::vector<std::string> endpoints;

						if (!endpointsJson.is_null() && !endpointsJson.is_boolean())
						{
							for (const auto& endpoint : endpointsJson)
							{
								endpoints.push_back(endpoint.get<std::string>());
							}
						}
						else if (!rawEndpoints.is_null() && rawEndpoints.is_array() && !rawEndpoints.empty())
						{
							for (const auto& endpoint : rawEndpoints)
							{
								endpoints.push_back(endpoint.get<std::string>());
							}
						}

						if (endpoints.empty())
						{
							auto uri = skyr::make_url(url);
							std::string endpoint;

							if (uri->port<int>())
							{
								endpoint = fmt::sprintf("%s:%d", uri->hostname(), *uri->port<int>());
							}
							else
							{
								endpoint = uri->hostname();
							}

							endpoints.push_back(endpoint);
						}

						// select an endpoint
						static std::random_device rand;
						static std::mt19937 rng(rand());
						std::uniform_int_distribution<> values(0, endpoints.size() - 1);

						const auto& endpoint = endpoints[values(rng)];
						auto addressStrRef = net::PeerAddress::FromString(endpoint);

						if (!addressStrRef)
						{
							OnConnectionError(fmt::sprintf("Could not resolve returned endpoint: %s", endpoint));
							m_connectionState = CS_IDLE;
							return true;
						}

						auto address = *addressStrRef;
						auto oldAddress = NetAddress(address.GetSocketAddress());

						m_currentServer = oldAddress;
						m_currentServerPeer = address;

						AddCrashometry("last_server", "%s", address.ToString());

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

						Instance<ICoreGameInit>::Get()->SetData("handoverBlob", (!node["handover"].is_null()) ? node["handover"].dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace) : "{}");

						Instance<ICoreGameInit>::Get()->EnhancedHostSupport = (!node["enhancedHostSupport"].is_null() && node.value("enhancedHostSupport", false));
						Instance<ICoreGameInit>::Get()->OneSyncEnabled = (!node["onesync"].is_null() && node["onesync"].get<bool>());
						Instance<ICoreGameInit>::Get()->OneSyncBigIdEnabled = (!node["onesync_lh"].is_null() && node["onesync_lh"].get<bool>());
						Instance<ICoreGameInit>::Get()->NetProtoVersion = bitVersion;

						bool big1s = (!node["onesync_big"].is_null() && node["onesync_big"].get<bool>());

						if (big1s)
						{
							AddCrashometry("onesync_big", "true");
							Instance<ICoreGameInit>::Get()->SetVariable("onesync_big");
						}
						else
						{
							AddCrashometry("onesync_big", "false");
							Instance<ICoreGameInit>::Get()->ClearVariable("onesync_big");
						}

						auto maxClients = (!node["maxClients"].is_null()) ? node["maxClients"].get<int>() : 64;

#ifndef _DEBUG
						std::string onesyncType = "onesync";

						if (maxClients <= 48)
						{
							onesyncType = "";
						}
						else if (maxClients <= 64)
						{
							onesyncType = "onesync";
						}
						else if (maxClients <= 128)
						{
							if (!big1s)
							{
								onesyncType = "onesync_plus";
							}
							else
							{
								onesyncType = "onesync_medium";
							}
						}
						else if (maxClients <= 2048)
						{
							onesyncType = "onesync_big";
						}
#else
						std::string onesyncType = "";
#endif

						AddCrashometry("onesync_enabled", (Instance<ICoreGameInit>::Get()->OneSyncEnabled) ? "true" : "false");

						m_serverProtocol = node["protocol"].get<uint32_t>();

						auto steam = GetSteam();

						if (steam)
						{
							steam->SetConnectValue(fmt::sprintf("+connect %s:%d", m_currentServer.GetAddress(), m_currentServer.GetPort()));
						}

						auto continueAfterAllowance = [=]()
						{
							auto doneCB = [=](const char* data, size_t size)
							{
								{
									try
									{
										json info = json::parse(data, data + size);

										if (info.is_object() && info["server"].is_string())
										{
											auto serverData = info["server"].get<std::string>();
											boost::algorithm::replace_all(serverData, " win32", "");
											boost::algorithm::replace_all(serverData, " linux", "");
											boost::algorithm::replace_all(serverData, " SERVER", "");
											boost::algorithm::replace_all(serverData, "FXServer-", "");

											try
											{
												g_serverVersion = std::stoi(serverData.substr(serverData.find_last_of('.') + 1));
											}
											catch (std::exception& e)
											{
												g_serverVersion = 0;
											}

											AddCrashometry("last_server_ver", serverData);
										}

										static std::set<std::string> policies;

										auto oneSyncPolicyFailure = [this, onesyncType, maxClients, big1s]()
										{
											int maxSlots = 48;
											std::string extraText;

											if (policies.find("onesync") != policies.end())
											{
												maxSlots = 64;
											}

											if (!big1s)
											{
												if (policies.find("onesync_plus") != policies.end())
												{
													maxSlots = 128;
												}
												else if (maxSlots >= 64 && maxClients > 64)
												{
													extraText = "\nUsing 128 slots with 'Element Club Aurum' requires you to enable OneSync 'on' (formerly named 'Infinity'), not 'legacy'. Check your server configuration.";
												}
											}
											else
											{
												if (policies.find("onesync_medium") != policies.end())
												{
													maxSlots = 128;
												}
											}

											if (policies.find("onesync_big") != policies.end())
											{
												maxSlots = 2048;
											}

											OnConnectionError(fmt::sprintf("This server uses more slots than allowed by the current subscription. The allowed slot count is %d, but the server has a maximum slot count of %d.%s",
												maxSlots,
												maxClients,
												extraText),
												json::object({
													{ "fault", "server" },
													{ "status", true },
													{ "action", "#ErrorAction_TryAgainContactOwner" },
												}).dump());

											m_connectionState = CS_IDLE;
										};

										auto policySuccess = [this]()
										{
											m_connectionState = CS_INITRECEIVED;
										};

										policies.clear();

										OnConnectionProgress("Requesting server feature policy...", 0, 100, false);

										if (info.is_object() && info["vars"].is_object())
										{
											auto val = info["vars"].value("sv_licenseKeyToken", "");

											if (!val.empty())
											{
												try
												{
													auto targetContext = val.substr(val.find_first_of('_') + 1);
													m_targetContext = targetContext.substr(0, targetContext.find_first_of(':'));
												}
												catch (std::exception& e)
												{
												}

												m_httpClient->DoGetRequest(fmt::sprintf("https://policy-live.fivem.net/api/policy/%s", val), [=](bool success, const char* data, size_t size)
												{
													std::string fact;

													// process policy response
													if (success)
													{
														try
														{
															json doc = json::parse(data, data + size);

															if (doc.is_array())
															{
																for (auto& entry : doc)
																{
																	if (entry.is_string())
																	{
																		policies.insert(entry.get<std::string>());
																	}
																}
															}
															else
															{
																fact = "Parsing policy failed (2).";
															}
														}
														catch (const std::exception& e)
														{
															trace("Policy parsing failed. %s\n", e.what());
															fact = "Parsing policy failed.";
														}
													}
													else
													{
														trace("Policy request failed. %s\n", std::string{ data, size });
														fact = "Requesting policy failed.";
													}

													// add forced policies
													if (maxClients <= 10)
													{
														// development/testing servers (<= 10 clients max - see ZAP defaults) get subdir_file_mapping granted
														policies.insert("subdir_file_mapping");
													}

													// dev server
													if (maxClients <= 8)
													{
														policies.insert("local_evaluation");
													}

													// format policy string and store it
													std::stringstream policyStr;

													for (const auto& line : policies)
													{
														policyStr << "[" << line << "]";
													}

													std::string policy = policyStr.str();

													if (!policy.empty())
													{
														trace("Server feature policy is %s\n", policy);
													}

													Instance<ICoreGameInit>::Get()->SetData("policy", policy);

													// check 1s policy
													if (Instance<ICoreGameInit>::Get()->OneSyncEnabled && !onesyncType.empty())
													{
														if (policies.find(onesyncType) == policies.end())
														{
															if (!fact.empty())
															{
																OnConnectionError(fmt::sprintf("Could not check server feature policy. %s", fact), json::object({
																	{ "fault", "cfx" },
																	{ "status", true },
																	{ "action", "#ErrorAction_TryAgainCheckStatus" },
																}).dump());

																m_connectionState = CS_IDLE;

																return;
															}

															oneSyncPolicyFailure();
															return;
														}
													}

													policySuccess();
												});

												return;
											}
										}

										policySuccess();
									}
									catch (std::exception& e)
									{
										OnConnectionError(fmt::sprintf("Info get failed for %s\n", e.what()), json::object({
											{ "fault", "server" },
											{ "action", "#ErrorAction_TryAgainContactOwner" },
										}).dump());

										m_connectionState = CS_IDLE;
									}
								}
							};

							m_httpClient->DoGetRequest(fmt::sprintf("%sinfo.json", url), [=](bool success, const char* data, size_t size)
							{
								if (success)
								{
									std::string blobStr(data, size);

									OnInfoBlobReceived(blobStr, [blobStr, doneCB]()
									{
										doneCB(blobStr.data(), blobStr.size());
									});
								}
								else
								{
									OnConnectionError("Failed to fetch /info.json to obtain policy metadata.", json::object({
												{ "fault", "server" },
												{ "action", "#ErrorAction_TryAgainContactOwner" },
									}).dump());

									m_connectionState = CS_IDLE;
								}
							});
						};

						auto blocklistResultHandler = [this, continueAfterAllowance](bool success, const char* data, size_t length)
						{
							if (success)
							{
								// poke if we aren't blocking *everyone* instead
								std::default_random_engine generator;
								std::uniform_int_distribution<int> distribution(1, 224);
								std::uniform_int_distribution<int> distribution2(1, 254);

								auto rndQ = fmt::sprintf("https://runtime.fivem.net/blocklist/%u.%u.%u.%u", distribution(generator), distribution2(generator), distribution2(generator), distribution2(generator));
								auto dStr = std::string(data, length);

								m_httpClient->DoGetRequest(rndQ, [this, continueAfterAllowance, dStr](bool success, const char* data, size_t length)
								{
									if (!success)
									{
										OnConnectionError(fmt::sprintf("This server has been blocked from the FiveM platform. Stated reason: %sIf you manage this server and you feel this is not justified, please contact your Technical Account Manager.", dStr).c_str());

										m_connectionState = CS_IDLE;

										return;
									}

									continueAfterAllowance();
								});

								return;
							}

							continueAfterAllowance();
						};

						OnConnectionProgress("Requesting server permissions...", 0, 100, false);

						HttpRequestOptions options;
						options.timeoutNoResponse = std::chrono::seconds(5);

						m_httpClient->DoGetRequest(fmt::sprintf("https://runtime.fivem.net/blocklist/%s", address.GetHost()), options, blocklistResultHandler);

						if (node.value("netlibVersion", 1) == 2)
						{
							std::unique_lock _(m_implMutex);
							m_impl = CreateNetLibraryImplV2(this);
						}
						else if (node.value("netlibVersion", 1) == 3 || node.value("netlibVersion", 1) == 4)
						{
							OnConnectionError("NetLibraryImplV3/NetLibraryImplV4 are no longer supported. Please reset `netlib` to the default value.");
							m_connectionState = CS_IDLE;
							return true;
						}
						else
						{
							OnConnectionError("Legacy servers are incompatible with this version of CitizenFX. Please tell the server owner to the server to the latest FXServer build. See https://fivem.net/ for more info.");
							m_connectionState = CS_IDLE;
							return true;
						}
					}
					catch (std::exception& e)
					{
						OnConnectionError(e.what());
						m_connectionState = CS_IDLE;
					}

					return false;
				};

				if (bitVersion >= 0x202004201223)
				{
					// to not complain about 'closed connection while deferring'
					isLegacyDeferral = true;

					fwMap<fwString, fwString> epMap;
					epMap["method"] = "getEndpoints";
					epMap["token"] = m_token;

					OnConnectionProgress("Requesting server endpoints...", 0, 100, false);

					m_httpClient->DoPostRequest(fmt::sprintf("%sclient", url), m_httpClient->BuildPostString(epMap), [rawEndpoints, continueAfterEndpoints](bool success, const char* data, size_t size)
					{
						if (success)
						{
							try
							{
								continueAfterEndpoints(nlohmann::json::parse(data, data + size));
								return;
							}
							catch (std::exception& e)
							{

							}
						}

						continueAfterEndpoints(rawEndpoints);
					});
				}
				else
				{
					continueAfterEndpoints(rawEndpoints);
				}
			}
			catch (std::exception & e)
			{
				OnConnectionError(e.what());
				m_connectionState = CS_IDLE;
			}
		}

		return true;
	};

	std::function<void()> continueRequest;

	performRequest = [=]()
	{
		OnConnectionProgress("Handshaking with server...", 0, 100, false);

		HttpRequestOptions options;
		options.streamingCallback = handleAuthResultData;

		m_handshakeRequest = m_httpClient->DoPostRequest(fmt::sprintf("%sclient", url), m_httpClient->BuildPostString(postMap), options, handleAuthResult);
	};

	m_cardResponseHandler = [this, url](const std::string& cardData, const std::string& token)
	{
		auto handleCardResult = [](bool result, const char* connDataStr, size_t size)
		{
			// TODO: response handling
		};

		m_handshakeRequest = m_httpClient->DoPostRequest(fmt::sprintf("%sclient", url), m_httpClient->BuildPostString({
			{ "method", "submitCard" },
			{ "data", cardData },
			{ "token", token }
		}), handleCardResult);
	};

	static std::string requestSteamTicket = "on";

	continueRequest = [=]()
	{
		auto steamComponent = GetSteam();

		if (steamComponent && requestSteamTicket == "on")
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

				OnConnectionProgress("Obtaining Steam ticket...", 0, 100, false);
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
	
	auto initiateRequest = [=]()
	{
		OnConnectionProgress("Requesting server variables...", 0, 100, true);
		
		m_httpClient->DoGetRequest(fmt::sprintf("%sinfo.json", url), [=](bool success, const char* data, size_t size)
		{
			using json = nlohmann::json;

			std::string licenseKeyToken;

			// We got a response way later than we wanted - user has canceled connection or joined another server
			if (m_connectionState != CS_INITING)
			{
				return;
			}

			if (!success)
			{
				static ConVar<bool> streamerMode("ui_streamerMode", ConVar_None, false);
				if (streamerMode.GetValue())
				{
					OnConnectionError("Failed to fetch server variables.", json::object({
								{ "fault", "server" },
								{ "action", "#ErrorAction_TryAgainContactOwner" },
					}).dump());
				}
				else
				{
					OnConnectionError(fmt::sprintf("Failed to fetch server variables. %s", std::string{ data, size }), json::object({
								{ "fault", "server" },
								{ "action", "#ErrorAction_TryAgainContactOwner" },
					}).dump());
				}
				m_connectionState = CS_IDLE;
				return;
			}

			try
			{
				json info = json::parse(data, data + size);
#if defined(GTA_FIVE) || defined(IS_RDR3)
				if (info.is_object() && info["vars"].is_object())
				{
					int pureLevel = 0;

					if (auto pureVal = info["vars"].value("sv_pureLevel", "0"); !pureVal.empty())
					{
						pureLevel = std::stoi(pureVal);
					}

					auto val = info["vars"].value("sv_enforceGameBuild", "");
					int buildRef = 0;

					if (!val.empty())
					{
						buildRef = std::stoi(val);

						if ((buildRef != 0 && buildRef != xbr::GetGameBuild()) || (pureLevel != fx::client::GetPureLevel()))
						{
#if defined(GTA_FIVE)
							if (buildRef != 1604 && buildRef != 2060 && buildRef != 2189 && buildRef != 2372 && buildRef != 2545 && buildRef != 2612 && buildRef != 2699 && buildRef != 2802)
#else
							if (buildRef != 1311 && buildRef != 1355 && buildRef != 1436 && buildRef != 1491)
#endif
							{
								OnConnectionError(va("Server specified an invalid game build enforcement (%d).", buildRef), json::object({
									{ "fault", "server" },
									{ "action", "#ErrorAction_ContactOwner" },
								})
								.dump());
								m_connectionState = CS_IDLE;
								return;
							}

							OnRequestBuildSwitch(buildRef, pureLevel);
							m_connectionState = CS_IDLE;
							return;
						}
					}

#if defined(GTA_FIVE)
					if (xbr::GetGameBuild() != 1604 && buildRef == 0)
					{
						OnRequestBuildSwitch(1604, 0);
						m_connectionState = CS_IDLE;
						return;
					}
#endif

					auto ival = info["vars"].value("sv_licenseKeyToken", "");

					if (!ival.empty())
					{
						licenseKeyToken = ival;
					}

					requestSteamTicket = info.value("requestSteamTicket", "on");
				}
#endif
			}
			catch (std::exception& e)
			{
			}

			if (OnInterceptConnectionForAuth(url, licenseKeyToken, [this, continueRequest](bool success, const std::map<std::string, std::string>& additionalPostData)
				{
					if (success)
					{
						for (const auto& entry : additionalPostData)
						{
							postMap[entry.first] = entry.second;
						}

						continueRequest();
					}
					else
					{
						m_connectionState = CS_IDLE;
					}
				}))
			{
				continueRequest();
			}
		});
	};

	if (OnInterceptConnection(url, initiateRequest))
	{
		initiateRequest();
	}
}

void NetLibrary::SubmitCardResponse(const std::string& dataJson, const std::string& token)
{
	auto responseHandler = m_cardResponseHandler;

	if (responseHandler)
	{
		responseHandler(dataJson, token);
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

static std::mutex g_disconnectionMutex;

void NetLibrary::Disconnect(const char* reason)
{
	g_disconnectReason = reason;

	OnAttemptDisconnect(reason);
	//GameInit::KillNetwork((const wchar_t*)1);

	std::unique_lock<std::mutex> lock(g_disconnectionMutex);

	if (m_connectionState == CS_DOWNLOADING)
	{
		OnFinalizeDisconnect(m_currentServer);
	}

	if (m_connectionState == CS_CONNECTING || m_connectionState == CS_ACTIVE || m_connectionState == CS_FETCHING)
	{
		m_disconnecting = true;

		SendReliableCommand("msgIQuit", g_disconnectReason.c_str(), g_disconnectReason.length() + 1);

		if (auto impl = GetImpl())
		{
			impl->Flush();

			// this is *somewhat* needed to ensure the server gets our msgIQuit first
			Sleep(std::min(750, impl->GetPing() + abs(impl->GetVariance())));

			impl->Reset();
		}

		OnFinalizeDisconnect(m_currentServer);

		m_connectionState = CS_IDLE;
		m_currentServer = NetAddress();

		// we don't want to tell Steam to launch a new child as we're exiting
		if (reason != std::string_view{ "Exiting" })
		{
			auto steam = GetSteam();

			if (steam)
			{
				steam->SetRichPresenceValue(0, "");
			}
		}
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

bool NetLibrary::IsPendingInGameReconnect()
{
	return (m_connectionState == CS_ACTIVE && GetImpl()->IsDisconnected());
}

static std::string g_steamPersonaName;

const char* NetLibrary::GetPlayerName()
{
	// if a higher-level component set a name, use that name instead
	if (!m_playerName.empty())
	{
		return m_playerName.c_str();
	}

	// do we have a Steam name?
	if (!g_steamPersonaName.empty())
	{
		return g_steamPersonaName.c_str();
	}

	static std::wstring returnNameWide;
	static std::string returnName;

	auto envName = _wgetenv(L"USERNAME");

	if (envName != nullptr)
	{
		returnNameWide = envName;
	}

	if (returnNameWide.empty())
	{
		static wchar_t computerName[64];
		DWORD nameSize = _countof(computerName);
		GetComputerNameW(computerName, &nameSize);
		returnNameWide = computerName;
	}

	if (returnNameWide.empty())
	{
		returnNameWide = L"UnknownPlayer";
	}

	returnName = ToNarrow(returnNameWide);

	return returnName.c_str();
}

void NetLibrary::SetPlayerName(const char* name)
{
	m_playerName = name;
}

void NetLibrary::SendData(const NetAddress& address, const char* data, size_t length)
{
	GetImpl()->SendData(address, data, length);
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

	net::Buffer buffer;

	if (i >= 0)
	{
		buffer.Write<uint16_t>(i);
	}

	buffer.Write<uint16_t>(uint16_t(eventNameLength + 1));
	buffer.Write(eventName.c_str(), eventNameLength + 1);

	buffer.Write(jsonString.c_str(), jsonString.size());
	
	SendReliableCommand(cmdType, reinterpret_cast<const char*>(buffer.GetBuffer()), buffer.GetCurOffset());
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
		net::Buffer buffer(reinterpret_cast<const uint8_t*>(buf), len);

		uint16_t hostNetID = buffer.Read<uint16_t>();
		uint32_t hostBase = buffer.Read<uint32_t>();

		lib->SetHost(hostNetID, hostBase);
	});

	OnNetLibraryCreate(lib);

	OnAbnormalTermination.Connect([lib](void* reason)
	{
		if (lib->GetConnectionState() != NetLibrary::CS_IDLE)
		{
			lib->Disconnect((const char*)reason);
		}
	});
	
	auto steamComponent = GetSteam();

	if (steamComponent)
	{
		IClientEngine* steamClient = steamComponent->GetPrivateClient();

		if (steamClient)
		{
			InterfaceMapper steamFriends(steamClient->GetIClientFriends(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTFRIENDS_INTERFACE_VERSION001"));

			if (steamFriends.IsValid())
			{
				g_steamPersonaName = steamFriends.Invoke<const char*>("GetPersonaName");
			}
		}
	}

	static ConVar<std::string> extNicknameVar("ui_extNickname", ConVar_ReadOnly, g_steamPersonaName);

	return lib;
}

int32_t NetLibrary::GetPing()
{
	if (auto impl = GetImpl())
	{
		return impl->GetPing();
	}

	return -1;
}

int32_t NetLibrary::GetVariance()
{
	if (auto impl = GetImpl())
	{
		return impl->GetVariance();
	}

	return -1;
}

void NetLibrary::SetRichError(const std::string& data /* = "{}" */)
{
	m_richError = data;
}
