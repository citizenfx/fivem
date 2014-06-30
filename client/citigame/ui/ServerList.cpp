#include "StdInc.h"
#include "CefOverlay.h"
#include <mmsystem.h>
#include <WS2tcpip.h>

struct gameserveritemext_t
{
	DWORD m_IP;
	WORD m_Port;
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
	sockaddr_in from;
	gameserveritemext_t servers[8192];
	int numServers;
	DWORD lastQueryStep;

	int curNumResults;

	DWORD queryTime;
} g_cls;

bool GSClient_Init()
{
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (err)
	{
		return false;
	}

	g_cls.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (g_cls.socket == INVALID_SOCKET)
	{
		trace("socket() failed - %d\n", WSAGetLastError());
		return false;
	}

	sockaddr_in bindAddr;
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_port = 0;
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(g_cls.socket, (sockaddr*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		trace("bind() failed - %d\n", WSAGetLastError());
		return false;
	}

	ULONG nonBlocking = 1;
	ioctlsocket(g_cls.socket, FIONBIO, &nonBlocking);

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

	sockaddr_in serverIP;
	serverIP.sin_family = AF_INET;
	serverIP.sin_addr.s_addr = htonl(server->m_IP);
	serverIP.sin_port = htons(server->m_Port);

	char message[128];
	_snprintf(message, sizeof(message), "\xFF\xFF\xFF\xFFgetinfo xxx");

	sendto(g_cls.socket, message, strlen(message), 0, (sockaddr*)&serverIP, sizeof(serverIP));
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

void GSClient_HandleInfoResponse(const char* bufferx, int len)
{
	trace("received infoResponse\n");

	for (int i = 0; i < g_cls.numServers; i++)
	{
		gameserveritemext_t* server = &g_cls.servers[i];

		if ((server->m_IP == ntohl(g_cls.from.sin_addr.s_addr)) && (server->m_Port == ntohs(g_cls.from.sin_port)))
		{
			bufferx++;

			char buffer[8192];
			strcpy(buffer, bufferx);

			g_cls.queryTime = timeGetTime();

			trace("received *matching* infoResponse - %d\n", i);

			// filter odd characters out of the result
			int len = strlen(buffer);

			for (int i = 0; i < len; i++)
			{
				char thisChar = buffer[i];

				if (thisChar < ' ' || thisChar > '~')
				{
					buffer[i] = ' ';
				}
			}

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

			server->m_IP = htonl(server->m_IP);

			replaceAll(server->m_hostName, "'", "\\'");

			char address[32];
			inet_ntop(AF_INET, &server->m_IP, address, sizeof(address));
			
			nui::ExecuteRootScript(va("citFrames['mpMenu'].contentWindow.postMessage({ type: 'serverAdd', name: '%s', clients: %d, maxclients: %d, ping: %d, addr: '%s:%d' }, '*');", server->m_hostName.c_str(), server->m_clients, server->m_maxClients, server->m_ping, address, server->m_Port));

			break;
		}
	}
}

typedef struct
{
	unsigned char ip[4];
	unsigned short port;
} serverAddress_t;

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

#define CMD_GSR "getserversResponse"
#define CMD_INFO "infoResponse"

void GSClient_HandleOOB(const char* buffer, size_t len)
{
	if (!_strnicmp(buffer, CMD_GSR, strlen(CMD_GSR)))
	{
		GSClient_HandleServersResponse(&buffer[strlen(CMD_GSR)], len - strlen(CMD_GSR));
	}

	if (!_strnicmp(buffer, CMD_INFO, strlen(CMD_INFO)))
	{
		GSClient_HandleInfoResponse(&buffer[strlen(CMD_INFO)], len - strlen(CMD_INFO));
	}
}

void GSClient_PollSocket()
{
	char buf[2048];
	memset(buf, 0, sizeof(buf));

	sockaddr_in from;
	memset(&from, 0, sizeof(from));

	int fromlen = sizeof(from);

	while (true)
	{
		int len = recvfrom(g_cls.socket, buf, 2048, 0, (sockaddr*)&from, &fromlen);

		if (len == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error != WSAEWOULDBLOCK)
			{
				trace("recv() failed - %d\n", error);
			}

			return;
		}

		g_cls.from = from;

		if (*(int*)buf == -1)
		{
			GSClient_HandleOOB(&buf[4], len - 4);
		}
	}
}

void GSClient_RunFrame()
{
	if (g_cls.socket)
	{
		GSClient_QueryStep();
		GSClient_PollSocket();
	}
}

void GSClient_QueryMaster()
{
	static bool lookedUp;
	static sockaddr_in masterIP;

	g_cls.queryTime = timeGetTime() + 15000;//(0xFFFFFFFF - 50000);

	g_cls.numServers = 0;

	if (!lookedUp)
	{
		hostent* host = gethostbyname("refint.org");

		if (!host)
		{
			trace("gethostbyname() failed - %d\n", WSAGetLastError());
			return;
		}

		masterIP.sin_family = AF_INET;
		masterIP.sin_addr.s_addr = *(ULONG*)host->h_addr_list[0];
		masterIP.sin_port = htons(30110);

		lookedUp = true;
	}

	char message[128];
	_snprintf(message, sizeof(message), "\xFF\xFF\xFF\xFFgetservers GTA4 2 full empty");

	sendto(g_cls.socket, message, strlen(message), 0, (sockaddr*)&masterIP, sizeof(masterIP));
}

void GSClient_Refresh()
{
	if (!g_cls.socket)
	{
		GSClient_Init();
	}

	GSClient_QueryMaster();
}