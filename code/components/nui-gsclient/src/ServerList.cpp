/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <nutsnbolts.h>
#include <CefOverlay.h>
#include <mmsystem.h>
#include <WS2tcpip.h>
#include <strsafe.h>
#include <fstream>
#include <array>

#include <CoreConsole.h>
#include <InternalRPCHandler.h>

#include <NetAddress.h> // net:base

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <fnv.h>

#include <HttpClient.h>
#include <windns.h>

#include <tbb/concurrent_unordered_set.h>

#pragma comment(lib, "dnsapi.lib")

#if defined(GTA_NY)
#define GS_GAMENAME "GTA4"
#elif defined(PAYNE)
#define GS_GAMENAME "Payne"
#elif defined(GTA_FIVE)
#define GS_GAMENAME "GTA5"
#else
#define GS_GAMENAME "CitizenFX"
#endif

template<typename TFunc>
void RequestInfoBlob(const std::string& server, const TFunc& cb)
{
	static HttpClient* httpClient = Instance<HttpClient>::Get();

	net::PeerAddress addr = net::PeerAddress::FromString(server).get();

	int port = 30120;

	std::string ip;
	std::array<char, 80> buffer;

	switch (addr.GetAddressFamily())
	{
	case AF_INET:
	{
		auto sa = (const sockaddr_in*)addr.GetSocketAddress();
		inet_ntop(AF_INET, &sa->sin_addr, buffer.data(), buffer.size());

		ip = buffer.data();

		port = htons(sa->sin_port);
		break;
	}
	case AF_INET6:
	{
		auto sa = (const sockaddr_in6*)addr.GetSocketAddress();
		inet_ntop(AF_INET6, &sa->sin6_addr, buffer.data(), buffer.size());

		port = htons(sa->sin6_port);

		ip = std::string("[") + buffer.data() + "]";
		break;
	}
	}

	httpClient->DoGetRequest(ToWide(ip), port, L"/info.json", [=] (bool success, const char* data, size_t length)
	{
		if (!success)
		{
			cb("{}");
			return;
		}

		rapidjson::Document doc;
		doc.Parse(data, length);

		if (!doc.HasParseError())
		{
			auto member = doc.FindMember("version");

			if (member != doc.MemberEnd() && member->value.IsInt())
			{
				rapidjson::StringBuffer sbuffer;
				rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

				doc.Accept(writer);

				cb(sbuffer.GetString());

				uint64_t infoBlobKey = fnv1a_t<8>()(server);
				std::wstring blobPath = MakeRelativeCitPath(fmt::sprintf(L"data\\cache\\servers\\%016llx.json", infoBlobKey));

				FILE* blobFile = _wfopen(blobPath.c_str(), L"w");

				if (blobFile)
				{
					fprintf(blobFile, "%s", sbuffer.GetString());
					fclose(blobFile);
				}
			}
		}
	});
}

template<typename TFunc>
void LoadInfoBlob(const std::string& server, int expectedVersion, const TFunc& cb)
{
	uint64_t infoBlobKey = fnv1a_t<8>()(server);
	std::wstring blobPath = MakeRelativeCitPath(fmt::sprintf(L"data\\cache\\servers\\%016llx.json", infoBlobKey));
	
	FILE* blobFile = _wfopen(blobPath.c_str(), L"r");

	if (blobFile)
	{
		fseek(blobFile, 0, SEEK_END);

		int fOff = ftell(blobFile);

		fseek(blobFile, 0, SEEK_SET);

		std::vector<char> blob(fOff);
		fread(&blob[0], 1, blob.size(), blobFile);

		fclose(blobFile);

		rapidjson::Document doc;
		doc.Parse(blob.data(), blob.size());

		if (!doc.HasParseError())
		{
			auto member = doc.FindMember("version");

			if (member != doc.MemberEnd() && member->value.IsInt() && member->value.GetInt() == expectedVersion)
			{
				cb(std::string(blob.begin(), blob.end()));
				return;
			}
		}
	}

	RequestInfoBlob(server, cb);
}

struct gameserveritemext_t
{
	net::PeerAddress m_Address;
	DWORD queryTime;
	bool responded;
	bool queried;
	std::string m_hostName;
	std::string m_qa;
	std::string m_attribution;
	int m_clients;
	int m_maxClients;
	int m_ping;
};

static struct
{
	SOCKET socket;
	SOCKET socket6;

	HANDLE event;
	HANDLE event6;

	HANDLE hTimer;

	// the list relies on sorting, so using a concurrent_unordered_map won't work
	std::recursive_mutex serversMutex;
	std::map<std::tuple<int, net::PeerAddress, std::string, std::string>, std::shared_ptr<gameserveritemext_t>> queryServers;
	std::map<net::PeerAddress, std::shared_ptr<gameserveritemext_t>> servers;
	DWORD lastQueryStep;

	bool isOneQuery;
	net::PeerAddress oneQueryAddress;
} g_cls;

static bool InitSocket(SOCKET* sock, HANDLE* event, int af)
{
	auto socket = ::socket(af, SOCK_DGRAM, IPPROTO_UDP);

	if (socket == INVALID_SOCKET)
	{
		trace("socket() failed - %d\n", WSAGetLastError());
		return false;
	}

	if (af == AF_INET)
	{
		sockaddr_in bindAddr;
		memset(&bindAddr, 0, sizeof(bindAddr));
		bindAddr.sin_family = af;
		bindAddr.sin_port = 0;
		bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

		if (bind(socket, (sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
		{
			trace("bind() failed - %d\n", WSAGetLastError());
			return false;
		}
	}
	else
	{
		sockaddr_in6 bindAddr;
		memset(&bindAddr, 0, sizeof(bindAddr));
		bindAddr.sin6_family = af;
		bindAddr.sin6_port = 0;
		bindAddr.sin6_addr = in6addr_any;

		if (bind(socket, (sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
		{
			trace("bind() failed - %d\n", WSAGetLastError());
			return false;
		}
	}

	ULONG nonBlocking = 1;
	ioctlsocket(socket, FIONBIO, &nonBlocking);

	setsockopt(socket, SOL_SOCKET, SO_BROADCAST, (char*)&nonBlocking, sizeof(nonBlocking));

	*sock = socket;

	*event = WSACreateEvent();
	WSAEventSelect(socket, *event, FD_READ);

	return true;
}

bool GSClient_Init()
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (err)
	{
		return false;
	}

	g_cls.hTimer = CreateWaitableTimerW(NULL, FALSE, NULL);

	if (!InitSocket(&g_cls.socket, &g_cls.event, AF_INET))
	{
		return false;
	}

	if (!InitSocket(&g_cls.socket6, &g_cls.event6, AF_INET6))
	{
		return false;
	}

	return true;
}

void GSClient_QueryServer(gameserveritemext_t& server)
{
	server.queried = true;
	server.responded = false;
	server.queryTime = timeGetTime();

	const sockaddr* addr = server.m_Address.GetSocketAddress();
	int addrlen = server.m_Address.GetSocketAddressLength();

	auto socket = (server.m_Address.GetAddressFamily() == AF_INET6) ? g_cls.socket6 : g_cls.socket;
	
	char message[128];
	strcpy(message, "\xFF\xFF\xFF\xFFgetinfo xxx");

	sendto(socket, message, strlen(message), 0, (sockaddr*)addr, addrlen);
}

static std::unique_ptr<ConVar<int>> ui_maxQueriesPerMinute;

void GSClient_QueryStep()
{
	int queriesPerStep = round(ui_maxQueriesPerMinute->GetValue() / 60.0f / (1000.0f / 250.0f));

	g_cls.lastQueryStep = timeGetTime();

	int count = 0;

	std::unique_lock lock(g_cls.serversMutex);

	for (auto& serverPair : g_cls.queryServers)
	{
		if (!serverPair.second->queried)
		{
			if (count < queriesPerStep)
			{
				GSClient_QueryServer(*serverPair.second);
				count++;
			}
		}
	}

	if (count == 0)
	{
		CancelWaitableTimer(g_cls.hTimer);
	}
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

void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

static std::mutex g_queryArgMutex;
static std::string g_queryArg;

// the original query argument, used for correlation
static std::string g_queryArgOrig;

void GSClient_HandleInfoResponse(const char* bufferx, int len, const net::PeerAddress& from)
{
	std::shared_ptr<gameserveritemext_t> server;

	{
		std::unique_lock<std::recursive_mutex> lock(g_cls.serversMutex);
		server = g_cls.servers[from];
	}

	if (!server)
	{
		return;
	}

	bufferx++;

	char buffer[8192];
	strcpy(buffer, bufferx);

	server->m_ping = timeGetTime() - server->queryTime;
	server->m_maxClients = atoi(Info_ValueForKey(buffer, "sv_maxclients"));
	server->m_clients = atoi(Info_ValueForKey(buffer, "clients"));
	server->m_hostName = Info_ValueForKey(buffer, "hostname");

	server->responded = true;

	replaceAll(server->m_hostName, "\"", "\\\"");

	const char* mapnameStr = Info_ValueForKey(buffer, "mapname");
	const char* gametypeStr = Info_ValueForKey(buffer, "gametype");

	std::string mapname;
	std::string gametype;

	if (mapnameStr)
	{
		mapname = mapnameStr;
	}

	if (gametypeStr)
	{
		gametype = gametypeStr;
	}

	replaceAll(mapname, "\"", "\\\"");
	replaceAll(gametype, "\"", "\\\"");

	std::string addressStr = server->m_Address.ToString();

	const char* infoBlobVersionString = Info_ValueForKey(buffer, "iv");
	bool isThisOneQuery = !server->m_qa.empty();

	auto onLoadCB = [=](const std::string& infoBlobJson)
	{
		rapidjson::Document doc;
		doc.Parse(infoBlobJson.c_str(), infoBlobJson.size());

		std::string host, port;

		if (!server->m_qa.empty())
		{
			host = server->m_qa.substr(0, server->m_qa.find_last_of(':'));
			port = server->m_qa.substr(server->m_qa.find_last_of(':') + 1);
		}

		if (!doc.HasParseError() && nui::HasFrame("mpMenu"))
		{
			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "%s", "name": "%s",)"
				R"("mapname": "%s", "gametype": "%s", "clients": "%d", "maxclients": %d, "ping": %d,)"
				R"("addr": "%s", "infoBlob": %s, "address": "%s", "port": "%s", "queryCorrelation": "%s" })",
				(isThisOneQuery) ? "serverQueried" : "serverAdd",
				server->m_hostName,
				mapname,
				gametype,
				server->m_clients,
				server->m_maxClients,
				server->m_ping,
				addressStr,
				infoBlobJson,
				host,
				port,
				server->m_attribution));
		}
	};

	if (isThisOneQuery && infoBlobVersionString && infoBlobVersionString[0])
	{
		std::string serverId = fmt::sprintf("%s", addressStr);
		int infoBlobVersion = atoi(infoBlobVersionString);

		LoadInfoBlob(serverId, infoBlobVersion, onLoadCB);
	}
	else
	{
		onLoadCB("{}");
	}
}

typedef struct
{
	unsigned char ip[4];
	unsigned short port;
} serverAddress_t;

#define CMD_GSR "getserversResponse"
#define CMD_INFO "infoResponse"

void GSClient_HandleOOB(const char* buffer, size_t len, const net::PeerAddress& from)
{
	if (!_strnicmp(buffer, CMD_INFO, strlen(CMD_INFO)))
	{
		GSClient_HandleInfoResponse(&buffer[strlen(CMD_INFO)], len - strlen(CMD_INFO), from);
	}
}

void GSClient_PollSocket(SOCKET socket, HANDLE event)
{
	char buf[2048];
	memset(buf, 0, sizeof(buf));

	sockaddr_storage from;
	memset(&from, 0, sizeof(from));

	int fromlen = sizeof(from);

	while (true)
	{
		int len = recvfrom(socket, buf, 2048, 0, (sockaddr*)&from, &fromlen);
		WSAResetEvent(event);

		if (len == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error != WSAEWOULDBLOCK)
			{
				console::DPrintf("nui:gsclient", "recv() failed - %d\n", error);
			}

			return;
		}

		if (*(int*)buf == -1)
		{
			if (len < sizeof(buf))
			{
				buf[len] = '\0';
			}

			GSClient_HandleOOB(&buf[4], size_t(len) - 4, net::PeerAddress((sockaddr*)&from, fromlen));
		}
	}
}

void GSClient_RunFrame()
{
	if (g_cls.socket)
	{
		GSClient_QueryStep();
		GSClient_PollSocket(g_cls.socket, g_cls.event);
		GSClient_PollSocket(g_cls.socket6, g_cls.event6);
	}
}

void GSClient_QueryMaster()
{
	static bool lookedUp;
	static sockaddr_in masterIP;

	char message[128];
	_snprintf(message, sizeof(message), "\xFF\xFF\xFF\xFFgetservers " GS_GAMENAME " 4 full empty");

	sendto(g_cls.socket, message, strlen(message), 0, (sockaddr*)&masterIP, sizeof(masterIP));

	for (int i = 30120; i < 30120 + 6; i++)
	{
		sockaddr_in broadcastIP = { 0 };
		broadcastIP.sin_family = AF_INET;
		broadcastIP.sin_port = htons(i);
		broadcastIP.sin_addr.s_addr = INADDR_BROADCAST;

		_snprintf(message, sizeof(message), "\xFF\xFF\xFF\xFFgetinfo xxx");
		sendto(g_cls.socket, message, strlen(message), 0, (sockaddr*)&broadcastIP, sizeof(broadcastIP));
	}
}

void GSClient_Refresh()
{
	if (!g_cls.socket)
	{
		GSClient_Init();
	}

	GSClient_QueryMaster();
}

template<typename TContainer>
void GSClient_QueryAddresses(const TContainer& addrs)
{
	if (!g_cls.socket)
	{
		GSClient_Init();
	}

	for (const auto& na : addrs)
	{
		auto server = std::make_shared<gameserveritemext_t>();
		server->queried = false;
		server->m_Address = std::get<net::PeerAddress>(na);
		server->m_qa = std::get<2>(na);
		server->m_attribution = std::get<3>(na);

		std::unique_lock<std::recursive_mutex> lock(g_cls.serversMutex);
		g_cls.queryServers[na] = server;
		g_cls.servers[server->m_Address] = server;
	}

	LARGE_INTEGER dueTime;
	dueTime.QuadPart = int64_t(-100) * 10000;
	SetWaitableTimer(g_cls.hTimer, &dueTime, 250, NULL, NULL, FALSE);
}

static void ContinueLanQuery(const std::string& qarg, const std::string& attribution)
{
	auto peerAddress = net::PeerAddress::FromString(qarg);

	if (peerAddress)
	{
		g_cls.isOneQuery = true;
		g_cls.oneQueryAddress = peerAddress.get();
		GSClient_QueryAddresses(std::vector<std::tuple<int, net::PeerAddress, std::string, std::string>>{ { 0, peerAddress.get(), qarg, attribution } });
	}
	else
	{
		nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "queryingFailed", "arg": "%s" })", qarg));
	}
}

static bool g_inLanQuery;

void GSClient_QueryOneServer(const std::wstring& arg)
{
	auto narrowArg = ToNarrow(arg);

	auto processQuery = [narrowArg](const std::string& url)
	{
		Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("%sdynamic.json", url), [url, narrowArg](bool success, const char* data, size_t size)
		{
			if (success)
			{
				std::string dynamicBlob{ data, size };

				Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("%sinfo.json", url), [narrowArg, dynamicBlob](bool success, const char* data, size_t size)
				{
					if (success)
					{
						std::string infoBlobJson{ data, size };

						rapidjson::Document dynDoc;
						dynDoc.Parse(dynamicBlob.c_str(), dynamicBlob.size());

						rapidjson::Document doc;
						doc.Parse(infoBlobJson.c_str(), infoBlobJson.size());

						if (!doc.HasParseError() && !dynDoc.HasParseError() && dynDoc.IsObject() && nui::HasFrame("mpMenu"))
						{
							std::string hostname = (dynDoc.HasMember("hostname") && dynDoc["hostname"].IsString()) ? dynDoc["hostname"].GetString() : "";
							std::string mapname = (dynDoc.HasMember("mapname") && dynDoc["mapname"].IsString()) ? dynDoc["mapname"].GetString() : "";
							std::string gametype = (dynDoc.HasMember("gametype") && dynDoc["gametype"].IsString()) ? dynDoc["gametype"].GetString() : "";

							replaceAll(hostname, "\"", "\\\"");
							replaceAll(mapname, "\"", "\\\"");
							replaceAll(gametype, "\"", "\\\"");

							nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "%s", "name": "%s",)"
																		 R"("mapname": "%s", "gametype": "%s", "clients": "%d", "maxclients": %d, "ping": %d,)"
																		 R"("addr": "%s", "infoBlob": %s, "queryCorrelation": "%s" })",
															"serverQueried",
															hostname,
															mapname,
															gametype,
															(dynDoc.HasMember("clients") && dynDoc["clients"].IsNumber()) ? dynDoc["clients"].GetInt() : 0,
															(dynDoc.HasMember("sv_maxclients") && dynDoc["sv_maxclients"].IsString()) ? atoi(dynDoc["sv_maxclients"].GetString()) : 0,
															42,
															narrowArg,
															infoBlobJson,
															narrowArg));
						}
					}
				});
			}
		});
	};

	if (narrowArg.find("cfx.re/join") != std::string::npos)
	{
		HttpRequestOptions ro;
		ro.responseHeaders = std::make_shared<HttpHeaderList>();

		Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("https://%s", narrowArg.substr(0, narrowArg.find_last_of(':'))), ro, [ro, processQuery](bool success, const char* data, size_t callback)
		{
			if (success)
			{
				const auto& rh = *ro.responseHeaders;

				if (auto it = rh.find("X-CitizenFX-Url"); it != rh.end() && it->second != "https://private-placeholder.cfx.re/")
				{
					auto url = it->second;

					processQuery(url);
				}
			}
		});

		return;
	}

	if (narrowArg.find("https://") == 0)
	{
		processQuery(narrowArg + ((narrowArg[narrowArg.length() - 1] == '/') ? "" : "/"));
		return;
	}

	if (narrowArg.find("localhost_sentinel") == 0)
	{
		if (g_inLanQuery)
		{
			return;
		}

		g_inLanQuery = true;

		std::string qa;
		std::string qao;

		{
			std::unique_lock _(g_queryArgMutex);
			g_queryArg = "localhost" + narrowArg.substr(strlen("localhost_sentinel"));
			g_queryArgOrig = narrowArg;

			qa = g_queryArg;
			qao = g_queryArgOrig;
		}

		// try hunting down a LAN server, maybe?
		static auto dnslib = LoadLibraryW(L"dnsapi.dll");

		if (dnslib)
		{
			static auto _DnsServiceBrowse = (decltype(&DnsServiceBrowse))GetProcAddress(dnslib, "DnsServiceBrowse");
			static auto _DnsServiceBrowseCancel = (decltype(&DnsServiceBrowseCancel))GetProcAddress(dnslib, "DnsServiceBrowseCancel");

			if (_DnsServiceBrowse)
			{
				static bool result = false;
				result = false;

				static tbb::concurrent_unordered_set<std::string> qaSeen;
				qaSeen.clear();

				DNS_SERVICE_BROWSE_REQUEST request = { 0 };
				request.Version = DNS_QUERY_REQUEST_VERSION1;
				request.InterfaceIndex = 0;
				request.QueryName = L"_cfx._udp.local";
				request.pBrowseCallback = [](DWORD Status,
										  PVOID pQueryContext,
										  PDNS_RECORD pDnsRecord)
				{
					std::string qa, qao;

					if (Status == ERROR_SUCCESS)
					{
						result = true;

						if (pDnsRecord)
						{
							for (auto rec = pDnsRecord; rec; rec = rec->pNext)
							{
								if (rec->wType == DNS_TYPE_SRV)
								{
									{
										std::unique_lock _(g_queryArgMutex);
										qa = g_queryArg = fmt::sprintf("%s:%d", ToNarrow(rec->Data.Srv.pNameTarget), rec->Data.Srv.wPort);
										qao = g_queryArgOrig;
									}

									if (!qaSeen.contains(qa))
									{
										qaSeen.insert(qa);

										ContinueLanQuery(qa, qao);
										break;
									}
								}
							}

							DnsRecordListFree(pDnsRecord, DnsFreeRecordList);
							return;
						}
					}

					{
						std::unique_lock _(g_queryArgMutex);
						qa = g_queryArg;
						qao = g_queryArgOrig;
					}

					if (!qaSeen.contains(qa))
					{
						qaSeen.insert(qa);

						ContinueLanQuery(qa, qao);
					}
				};

				static DNS_SERVICE_CANCEL cancel;

				if (_DnsServiceBrowse(&request, &cancel) == DNS_REQUEST_PENDING)
				{
					Sleep(2000);
					g_inLanQuery = false;

					_DnsServiceBrowseCancel(&cancel);

					if (result)
					{
						return;
					}
				}
			}
		}

		g_inLanQuery = false;

		ContinueLanQuery(qa, qao);
		return;
	}

	ContinueLanQuery(narrowArg, narrowArg);
}

DWORD WINAPI GSClient_QueryOneServerWrap(LPVOID ctx)
{
	auto str = (std::wstring*)ctx;
	GSClient_QueryOneServer(*str);
	delete str;

	return 0;
}

void GSClient_Ping(const std::wstring& arg)
{
	g_cls.isOneQuery = false;

	std::string serverArray = ToNarrow(arg);

	rapidjson::Document doc;
	doc.Parse(serverArray.c_str(), serverArray.size());

	if (doc.HasParseError())
	{
		return;
	}

	if (!doc.IsArray())
	{
		return;
	}

	std::vector<std::tuple<int, net::PeerAddress, std::string, std::string>> addresses;

	for (auto it = doc.Begin(); it != doc.End(); it++)
	{
		if (!it->IsArray())
		{
			continue;
		}

		if (it->Size() != 2 && it->Size() != 3)
		{
			continue;
		}

		auto& addrVal = (*it)[0];
		auto& portVal = (*it)[1];

		auto weight = 0;

		if (it->Size() == 3)
		{
			auto& weightVal = (*it)[2];

			if (weightVal.IsInt())
			{
				weight = weightVal.GetInt();
			}
		}

		if (!addrVal.IsString() || !portVal.IsInt())
		{
			continue;
		}

		auto addr = addrVal.GetString();
		auto port = portVal.GetInt();

		auto netAddr = net::PeerAddress::FromString(addr, port);
		addresses.push_back({ 1000 - weight, netAddr.get(), "", "" });
	}

	GSClient_QueryAddresses(addresses);
}

void GSClient_GetFavorites()
{
	std::ifstream favFile(MakeRelativeCitPath(L"favorites.json"));
	std::string json;
	favFile >> json;
	favFile.close();

	if (json.empty())
	{
		json = "[]";
	}

	nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "getFavorites", "list": %s })", json));
}

void GSClient_SaveFavorites(const wchar_t *json)
{
	std::wofstream favFile(MakeRelativeCitPath(L"favorites.json"));
	favFile << json;
	favFile.close();
}

static InitFunction initFunction([] ()
{
	nui::RPCHandlerManager* rpcHandlerManager = Instance<nui::RPCHandlerManager>::Get();
	rpcHandlerManager->RegisterEndpoint("gsclient", [](std::string functionName, std::string arguments, std::map<std::string, std::string> postMap, nui::RPCHandlerManager::TCallbackFn cb)
	{
		if (!nui::HasMainUI())
		{
			cb("null");
			return;
		}

		if (functionName == "url" || functionName == "dynamic")
		{
			auto urlEntry = postMap["url"];

			struct Context
			{
				decltype(cb) cb;
				decltype(urlEntry) url;
				decltype(functionName) functionName;
			};

			QueueUserWorkItem([](LPVOID cxt) -> DWORD
			{
				auto c = (Context*)cxt;

				// if not starting with 'http'
				if (c->url.find("http") != 0)
				{
					auto peerAddress = net::PeerAddress::FromString(c->url, 30120, net::PeerAddress::LookupType::ResolveWithService);

					if (peerAddress)
					{
						c->url = "https://" + peerAddress->ToString();
					}
				}

				if (c->functionName == "dynamic")
				{
					c->url += "/dynamic.json";
				}

				// request it
				HttpRequestOptions options;
				auto rhl = std::make_shared<HttpHeaderList>();
				options.responseHeaders = rhl;
				options.followLocation = false;

				Instance<HttpClient>::Get()->DoGetRequest(c->url, options, [rhl, c](bool success, const char* data, size_t length)
				{
					if (c->functionName == "dynamic")
					{
						auto cb = std::move(c->cb);
						delete c;

						if (success)
						{
							cb(std::string(data, length));
						}
						else
						{
							cb("null");
						}

						return;
					}

					auto cb = std::move(c->cb);
					delete c;

					if (auto it = rhl->find("location"); it != rhl->end())
					{
						if (it->second.find("https://cfx.re/join/") == 0)
						{
							cb(it->second.substr(20));
						}
						else
						{
							cb(it->second);
						}

						return;
					}

					cb("");
				});

				return 0;
			},
			new Context{ cb, urlEntry, functionName }, 0);

			return;
		}

		cb("null");
	});

	ui_maxQueriesPerMinute = std::make_unique<ConVar<int>>("ui_maxQueriesPerMinute", ConVar_Archive, 5000);

	CreateDirectory(MakeRelativeCitPath(L"data\\cache\\servers\\").c_str(), nullptr);

	nui::OnInvokeNative.Connect([] (const wchar_t* type, const wchar_t* arg)
	{
		if (!_wcsicmp(type, L"refreshServers"))
		{
			trace("Refreshing server list...\n");

			GSClient_Refresh();
		}

		if (!_wcsicmp(type, L"pingServers"))
		{
			GSClient_Ping(arg);
		}

		if (!_wcsicmp(type, L"queryServer"))
		{
			console::DPrintf("nui:gsclient", "Pinging specified server...\n");

			QueueUserWorkItem(GSClient_QueryOneServerWrap, new std::wstring(arg), 0);
		}

		if (!_wcsicmp(type, L"getFavorites"))
		{
			GSClient_GetFavorites();
		}

		if (!_wcsicmp(type, L"saveFavorites"))
		{
			GSClient_SaveFavorites(arg);
		}
	});

	std::thread([] ()
	{
		while (true)
		{
			if (!g_cls.hTimer)
			{
				Sleep(500);
				continue;
			}

			HANDLE handles[] =
			{
				g_cls.hTimer,
				g_cls.event,
				g_cls.event6,
			};

			WaitForMultipleObjects(std::size(handles), handles, FALSE, INFINITE);

			GSClient_RunFrame();
		}
	}).detach();
});
