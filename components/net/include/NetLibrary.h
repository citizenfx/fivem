#pragma once
#include <queue>
#include <bitset>
#include <functional>
#include <WS2tcpip.h>
#include "HttpClient.h"
#include "CrossLibraryInterfaces.h"

#define NETWORK_PROTOCOL 2

enum NetAddressType
{
	NA_NONE,
	NA_INET4,
	NA_INET6
};

class
#ifdef COMPILING_NET
	__declspec(dllexport)
#endif
	NetAddress
{
private:
	NetAddressType m_type;

	union
	{
		sockaddr_in m_in4;
		sockaddr_in6 m_in6;
	};

public:
	NetAddress() { m_type = NA_NONE; }
	NetAddress(const sockaddr* addr);
	NetAddress(const sockaddr_in* addr) : NetAddress((const sockaddr*)addr) {}
	NetAddress(const sockaddr_in6* addr) : NetAddress((const sockaddr*)addr) {}
	NetAddress(const char* address, uint16_t port);

	bool operator==(const NetAddress& right) const;
	bool operator!=(const NetAddress& right) const;

	fwString GetAddress();
	fwWString GetWAddress();
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
	char* m_fragmentBuffer;
	std::bitset<65536 / FRAGMENT_SIZE> m_fragmentValidSet;
	int m_fragmentLastBit;

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

class
#ifdef COMPILING_NET
	__declspec(dllexport)
#endif
	NetLibrary : public INetLibrary
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

	SOCKET m_socket6;

	uint32_t m_lastSend;

	uint32_t m_outSequence;

	NetChannel m_netChannel;

	uint32_t m_serverProtocol;

	uint32_t m_lastReceivedReliableCommand;

	uint32_t m_outReliableAcknowledged;

	uint32_t m_outReliableSequence;

	std::list<OutReliableCommand> m_outReliableCommands;

	uint32_t m_lastReceivedAt;

	uint32_t m_lastFrameNumber;

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

	void ProcessPacketsInternal(NetAddressType addrType);

	NetLibrary();

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

	void AddReliableHandler(const char* type, ReliableHandlerType function);

	void Death();

	void Resurrection();

	void SendNetEvent(fwString eventName, fwString argsSerialized, int target);

	inline uint32_t GetServerBase() { return m_serverBase; }

	inline bool IsDisconnected() { return m_connectionState == CS_IDLE; }

public:
	static NetLibrary* Create();

public:
	static
#ifndef COMPILING_NET
		__declspec(dllimport)
#endif
		fwEvent<NetLibrary*> OnNetLibraryCreate;

	fwEvent<const char*> OnAttemptDisconnect;

	fwEvent<NetAddress> OnInitReceived;

	fwEvent<NetAddress> OnFinalizeDisconnect;

	fwEvent<const char*> OnConnectionError;

	fwEvent<NetBuffer&> OnBuildMessage;

	fwEvent<> OnConnectionTimedOut;
};
//extern NetLibrary* g_netLibrary;