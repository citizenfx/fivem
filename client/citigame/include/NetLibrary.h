#pragma once
#include <queue>
#include <WS2tcpip.h>
#include "CrossLibraryInterfaces.h"
#include "HttpClient.h"

enum NetAddressType
{
	NA_NONE,
	NA_INET4
};

class NetAddress
{
private:
	NetAddressType m_type;

	union
	{
		sockaddr_in m_in4;
	};

public:
	NetAddress() { m_type = NA_NONE; }
	NetAddress(const sockaddr_in* sockaddr) { m_in4 = *sockaddr; m_type = NA_INET4; }
	NetAddress(const char* address, uint16_t port);

	bool operator==(const NetAddress& right) const;
	bool operator!=(const NetAddress& right) const;

	std::string GetAddress();
	std::wstring GetWAddress();
	int GetPort();

	void GetSockAddr(sockaddr_storage* addr, int* addrLen);
};

#include "NetBuffer.h"

class NetLibrary;

#define FRAGMENT_SIZE 1300

class NetChannel
{
private:
	int m_fragmentSequence;
	int m_fragmentLength;
	std::string m_fragmentBuffer;

	int m_inSequence;
	int m_outSequence;

	NetAddress m_targetAddress;
	NetLibrary* m_netLibrary;

private:
	void SendFragmented(NetBuffer& buffer);

public:
	NetChannel();

	void Reset(NetAddress& target, NetLibrary* netLibrary);

	void Send(NetBuffer& buffer);

	bool Process(const char* message, size_t size, NetBuffer** buffer);
};

#define MAX_RELIABLE_COMMANDS 64

class NetLibrary : public INetLibrary
{
public:
	enum ConnectionState
	{
		CS_IDLE,
		CS_INITING,
		CS_INITRECEIVED,
		CS_DOWNLOADING,
		CS_DOWNLOADCOMPLETE,
		CS_CONNECTING,
		CS_CONNECTED,
		CS_ACTIVE
	};

	struct OutReliableCommand
	{
		uint32_t id;
		uint32_t type;
		std::string command;
	};

private:
	uint16_t m_serverNetID;

	uint16_t m_hostNetID;

	uint32_t m_serverBase;

	uint32_t m_hostBase;

	ConnectionState m_connectionState;

	NetAddress m_currentServer;

	std::string m_token;

	uint32_t m_tempGuid;

	uint32_t m_lastConnect;

	uint32_t m_connectAttempts;

	HttpClient* m_httpClient;

	SOCKET m_socket;

	uint32_t m_lastSend;

	uint32_t m_outSequence;

	NetChannel m_netChannel;

	uint32_t m_lastReceivedReliableCommand;

	uint32_t m_outReliableAcknowledged;

	uint32_t m_outReliableSequence;

	std::list<OutReliableCommand> m_outReliableCommands;

	uint32_t m_lastReceivedAt;

	std::string m_playerName;

private:
	typedef std::function<void(const char* buf, size_t len)> ReliableHandlerType;

	std::unordered_multimap<uint32_t, ReliableHandlerType> m_reliableHandlers;

private:
	struct RoutingPacket
	{
		uint16_t netID;
		std::string payload;
		uint32_t genTime;

		RoutingPacket();
	};

private:
	std::queue<RoutingPacket> m_incomingPackets;
	std::queue<RoutingPacket> m_outgoingPackets;

private:
	void ProcessOOB(NetAddress& from, char* oob, size_t length);

	void ProcessServerMessage(NetBuffer& msg);

	void ProcessSend();

	void HandleReliableCommand(uint32_t msgType, const char* buf, size_t length);

	void AddReliableHandlerImpl(const char* type, ReliableHandlerType function);

public:
	virtual uint16_t GetServerNetID();

	virtual uint16_t GetHostNetID();

	virtual uint32_t GetHostBase();

	virtual const char* GetPlayerName();
	
	virtual void SetPlayerName(const char* name);

	virtual void SetBase(uint32_t base);

	virtual void RunFrame();

	virtual void ConnectToServer(const char* hostname, uint16_t port);

	virtual void Disconnect(const char* reason);

	virtual void FinalizeDisconnect();

	virtual bool DequeueRoutedPacket(char* buffer, size_t* length, uint16_t* netID);

	virtual void RoutePacket(const char* buffer, size_t length, uint16_t netID);

	virtual void SendReliableCommand(const char* type, const char* buffer, size_t length);

	virtual void PreProcessNativeNet();

	virtual void PostProcessNativeNet();

	void EnqueueRoutedPacket(uint16_t netID, std::string packet);

	void SendOutOfBand(NetAddress& address, const char* format, ...);

	void SendData(NetAddress& address, const char* data, size_t length);

	void CreateResources();

	void SetHost(uint16_t netID, uint32_t base);

	void ProcessPackets();

	void DownloadsComplete();

	// waits for connection during the pre-game loading sequence
	bool ProcessPreGameTick();
	
	static void AddReliableHandler(const char* type, ReliableHandlerType function);
};

extern NetLibrary* g_netLibrary;