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

#include <NetAddress.h> // net:base

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include <fnv.h>

#include <HttpClient.h>

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
				std::wstring blobPath = MakeRelativeCitPath(fmt::sprintf(L"cache\\servers\\%016llx.json", infoBlobKey));

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
	std::wstring blobPath = MakeRelativeCitPath(fmt::sprintf(L"cache\\servers\\%016llx.json", infoBlobKey));
	
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
	int m_clients;
	int m_maxClients;
	int m_ping;
};

static struct
{
	SOCKET socket;
	SOCKET socket6;

	// the list relies on sorting, so using a concurrent_unordered_map won't work
	std::recursive_mutex serversMutex;
	std::map<std::tuple<int, net::PeerAddress>, std::shared_ptr<gameserveritemext_t>> queryServers;
	std::map<net::PeerAddress, std::shared_ptr<gameserveritemext_t>> servers;
	DWORD lastQueryStep;

	bool isOneQuery;
	net::PeerAddress oneQueryAddress;
} g_cls;

static bool InitSocket(SOCKET* sock, int af)
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

	if (!InitSocket(&g_cls.socket, AF_INET))
	{
		return false;
	}

	if (!InitSocket(&g_cls.socket6, AF_INET6))
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
	if ((timeGetTime() - g_cls.lastQueryStep) < 250)
	{
		return;
	}

	int queriesPerStep = round(ui_maxQueriesPerMinute->GetValue() / 60.0f / (1000.0f / 250.0f));

	g_cls.lastQueryStep = timeGetTime();

	int count = 0;

	std::unique_lock<std::recursive_mutex> lock(g_cls.serversMutex);

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

	auto onLoadCB = [=](const std::string& infoBlobJson)
	{
		rapidjson::Document doc;
		doc.Parse(infoBlobJson.c_str(), infoBlobJson.size());

		bool isThisOneQuery = (g_cls.isOneQuery && from == g_cls.oneQueryAddress);

		if (!doc.HasParseError() && nui::HasFrame("mpMenu"))
		{
			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "%s", "name": "%s",)"
				R"("mapname": "%s", "gametype": "%s", "clients": "%d", "maxclients": %d, "ping": %d,)"
				R"("addr": "%s", "infoBlob": %s })",
				(isThisOneQuery) ? "serverQueried" : "serverAdd",
				server->m_hostName,
				mapname,
				gametype,
				server->m_clients,
				server->m_maxClients,
				server->m_ping,
				addressStr,
				infoBlobJson));
		}

		if (isThisOneQuery)
		{
			g_cls.isOneQuery = false;
			g_cls.oneQueryAddress = net::PeerAddress();
		}
	};

	if (g_cls.isOneQuery && infoBlobVersionString && infoBlobVersionString[0])
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

void GSClient_PollSocket(SOCKET socket)
{
	char buf[2048];
	memset(buf, 0, sizeof(buf));

	sockaddr_storage from;
	memset(&from, 0, sizeof(from));

	int fromlen = sizeof(from);

	while (true)
	{
		int len = recvfrom(socket, buf, 2048, 0, (sockaddr*)&from, &fromlen);

		if (len == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error != WSAEWOULDBLOCK)
			{
				trace("recv() failed - %d\n", error);
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
		GSClient_PollSocket(g_cls.socket);
		GSClient_PollSocket(g_cls.socket6);
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

		std::unique_lock<std::recursive_mutex> lock(g_cls.serversMutex);
		g_cls.queryServers[na] = server;
		g_cls.servers[server->m_Address] = server;
	}
}

void GSClient_QueryOneServer(const std::wstring& arg)
{
	auto narrowArg = ToNarrow(arg);

	if (narrowArg.find("cfx.re/join") != std::string::npos)
	{
		HttpRequestOptions ro;
		ro.responseHeaders = std::make_shared<HttpHeaderList>();

		Instance<HttpClient>::Get()->DoGetRequest(fmt::sprintf("https://%s", narrowArg.substr(0, narrowArg.find_last_of(':'))), ro, [ro, narrowArg](bool success, const char* data, size_t callback)
		{
			if (success)
			{
				const auto& rh = *ro.responseHeaders;

				if (rh.find("X-CitizenFX-Url") != rh.end())
				{
					auto url = rh.find("X-CitizenFX-Url")->second;

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

									if (!doc.HasParseError() && !dynDoc.HasParseError() && nui::HasFrame("mpMenu"))
									{
										std::string hostname = dynDoc["hostname"].GetString();
										std::string mapname = dynDoc["mapname"].GetString();
										std::string gametype = dynDoc["gametype"].GetString();

										replaceAll(hostname, "\"", "\\\"");
										replaceAll(mapname, "\"", "\\\"");
										replaceAll(gametype, "\"", "\\\"");

										nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "%s", "name": "%s",)"
											R"("mapname": "%s", "gametype": "%s", "clients": "%d", "maxclients": %d, "ping": %d,)"
											R"("addr": "%s", "infoBlob": %s })",
											"serverQueried",
											hostname,
											mapname,
											gametype,
											dynDoc["clients"].GetInt(),
											atoi(dynDoc["sv_maxclients"].GetString()),
											42,
											narrowArg,
											infoBlobJson));
									}
								}
							});
						}
					});

					return;
				}
			}
		});

		return;
	}

	auto peerAddress = net::PeerAddress::FromString(narrowArg);

	if (peerAddress)
	{
		g_cls.isOneQuery = true;
		g_cls.oneQueryAddress = peerAddress.get();
		GSClient_QueryAddresses(std::vector<std::tuple<int, net::PeerAddress>>{ { 0, peerAddress.get() } });
	}
	else
	{
		nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "queryingFailed", "arg": "%s" })", ToNarrow(arg)));
	}
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

	std::vector<std::tuple<int, net::PeerAddress>> addresses;

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
		addresses.push_back({ 1000 - weight, netAddr.get() });
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
	ui_maxQueriesPerMinute = std::make_unique<ConVar<int>>("ui_maxQueriesPerMinute", ConVar_Archive, 5000);

	CreateDirectory(MakeRelativeCitPath(L"cache\\servers\\").c_str(), nullptr);

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
			trace("Pinging specified server...\n");

			GSClient_QueryOneServer(arg);
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
			Sleep(1);

			GSClient_RunFrame();
		}
	}).detach();
});
