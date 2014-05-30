#include "StdInc.h"
#include "NetLibrary.h"
#include "GameInit.h"
#include "DownloadMgr.h"
#include <yaml-cpp/yaml.h>
#include <libnp.h>

uint16_t NetLibrary::GetServerNetID()
{
	return m_serverNetID;
}

uint16_t NetLibrary::GetHostNetID()
{
	return m_hostNetID;
}

void NetLibrary::ProcessPackets()
{
	char buf[2048];
	memset(buf, 0, sizeof(buf));

	sockaddr_in from;
	memset(&from, 0, sizeof(from));

	int fromlen = sizeof(from);

	while (true)
	{
		int len = recvfrom(m_socket, buf, 2048, 0, (sockaddr*)&from, &fromlen);

		NetAddress fromAddr(&from);

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
			ProcessOOB(fromAddr, &buf[4], len - 4);
		}
		else
		{
			if (fromAddr != m_currentServer)
			{
				trace("invalid from address for server msg\n");
				return;
			}

			NetBuffer* msg;

			if (m_netChannel.Process(buf, len, &msg))
			{
				ProcessServerMessage(*msg);

				delete msg;
			}
		}
	}
}

void NetLibrary::ProcessServerMessage(NetBuffer& msg)
{
	// update received-at time
	m_lastReceivedAt = GetTickCount();

	// receive the message
	uint32_t msgType;
	
	uint32_t curReliableAck = msg.Read<uint32_t>();

	if (curReliableAck != m_outReliableAcknowledged)
	{
		for (auto it = m_outReliableCommands.begin(); it != m_outReliableCommands.end();)
		{
			if (it->id <= curReliableAck)
			{
				it = m_outReliableCommands.erase(it);
			}
			else
			{
				it++;
			}
		}

		m_outReliableAcknowledged = curReliableAck;
	}

	if (m_connectionState == CS_CONNECTED)
	{
		m_connectionState = CS_ACTIVE;
	}

	if (m_connectionState != CS_ACTIVE)
	{
		return;
	}

	do 
	{
		if (msg.End())
		{
			break;
		}

		msgType = msg.Read<uint32_t>();

		if (msgType == 0xE938445B) // 'msgRoute'
		{
			uint16_t netID = msg.Read<uint16_t>();
			uint16_t rlength = msg.Read<uint16_t>();

			trace("msgRoute from %d len %d\n", netID, rlength);

			char routeBuffer[65536];
			msg.Read(routeBuffer, rlength);

			EnqueueRoutedPacket(netID, std::string(routeBuffer, rlength));
		}
		else if (msgType != 0xCA569E63) // reliable command
		{
			uint32_t id = msg.Read<uint32_t>();
			uint32_t size;

			if (id & 0x80000000)
			{
				size = msg.Read<uint32_t>();
				id &= ~0x80000000;
			}
			else
			{
				size = msg.Read<uint16_t>();
			}

			// test for bad scenarios
			if (id > (m_lastReceivedReliableCommand + 64))
			{
				__asm int 3
			}

			char* reliableBuf = new(std::nothrow) char[size];

			if (!reliableBuf)
			{
				return;
			}

			msg.Read(reliableBuf, size);

			// check to prevent double execution
			if (id > m_lastReceivedReliableCommand)
			{
				HandleReliableCommand(msgType, reliableBuf, size);

				m_lastReceivedReliableCommand = id;
			}

			delete[] reliableBuf;
		}
	} while (msgType != 0xCA569E63); // 'msgEnd'
}

void NetLibrary::EnqueueRoutedPacket(uint16_t netID, std::string packet)
{
	RoutingPacket routePacket;
	routePacket.netID = netID;
	routePacket.payload = packet;

	m_incomingPackets.push(routePacket);
}

bool NetLibrary::DequeueRoutedPacket(char* buffer, size_t* length, uint16_t* netID)
{
	if (m_incomingPackets.empty())
	{
		return false;
	}

	auto packet = m_incomingPackets.front();
	m_incomingPackets.pop();

	memcpy(buffer, packet.payload.c_str(), packet.payload.size());
	*netID = packet.netID;
	*length = packet.payload.size();

	return true;
}

void NetLibrary::RoutePacket(const char* buffer, size_t length, uint16_t netID)
{
	RoutingPacket routePacket;
	routePacket.netID = netID;
	routePacket.payload = std::string(buffer, length);

	m_outgoingPackets.push(routePacket);
}

void NetLibrary::ProcessOOB(NetAddress& from, char* oob, size_t length)
{
	if (from == m_currentServer)
	{
		if (!_strnicmp(oob, "connectOK", 9))
		{
			char* clientNetIDStr = &oob[10];
			char* hostIDStr = strchr(clientNetIDStr, ' ');

			hostIDStr[0] = '\0';
			hostIDStr++;

			char* hostBaseStr = strchr(hostIDStr, ' ');

			hostBaseStr[0] = '\0';
			hostBaseStr++;

			m_serverNetID = atoi(clientNetIDStr);
			m_hostNetID = atoi(hostIDStr);
			m_hostBase = atoi(hostBaseStr);

			m_lastReceivedReliableCommand = 0;

			trace("connectOK, our id %d, host id %d\n", m_serverNetID, m_hostNetID);

			m_netChannel.Reset(m_currentServer, this);
			m_connectionState = CS_CONNECTED;
		}
		else if (!_strnicmp(oob, "error", 5))
		{
			if (from != m_currentServer)
			{
				trace("Received 'error' request was not from the host\n");
				return;
			}

			char* errorStr = &oob[6];

			GlobalError("%s", errorStr);
		}
	}
}

void NetLibrary::SetBase(uint32_t base)
{
	m_serverBase = base;
}

uint32_t NetLibrary::GetHostBase()
{
	return m_hostBase;
}

void NetLibrary::HandleReliableCommand(uint32_t msgType, const char* buf, size_t length)
{
	auto range = m_reliableHandlers.equal_range(msgType);

	std::for_each(range.first, range.second, [&] (std::pair<uint32_t, ReliableHandlerType> handler)
	{
		handler.second(buf, length);
	});
}

void NetLibrary::ProcessSend()
{
	// is it time to send a packet yet?
	uint32_t diff = GetTickCount() - m_lastSend;

	if (diff < (1000 / 40))
	{
		return;
	}

	// do we have data to send?
	if (m_connectionState != CS_ACTIVE)
	{
		return;
	}

	// build a nice packet
	NetBuffer msg(12000);

	msg.Write(m_lastReceivedReliableCommand);

	while (!m_outgoingPackets.empty())
	{
		auto packet = m_outgoingPackets.front();
		m_outgoingPackets.pop();

		msg.Write(0xE938445B); // msgRoute
		msg.Write(packet.netID);
		msg.Write<uint16_t>(packet.payload.size());

		trace("sending msgRoute to %d len %d\n", packet.netID, packet.payload.size());

		msg.Write(packet.payload.c_str(), packet.payload.size());
	}

	// send pending reliable commands
	for (auto& command : m_outReliableCommands)
	{
		msg.Write(command.type);

		if (command.command.size() > UINT16_MAX)
		{
			msg.Write(command.id | 0x80000000);

			msg.Write<uint32_t>(command.command.size());
		}
		else
		{
			msg.Write(command.id);

			msg.Write<uint16_t>(command.command.size());
		}
		
		msg.Write(command.command.c_str(), command.command.size());
	}

	// FIXME: REPLACE HARDCODED STUFF
	if (*(BYTE*)0x18A82FD) // is server running
	{
		msg.Write(0xB3EA30DE); // msgIHost
		msg.Write(m_serverBase);
	}

	msg.Write(0xCA569E63); // msgEnd

	m_netChannel.Send(msg);

	m_lastSend = GetTickCount();
}

void NetLibrary::SendReliableCommand(const char* type, const char* buffer, size_t length)
{
	uint32_t unacknowledged = m_outReliableSequence - m_outReliableAcknowledged;

	if (unacknowledged > MAX_RELIABLE_COMMANDS)
	{
		GlobalError("Reliable client command overflow.");
	}

	m_outReliableSequence++;

	OutReliableCommand cmd;
	cmd.type = HashRageString(type);
	cmd.id = m_outReliableSequence;
	cmd.command = std::string(buffer, length);

	m_outReliableCommands.push_front(cmd);
}

#include "../ui/CefOverlay.h"
static std::shared_ptr<NUIWindow> nuiWindow;

void NetLibrary::RunFrame()
{
	ProcessPackets();

	ProcessSend();

	static bool testState;

	if (GetAsyncKeyState(VK_F4))
	{
		static bool nuiw = false;

		if (!nuiw)
		{
			nuiWindow = NUIWindow::Create(false, 2560, 1440, "nui://game/test.html");

			nuiw = true;
		}
	}

	if (GetAsyncKeyState(VK_F5) && m_connectionState == CS_IDLE)
	{
		if (!testState)
		{
			if (!_stricmp(getenv("computername"), "fallarbor") || !_stricmp(getenv("computername"), "snowpoint"))
			{
				ConnectToServer("192.168.178.83", 30120);
			}
			else
			{
				ConnectToServer("77.22.64.7", 30120);
			}

			testState = true;
		}
	}
	else
	{
		testState = false;
	}

	static bool testState2;

	if (GetAsyncKeyState(VK_F6))
	{
		if (!testState2)
		{
			Disconnect("hey");
			GameInit::KillNetwork(nullptr);

			testState2 = true;
		}
	}
	else
	{
		testState2 = false;
	}

	switch (m_connectionState)
	{
		case CS_INITRECEIVED:
			//m_connectionState = CS_DOWNLOADCOMPLETE;

			m_connectionState = CS_DOWNLOADING;

			if (!GameInit::GetGameLoaded())
			{
				GameInit::SetLoadScreens();

				TheDownloads.SetServer(m_currentServer);

				GameInit::LoadGameFirstLaunch([] ()
				{
					// download frame code
					Sleep(1);

					return TheDownloads.Process();
				});
			}
			else
			{
				GameInit::SetLoadScreens();

				TheDownloads.SetServer(m_currentServer);

				while (!TheDownloads.Process())
				{
					HANDLE hThread = GetCurrentThread();

					MsgWaitForMultipleObjects(1, &hThread, TRUE, 15, 0);
				}

				GameInit::ReloadGame();
			}

			break;

		case CS_DOWNLOADCOMPLETE:
			/*if (!GameInit::GetGameLoaded())
			{
				GameInit::LoadGameFirstLaunch(nullptr);
			}
			else
			{
				GameInit::ReloadGame();
			}*/

			m_connectionState = CS_CONNECTING;
			m_lastConnect = 0;

			break;

		case CS_CONNECTING:
			if ((GetTickCount() - m_lastConnect) > 5000)
			{
				NPID npID;
				NP_GetNPID(&npID);

				SendOutOfBand(m_currentServer, "connect token=%s&guid=%lld", m_token.c_str(), npID); // m_tempGuid

				m_lastConnect = GetTickCount();
			}

			break;

		case CS_ACTIVE:
			if ((GetTickCount() - m_lastReceivedAt) > 15000)
			{
				Disconnect("Connection timed out.");
				GlobalError("Server connection timed out after 15 seconds.");
			}

			break;
	}
}

void NetLibrary::ConnectToServer(const char* hostname, uint16_t port)
{
	if (m_connectionState != CS_IDLE)
	{
		Disconnect("Bye!");
	}

	m_connectionState = CS_INITING;
	m_currentServer = NetAddress(hostname, port);

	m_outReliableAcknowledged = 0;
	m_outSequence = 0;
	m_lastReceivedReliableCommand = 0;
	m_outReliableCommands.clear();

	wchar_t wideHostname[256];
	mbstowcs(wideHostname, hostname, _countof(wideHostname) - 1);

	wideHostname[255] = L'\0';

	std::map<std::string, std::string> postMap;
	postMap["method"] = "initConnect";
	postMap["name"] = GetPlayerName();

	NPID npID;
	NP_GetNPID(&npID);

	postMap["guid"] = va("%lld", npID);

	m_httpClient->DoPostRequest(wideHostname, port, L"/client", postMap, [&] (bool result, std::string connData)
	{
		if (!result)
		{
			// TODO: add UI output
			m_connectionState = CS_IDLE;

			return;
		}

		try
		{
			auto node = YAML::Load(connData);

			m_token = node["token"].as<std::string>();

			m_connectionState = CS_INITRECEIVED;
		}
		catch (YAML::Exception&)
		{
			m_connectionState = CS_IDLE;
		}
	});
}

static std::string g_disconnectReason;

void NetLibrary::Disconnect(const char* reason)
{
	g_disconnectReason = reason;

	GameInit::KillNetwork((const wchar_t*)1);
}

void NetLibrary::FinalizeDisconnect()
{
	if (m_connectionState == CS_CONNECTING || m_connectionState == CS_ACTIVE)
	{
		SendReliableCommand("msgIQuit", g_disconnectReason.c_str(), g_disconnectReason.length() + 1);

		m_lastSend = 0;
		ProcessSend();

		m_lastSend = 0;
		ProcessSend();

		TheDownloads.ReleaseLastServer();

		m_connectionState = CS_IDLE;
		m_currentServer = NetAddress();
	}
}

void NetLibrary::CreateResources()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		GlobalError("WSAStartup did not succeed.");
	}

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket == INVALID_SOCKET)
	{
		GlobalError("only one sock in pair");
	}

	// explicit bind
	sockaddr_in localAddr = { 0 };
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = ADDR_ANY;
	localAddr.sin_port = 0;

	bind(m_socket, (sockaddr*)&localAddr, sizeof(localAddr));

	// non-blocking
	u_long arg = true;
	ioctlsocket(m_socket, FIONBIO, &arg);

	m_httpClient = new HttpClient();
}

void NetLibrary::SendOutOfBand(NetAddress& address, const char* format, ...)
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
	return Auth_GetUsername();
}

void NetLibrary::SetPlayerName(const char* name)
{
	m_playerName = name;
}
	
void NetLibrary::SendData(NetAddress& address, const char* data, size_t length)
{
	sockaddr_storage addr;
	int addrLen;
	address.GetSockAddr(&addr, &addrLen);

	sendto(m_socket, data, length, 0, (sockaddr*)&addr, addrLen);
}

void NetLibrary::AddReliableHandlerImpl(const char* type, ReliableHandlerType function)
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

static NetLibrary netLibrary;
NetLibrary* g_netLibrary = &netLibrary;

void NetLibrary::AddReliableHandler(const char* type, ReliableHandlerType function)
{
	netLibrary.AddReliableHandlerImpl(type, function);
}

static InitFunction initFunction([] ()
{
	netLibrary.CreateResources();

	HooksDLLInterface::SetNetLibrary(&netLibrary);
	GameSpecDLLInterface::SetNetLibrary(&netLibrary);
});