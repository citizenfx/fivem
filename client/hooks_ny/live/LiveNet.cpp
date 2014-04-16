#include "StdInc.h"
#include "Live.h"

static SOCKET g_gameSocket;

SOCKET __stdcall XCreateSocket(int af, int type, int protocol)
{
	bool wasVDP = false;

	if (protocol == 254)
	{
		protocol = IPPROTO_UDP;
		wasVDP = true;
	}

	SOCKET sock = socket(af, type, protocol);

	if (wasVDP)
	{
		g_gameSocket = sock;
	}

	return sock;
}

#pragma region dummy calls
int __stdcall XWSAStartup(WORD wVersionRequested, LPWSADATA lpWsaData)
{
	return WSAStartup(wVersionRequested, lpWsaData);;
}

void __stdcall XWSACleanup()
{
	WSACleanup();
}

int __stdcall XSocketClose(SOCKET s)
{
	return closesocket(s);
}

int __stdcall XSocketShutdown(SOCKET s, int how)
{
	return shutdown(s, how);
}

int __stdcall XSocketIOCTLSocket(SOCKET s, long cmd, u_long * argp)
{
	return ioctlsocket(s, cmd, argp);
}

int __stdcall XSocketSetSockOpt(SOCKET s, int level, int optname, const char* optval, int optlen)
{
	return setsockopt(s, level, optname, optval, optlen);
}

int __stdcall XSocketGetSockName(SOCKET s, sockaddr_in * name, int * namelen)
{
	return getsockname(s, (sockaddr*)name, namelen);
}

int __stdcall XSocketBind(SOCKET s, sockaddr * addr, int addrlen)
{
	return bind(s, addr, addrlen);
}

int __stdcall XSocketConnect(SOCKET s, sockaddr * addr, int addrlen)
{
	sockaddr_in* in = (sockaddr_in*)addr;
	return connect(s, addr, addrlen);
}

int __stdcall XSocketListen(SOCKET s, int backlog)
{
	return listen(s, backlog);
}

SOCKET __stdcall XSocketAccept(SOCKET s, sockaddr * addr, int * addrlen)
{
	return accept(s, addr, addrlen);
}

int __stdcall XSocketSelect(int n, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, const struct timeval * timeout)
{
	return select(n, readfds, writefds, exceptfds, timeout);
}

int __stdcall XSocketRecv(SOCKET s, char * buf, int len, int flags)
{
	return recv(s, buf, len, flags);
}

int __stdcall XSocketRecvFrom(SOCKET s, char * buf, int len, int flags, sockaddr * from, int * fromlen)
{
	

	int lens = recvfrom(s, buf, len, flags, from, fromlen);

	return lens;
}

// #22: XSocketSend
int __stdcall XSocketSend(SOCKET s, char * buf, int len, int flags)
{
	return send(s, buf, len, flags);
}

int __stdcall XSocketSendTo(SOCKET s, char * buf, int len, int flags, sockaddr * to, int tolen)
{
	return sendto(s, buf, len, flags, to, tolen);
}

int __stdcall XSocketInet_Addr(char * arg)
{
	return inet_addr(arg);
}

int __stdcall XWSAGetLastError()
{
	return WSAGetLastError();
}

WORD __stdcall XSocketNTOHS(WORD n)
{
	return ((n & 0xFF00) >> 8) | ((n & 0xFF) << 8);
}

DWORD __stdcall XSocketNTOHL(DWORD n)
{
	return ((n & 0xFF000000) >> 24) | ((n & 0x00FF0000) >> 8) | ((n & 0x0000FF00) << 8) | ((n & 0x000000FF) << 24);
}
#pragma endregion

int __stdcall XNetStartup(void *)
{
	return 0;
}

int __stdcall XNetCleanup()
{
	return 0;
}

int __stdcall XNetCreateKey(void * pxnkid, void * pxnkey)
{
	return 0;
}

// #55: XNetRegisterKey
int __stdcall XNetRegisterKey(DWORD, DWORD)
{
	return 0;
}

// #56: XNetUnregisterKey
int __stdcall XNetUnregisterKey(DWORD)
{
	return 0;
}

// #57: XNetXnAddrToInAddr
int __stdcall XNetXnAddrToInAddr(const XNADDR* addr, const XNKID* pxnkid, IN_ADDR* add)
{
	return 0;
}

// #58: XNetServerToInAddr
DWORD __stdcall XNetServerToInAddr(const IN_ADDR ina,
											   DWORD dwServiceId,
											   IN_ADDR *pina
											   )
{
	*pina = ina;
	return 0;
}

// #60: XNetInAddrToXnAddr
extern "C" DWORD __stdcall XNetInAddrToXnAddr(DWORD, DWORD, DWORD)
{
	return 1;
}

// #63: XNetUnregisterInAddr
int __stdcall XNetUnregisterInAddr(DWORD)
{
	return 0;
}

// #65: XNetConnect
extern "C" int __stdcall XNetConnect(DWORD)
{
	return 0;
}

// #66: XNetGetConnectStatus
int __stdcall XNetGetConnectStatus(DWORD)
{
	return 2;	// XNET_CONNECT_STATUS_CONNECTED
}

// #69: XNetQosListen
DWORD __stdcall XNetQosListen(XNKID * pxnkid,
										  BYTE * pb,
										  UINT cb,
										  DWORD dwBitsPerSec,
										  DWORD dwFlags
										  )
{
	return 0;
}

// #70: XNetQosLookup
DWORD __stdcall XNetQosLookup(UINT cxna,
										  XNADDR * apxna[],
										  XNKID * apxnkid[],
										  XNKEY * apxnkey[],
										  UINT cina,
										  IN_ADDR aina[],
										  DWORD adwServiceId[],
										  UINT cProbes,
										  DWORD dwBitsPerSec,
										  DWORD dwFlags,
										  HANDLE hEvent,
										  XNQOS** ppxnqos
										  )
{
	return 0;
}

// #71: XNetQosServiceLookup
extern "C" DWORD __stdcall XNetQosServiceLookup(DWORD, DWORD, DWORD)
{
	return 0;
}

// #72: XNetQosRelease
DWORD __stdcall XNetQosRelease(XNQOS* q)
{
	return 0;
}

// #73: XNetGetTitleXnAddr
static bool setTitle = false;

bool SteamProxy_Init();

DWORD __stdcall XNetGetTitleXnAddr(XNADDR * pAddr)
{
	memset(pAddr, 0, sizeof(*pAddr));

	return 0xE4;
}

DWORD __stdcall XNetGetEthernetLinkStatus()
{
	return 1;
}

DWORD __stdcall XNetSetSystemLinkPort(DWORD)
{
	return 0;
}

static HookFunction hookFunction([] ()
{
	hook::iat("xlive.dll", XCreateSocket, 3);
	hook::iat("xlive.dll", XSocketSend, 22);
	hook::iat("xlive.dll", XSocketSendTo, 24);
	hook::iat("xlive.dll", XSocketRecv, 18);
	hook::iat("xlive.dll", XSocketRecvFrom, 20);
	hook::iat("xlive.dll", XSocketSelect, 15);
	hook::iat("xlive.dll", XSocketSetSockOpt, 7);
	hook::iat("xlive.dll", XSocketBind, 11);
	hook::iat("xlive.dll", XSocketConnect, 12);
	hook::iat("xlive.dll", XSocketListen, 13);
	hook::iat("xlive.dll", XSocketAccept, 14);
	hook::iat("xlive.dll", XWSAStartup, 1);
	hook::iat("xlive.dll", XWSACleanup, 2);
	hook::iat("xlive.dll", XWSAGetLastError, 27);
	hook::iat("xlive.dll", XSocketNTOHL, 39);
	hook::iat("xlive.dll", XSocketNTOHS, 38);
	hook::iat("xlive.dll", XSocketShutdown, 5);
	hook::iat("xlive.dll", XSocketClose, 4);
	hook::iat("xlive.dll", XSocketInet_Addr, 26);
	hook::iat("xlive.dll", XSocketGetSockName, 9);
	hook::iat("xlive.dll", XSocketIOCTLSocket, 6);

	hook::iat("xlive.dll", XNetStartup, 51);
	hook::iat("xlive.dll", XNetCleanup, 52);

	hook::iat("xlive.dll", XNetCreateKey, 54);
	hook::iat("xlive.dll", XNetRegisterKey, 55);
	hook::iat("xlive.dll", XNetUnregisterKey, 56);
	hook::iat("xlive.dll", XNetXnAddrToInAddr, 57);
	hook::iat("xlive.dll", XNetServerToInAddr, 58);
	hook::iat("xlive.dll", XNetInAddrToXnAddr, 60);
	hook::iat("xlive.dll", XNetUnregisterInAddr, 63);
	hook::iat("xlive.dll", XNetConnect, 65);
	hook::iat("xlive.dll", XNetGetConnectStatus, 66);
	hook::iat("xlive.dll", XNetQosListen, 69);
	hook::iat("xlive.dll", XNetQosLookup, 70);
	hook::iat("xlive.dll", XNetQosRelease, 72);

	hook::iat("xlive.dll", XNetGetTitleXnAddr, 73);
	hook::iat("xlive.dll", XNetGetEthernetLinkStatus, 75);
	hook::iat("xlive.dll", XNetSetSystemLinkPort, 84);
});