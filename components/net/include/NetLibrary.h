/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include <queue>
#include <bitset>
#include <functional>
#include <thread>
#include <WS2tcpip.h>
#include "HttpClient.h"
#include "CrossLibraryInterfaces.h"

#include "INetMetricSink.h"

#include "NetLibraryImplBase.h"

#include <enet/enet.h>

#include <concurrent_queue.h>

#define NETWORK_PROTOCOL 4

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
	NetAddress(const ENetAddress* addr);
	NetAddress(const char* address, uint16_t port);

	bool operator==(const NetAddress& right) const;
	bool operator!=(const NetAddress& right) const;

	fwString GetAddress();
	fwWString GetWAddress();
	int GetPort();

	void GetSockAddr(sockaddr_storage* addr, int* addrLen) const;

	ENetAddress GetENetAddress() const;
};

#include "NetBuffer.h"

class NetLibrary;

#define FRAGMENT_SIZE (uint32_t)1300

class NetChannel
{
private:
	int m_fragmentSequence;
	int m_fragmentLength;
	char* m_fragmentBuffer;
	std::bitset<65536 / FRAGMENT_SIZE> m_fragmentValidSet;
	int m_fragmentLastBit;

	uint32_t m_inSequence;
	uint32_t m_outSequence;

	NetAddress m_targetAddress;
	NetLibraryImplBase* m_netLibrary;

private:
	void SendFragmented(NetBuffer& buffer);

public:
	NetChannel();

	void Reset(NetAddress& target, NetLibraryImplBase* netLibrary);

	void Send(NetBuffer& buffer);

	bool Process(const char* message, size_t size, NetBuffer** buffer);
};

#define MAX_RELIABLE_COMMANDS 64

class
#ifdef COMPILING_NET
	__declspec(dllexport)
#endif
	NetLibrary : public INetLibrary, public INetLibraryInherit
{
public:
	enum ConnectionState
	{
		CS_IDLE,
		CS_INITING,
		CS_INITRECEIVED,
		CS_DOWNLOADING,
		CS_DOWNLOADCOMPLETE,
		CS_FETCHING,
		CS_CONNECTING,
		CS_CONNECTED,
		CS_ACTIVE
	};

private:
	std::unique_ptr<NetLibraryImplBase> m_impl;

	uint16_t m_serverNetID;

	uint16_t m_hostNetID;

	uint32_t m_serverBase;

	uint32_t m_hostBase;

	ConnectionState m_connectionState;

	ConnectionState m_lastConnectionState;

	NetAddress m_currentServer;

	std::string m_token;

	uint32_t m_lastConnect;

	uint32_t m_connectAttempts;

	HttpClient* m_httpClient;

	uint32_t m_outSequence;

	uint32_t m_serverProtocol;

	std::string m_playerName;

	fwRefContainer<INetMetricSink> m_metricSink;

	std::string m_infoString;

	HANDLE m_receiveEvent;

private:
	typedef std::function<void(const char* buf, size_t len)> ReliableHandlerType;

	std::unordered_multimap<uint32_t, ReliableHandlerType> m_reliableHandlers;

private:
	std::mutex m_incomingPacketMutex;
	std::queue<RoutingPacket> m_incomingPackets;
	concurrency::concurrent_queue<RoutingPacket> m_outgoingPackets;

private:
	void ProcessServerMessage(NetBuffer& msg);

	void ProcessSend();

	void HandleReliableCommand(uint32_t msgType, const char* buf, size_t length);

	NetLibrary();

public:
	inline bool AreDownloadsComplete()
	{
		return m_connectionState >= CS_DOWNLOADCOMPLETE;
	}

	virtual void ProcessOOB(const NetAddress& from, const char* oob, size_t length);

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

	void HandleConnected(int serverNetID, int hostNetID, int hostBase) override;

	bool GetOutgoingPacket(RoutingPacket& packet) override;

	bool WaitForRoutedPacket(uint32_t timeout);

	void EnqueueRoutedPacket(uint16_t netID, const std::string& packet) override;

	void SendOutOfBand(const NetAddress& address, const char* format, ...);

	void SendData(const NetAddress& address, const char* data, size_t length);

	void CreateResources();

	void SetHost(uint16_t netID, uint32_t base);

	void DownloadsComplete();

	// waits for connection during the pre-game loading sequence
	bool ProcessPreGameTick();

	void AddReliableHandler(const char* type, ReliableHandlerType function);

	void Death();

	void Resurrection();

	void SendNetEvent(const std::string& eventName, const std::string& argsSerialized, int target);

	inline uint32_t GetServerBase() { return m_serverBase; }

	inline bool IsDisconnected() { return m_connectionState == CS_IDLE; }

	inline INetMetricSink* GetMetricSink()
	{
		return m_metricSink.GetRef();
	}

	inline virtual NetAddress GetCurrentServer() override
	{
		return m_currentServer;
	}

	inline int GetServerProtocol() override
	{
		return m_serverProtocol;
	}

	inline int GetConnectionState() override
	{
		return m_connectionState;
	}

	inline void SetConnectionState(int state) override
	{
		m_connectionState = (ConnectionState)state;
	}

	void SetMetricSink(fwRefContainer<INetMetricSink>& sink);

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

	fwEvent<NetAddress> OnConnectOKReceived;

	fwEvent<NetAddress> OnFinalizeDisconnect;

	fwEvent<const char*> OnConnectionError;

	// a1: status message
	// a2: current progress
	// a3: total progress
	fwEvent<const std::string&, int, int> OnConnectionProgress;

	// a1: detailed progress message
	fwEvent<const std::string&> OnConnectionSubProgress;

	static
#ifndef COMPILING_NET
		__declspec(dllimport)
#endif
		fwEvent<const std::function<void(uint32_t, const char*, int)>&> OnBuildMessage;

	fwEvent<> OnConnectionTimedOut;

	// a1: new connection state
	// a2: previous connection state
	fwEvent<ConnectionState, ConnectionState> OnStateChanged;
};
//extern NetLibrary* g_netLibrary;