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
	gameserveritemext_t servers[8192];
	int numServers;
	DWORD lastQueryStep;

	int curNumResults;

	DWORD queryTime;

	bool isOneQuery;
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

gameserveritemext_t* GSClient_ServerItem(int i)
{
	return &g_cls.servers[i];
}

int GSClient_NumServers()
{
	return g_cls.numServers;
}

void GSClient_QueryServer(int i)
{
	gameserveritemext_t* server = &g_cls.servers[i];

	server->queried = true;
	server->responded = false;
	server->queryTime = timeGetTime();

	const sockaddr* addr = server->m_Address.GetSocketAddress();
	int addrlen = server->m_Address.GetSocketAddressLength();

	auto socket = (server->m_Address.GetAddressFamily() == AF_INET6) ? g_cls.socket6 : g_cls.socket;
	
	char message[128];
	_snprintf(message, sizeof(message), "\xFF\xFF\xFF\xFFgetinfo xxx");

	sendto(socket, message, strlen(message), 0, (sockaddr*)addr, addrlen);
}

void GSClient_QueryStep()
{
	if ((GetTickCount() - g_cls.lastQueryStep) < 50)
	{
		return;
	}

	g_cls.lastQueryStep = GetTickCount();

	int count = 0;

	for (int i = 0; i < g_cls.numServers && count < 20; i++)
	{
		if (!g_cls.servers[i].queried)
		{
			GSClient_QueryServer(i);
			count++;
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
	auto tempServer = std::make_shared<gameserveritemext_t>();
	tempServer->queryTime = timeGetTime();
	tempServer->m_Address = from;

	gameserveritemext_t* server = tempServer.get();

	for (int i = 0; i < g_cls.numServers; i++)
	{
		gameserveritemext_t* thisServer = &g_cls.servers[i];

		if (thisServer->m_Address == from)
		{
			server = thisServer;
			break;
		}
	}

	bufferx++;

	char buffer[8192];
	strcpy(buffer, bufferx);

	g_cls.queryTime = timeGetTime();

	if (g_cls.curNumResults > 8192)
	{
		return;
	}

	int j = g_cls.curNumResults;

	g_cls.curNumResults++;

	server->m_ping = timeGetTime() - server->queryTime;
	server->m_maxClients = atoi(Info_ValueForKey(buffer, "sv_maxclients"));
	server->m_clients = atoi(Info_ValueForKey(buffer, "clients"));
	server->m_hostName = Info_ValueForKey(buffer, "hostname");

	replaceAll(server->m_hostName, "'", "\\'");

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

	replaceAll(mapname, "'", "\\'");
	replaceAll(gametype, "'", "\\'");

	std::string addressStr = server->m_Address.ToString();

	const char* infoBlobVersionString = Info_ValueForKey(buffer, "iv");

	auto onLoadCB = [=](const std::string& infoBlobJson)
	{
		rapidjson::Document doc;
		doc.Parse(infoBlobJson.c_str(), infoBlobJson.size());

		bool hasHardCap = true;

		auto it = doc.FindMember("resources");

		if (it != doc.MemberEnd())
		{
			auto& value = it->value;

			if (value.IsArray())
			{
				hasHardCap = false;

				for (auto i = value.Begin(); i != value.End(); ++i)
				{
					if (i->IsString())
					{
						if (strcmp(i->GetString(), "hardcap") == 0)
						{
							hasHardCap = true;
						}
					}
				}
			}
		}

		if (!hasHardCap)
		{
			server->m_clients = 0;
			server->m_ping = 404;
			server->m_hostName += " [BROKEN, DO NOT JOIN - ERROR CODE #53]";
		}

		nui::ExecuteRootScript(fmt::sprintf("citFrames['mpMenu'].contentWindow.postMessage({ type: '%s', name: '%s',"
			"mapname: '%s', gametype: '%s', clients: %d, maxclients: %d, ping: %d,"
			"addr: '%s', infoBlob: %s }, '*');",
			(g_cls.isOneQuery) ? "serverQueried" : "serverAdd",
			server->m_hostName,
			mapname,
			gametype,
			server->m_clients,
			server->m_maxClients,
			server->m_ping,
			addressStr,
			infoBlobJson));

		g_cls.isOneQuery = false;

		tempServer->m_Address = net::PeerAddress();
	};

	if (infoBlobVersionString && infoBlobVersionString[0])
	{
		std::string serverId = fmt::sprintf("%s", addressStr);
		int infoBlobVersion = atoi(infoBlobVersionString);

		LoadInfoBlob(serverId, infoBlobVersion, onLoadCB);
	}
	else
	{
		onLoadCB("{}");
	}

	// have over 60% of servers been shown?
	if (g_cls.curNumResults >= (g_cls.numServers * 0.6))
	{
		nui::ExecuteRootScript("citFrames['mpMenu'].contentWindow.postMessage({ type: 'refreshingDone' }, '*');");
	}
}

typedef struct
{
	unsigned char ip[4];
	unsigned short port;
} serverAddress_t;

#if 0
void GSClient_HandleServersResponse(const char* buffer, int len)
{
	int numservers = 0;
	const char* buffptr = buffer;
	const char* buffend = buffer + len;
	serverAddress_t addresses[256];
	while (buffptr + 1 < buffend)
	{
		// advance to initial token
		do
		{
			if (*buffptr++ == '\\')
				break;
		} while (buffptr < buffend);

		if (buffptr >= buffend - 8)
		{
			break;
		}

		// parse out ip
		addresses[numservers].ip[0] = *buffptr++;
		addresses[numservers].ip[1] = *buffptr++;
		addresses[numservers].ip[2] = *buffptr++;
		addresses[numservers].ip[3] = *buffptr++;

		// parse out port
		addresses[numservers].port = (*(buffptr++)) << 8;
		addresses[numservers].port += (*(buffptr++)) & 0xFF;
		addresses[numservers].port = addresses[numservers].port;

		// syntax check
		if (*buffptr != '\\')
		{
			break;
		}

		numservers++;
		if (numservers >= 256)
		{
			break;
		}

		// parse out EOT
		if (buffptr[1] == 'E' && buffptr[2] == 'O' && buffptr[3] == 'T')
		{
			break;
		}
	}

	int count = g_cls.numServers;
	int max = 8192;

	for (int i = 0; i < numservers && count < max; i++)
	{
		// build net address
		unsigned int ip = (addresses[i].ip[0] << 24) | (addresses[i].ip[1] << 16) | (addresses[i].ip[2] << 8) | (addresses[i].ip[3]);
		//g_cls.servers[count].m_NetAdr.Init(ip, addresses[i].qport, addresses[i].port);
		g_cls.servers[count].m_IP = ip;
		g_cls.servers[count].m_Port = addresses[i].port;
		g_cls.servers[count].queried = false;

		count++;
	}

	g_cls.queryTime = timeGetTime();
	GSClient_QueryStep();

	g_cls.numServers = count;
}
#endif

#define CMD_GSR "getserversResponse"
#define CMD_INFO "infoResponse"

void GSClient_HandleOOB(const char* buffer, size_t len, const net::PeerAddress& from)
{
#if 0
	if (!_strnicmp(buffer, CMD_GSR, strlen(CMD_GSR)))
	{
		GSClient_HandleServersResponse(&buffer[strlen(CMD_GSR)], len - strlen(CMD_GSR));
	}
#endif

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

			GSClient_HandleOOB(&buf[4], len - 4, net::PeerAddress((sockaddr*)&from, fromlen));
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

	g_cls.queryTime = timeGetTime() + 15000;//(0xFFFFFFFF - 50000);

	g_cls.numServers = 0;

	g_cls.curNumResults = 0;

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

	g_cls.numServers = 0;
	g_cls.curNumResults = 0;

	char message[128];
	_snprintf(message, sizeof(message), "\xFF\xFF\xFF\xFFgetinfo xxx");

	for (const net::PeerAddress& na : addrs)
	{
		int count = g_cls.numServers;

		if (count < 8192)
		{
			// build net address
			g_cls.servers[count].m_Address = na;
			g_cls.servers[count].queried = false;
			
			g_cls.numServers++;
		}
	}
}

void GSClient_QueryOneServer(const std::wstring& arg)
{
	auto peerAddress = net::PeerAddress::FromString(ToNarrow(arg));

	if (peerAddress)
	{
		g_cls.isOneQuery = true;
		GSClient_QueryAddresses(std::vector<net::PeerAddress>{ peerAddress.get() });
	}
	else
	{
		nui::ExecuteRootScript(fmt::sprintf("citFrames['mpMenu'].contentWindow.postMessage({ type: 'queryingFailed', arg: '%s' }, '*');", ToNarrow(arg)));
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

	std::vector<net::PeerAddress> addresses;

	for (auto it = doc.Begin(); it != doc.End(); it++)
	{
		if (!it->IsArray())
		{
			continue;
		}

		if (it->Size() != 2)
		{
			continue;
		}

		auto& addrVal = (*it)[0];
		auto& portVal = (*it)[1];

		if (!addrVal.IsString() || !portVal.IsInt())
		{
			continue;
		}

		auto addr = addrVal.GetString();
		auto port = portVal.GetInt();

		auto netAddr = net::PeerAddress::FromString(addr, port);
		addresses.push_back(netAddr.get());
	}

	GSClient_QueryAddresses(addresses);
}

void GSClient_GetFavorites()
{
	std::ifstream favFile(MakeRelativeCitPath(L"favorites.json"));
	std::string json;
	favFile >> json;
	favFile.close();

	nui::ExecuteRootScript(va("citFrames['mpMenu'].contentWindow.postMessage({ type: 'getFavorites', list: %s }, '*');", json));
}

void GSClient_SaveFavorites(const wchar_t *json)
{
	std::wofstream favFile(MakeRelativeCitPath(L"favorites.json"));
	favFile << json;
	favFile.close();
}

static InitFunction initFunction([] ()
{
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
			trace("Pinging specified servers...\n");

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
