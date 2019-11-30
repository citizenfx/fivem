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
#include <ppltasks.h>
#include <WS2tcpip.h>
#include "HttpClient.h"
#include "CrossLibraryInterfaces.h"

#include "INetMetricSink.h"

#include "NetLibraryImplBase.h"

#include <NetAddress.h>

// hacky include path to not conflict with our own NetBuffer.h
#include <../../components/net-base/include/NetBuffer.h>

#include <enet/enet.h>

#include <concurrent_queue.h>

#define NETWORK_PROTOCOL 5

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

struct NetLibraryClientInfo
{
	int netId;
	int slotId;
	std::string name;
};

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

	uint16_t m_serverSlotID;

	uint16_t m_hostNetID;

	uint32_t m_serverBase;

	uint32_t m_hostBase;

	uint64_t m_serverTime;

	volatile ConnectionState m_connectionState;

	ConnectionState m_lastConnectionState;

	NetAddress m_currentServer;

	net::PeerAddress m_currentServerPeer;

	std::string m_currentServerUrl;

	std::string m_token;

	uint32_t m_lastConnect;

	uint32_t m_connectAttempts;

	uint32_t m_lastReconnect;

	uint32_t m_reconnectAttempts;

	HttpClient* m_httpClient;

	HttpRequestPtr m_handshakeRequest;

	uint32_t m_outSequence;

	uint32_t m_serverProtocol;

	std::string m_playerName;

	fwRefContainer<INetMetricSink> m_metricSink;

	std::string m_infoString;

	HANDLE m_receiveEvent;

	concurrency::concurrent_queue<std::function<void()>> m_mainFrameQueue;

	std::function<void(const std::string&, const std::string&)> m_cardResponseHandler;

private:
	typedef std::function<void(const char* buf, size_t len)> ReliableHandlerType;

	std::unordered_multimap<uint32_t, std::tuple<ReliableHandlerType, bool>> m_reliableHandlers;

private:
	std::mutex m_incomingPacketMutex;
	std::queue<RoutingPacket> m_incomingPackets;
	concurrency::concurrent_queue<RoutingPacket> m_outgoingPackets;

private:
	void HandleReliableCommand(uint32_t msgType, const char* buf, size_t length) override;

	NetLibrary();

public:
	inline bool AreDownloadsComplete()
	{
		return m_connectionState >= CS_DOWNLOADCOMPLETE;
	}

	virtual void ProcessOOB(const NetAddress& from, const char* oob, size_t length) override;

	virtual uint16_t GetServerNetID() override;

	virtual uint16_t GetHostNetID() override;

	virtual uint16_t GetServerSlotID() override;

	virtual uint32_t GetHostBase() override;

	virtual const char* GetPlayerName() override;

	virtual void SetPlayerName(const char* name) override;

	virtual void SetBase(uint32_t base) override;

	virtual void RunFrame() override;

	virtual concurrency::task<void> ConnectToServer(const std::string& rootUrl);

	virtual void Disconnect(const char* reason) override;

	virtual void FinalizeDisconnect() override;

	virtual bool DequeueRoutedPacket(char* buffer, size_t* length, uint16_t* netID) override;

	virtual void RoutePacket(const char* buffer, size_t length, uint16_t netID) override;

	virtual void SendReliableCommand(const char* type, const char* buffer, size_t length) override;

	void SendUnreliableCommand(const char* type, const char* buffer, size_t length);

	void RunMainFrame();

	void HandleConnected(int serverNetID, int hostNetID, int hostBase, int slotID, uint64_t serverTime) override;

	bool GetOutgoingPacket(RoutingPacket& packet) override;

	bool WaitForRoutedPacket(uint32_t timeout);

	void EnqueueRoutedPacket(uint16_t netID, const std::string& packet) override;

	void SendOutOfBand(const NetAddress& address, const char* format, ...) override;

	void SendData(const NetAddress& address, const char* data, size_t length);

	void CreateResources();

	void SetHost(uint16_t netID, uint32_t base);

	void DownloadsComplete();

	bool IsPendingInGameReconnect();

	// waits for connection during the pre-game loading sequence
	bool ProcessPreGameTick();

	void AddReliableHandler(const char* type, const ReliableHandlerType& function, bool runOnMainThreadOnly = false);

	void Death();

	void Resurrection();

	void CancelDeferredConnection();

	void SubmitCardResponse(const std::string& dataJson, const std::string& token);

	uint64_t GetGUID();

	void SendNetEvent(const std::string& eventName, const std::string& argsSerialized, int target);

	inline uint32_t GetServerBase() { return m_serverBase; }

	inline bool IsDisconnected() { return m_connectionState == CS_IDLE; }

	inline INetMetricSink* GetMetricSink() override
	{
		return m_metricSink.GetRef();
	}

	inline virtual NetAddress GetCurrentServer() override
	{
		return m_currentServer;
	}

	inline virtual net::PeerAddress GetCurrentPeer()
	{
		return m_currentServerPeer;
	}

	inline virtual const std::string& GetCurrentServerUrl() override
	{
		return m_currentServerUrl;
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

	inline uint64_t GetServerInitTime()
	{
		return m_serverTime;
	}

	void SetMetricSink(fwRefContainer<INetMetricSink>& sink);

	virtual void AddReceiveTick() override;

	virtual void AddSendTick() override;

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

	// a1: adaptive card JSON
	// a2: connection token
	fwEvent<const std::string&, const std::string&> OnConnectionCardPresent;

	// a1: status message
	// a2: current progress
	// a3: total progress
	fwEvent<const std::string&, int, int> OnConnectionProgress;

	// a1: detailed progress message
	fwEvent<const std::string&> OnConnectionSubProgress;

	// a1: message to spam the player with
	fwEvent<const std::string&> OnReconnectProgress;

	// event to intercept connection requests
	// return false to intercept connection (and call the callback to continue)
	// this won't like more than one interception attempt, however
	// a1: connection address
	// a2: continuation callback
	fwEvent<const std::string&, const std::function<void()>&> OnInterceptConnection;

	// same as the other routine, except it's for authentication
	fwEvent<const std::string&, const std::function<void(bool success, const std::map<std::string, std::string>& additionalPostData)>&> OnInterceptConnectionForAuth;

	// event to intercept server events for debugging
	// a1: event name
	// a2: event payload
	fwEvent<const std::string&, const std::string&> OnTriggerServerEvent;

	static
#ifndef COMPILING_NET
		__declspec(dllimport)
#endif
		fwEvent<const std::function<void(uint32_t, const char*, int)>&> OnBuildMessage;

	fwEvent<> OnConnectionTimedOut;

	// a1: new connection state
	// a2: previous connection state
	fwEvent<ConnectionState, ConnectionState> OnStateChanged;

	// for use from high-level code calling down to lower-level code
	fwEvent<const NetLibraryClientInfo&> OnClientInfoReceived;

	fwEvent<const NetLibraryClientInfo&> OnClientInfoDropped;
};

DECLARE_INSTANCE_TYPE(NetLibrary);

extern DLL_IMPORT fwEvent<const std::string&> OnRichPresenceSetTemplate;
extern DLL_IMPORT fwEvent<int, const std::string&> OnRichPresenceSetValue;
