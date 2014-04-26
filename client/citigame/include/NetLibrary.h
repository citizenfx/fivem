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

	void GetSockAddr(sockaddr_storage* addr, int* addrLen);
};

class NetBuffer
{
private:
	char* m_bytes;
	size_t m_curOff;
	size_t m_length;

	bool m_end;
	bool m_bytesManaged;

public:
	NetBuffer(const char* bytes, size_t length);
	NetBuffer(size_t length);

	virtual ~NetBuffer();

	bool End();

	void Read(void* buffer, size_t length);
	void Write(const void* buffer, size_t length);

	template<typename T>
	T Read()
	{
		T tempValue;
		Read(&tempValue, sizeof(T));
		
		return tempValue;
	}

	template<typename T>
	void Write(T value)
	{
		Write(&value, sizeof(T));
	}

	inline const char* GetBuffer() { return m_bytes; }
	inline size_t GetLength() { return m_length; }
};

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

	HttpClient* m_httpClient;

	SOCKET m_socket;

	uint32_t m_lastSend;

	uint32_t m_outSequence;

private:
	struct RoutingPacket
	{
		uint16_t netID;
		std::string payload;
	};

private:
	std::queue<RoutingPacket> m_incomingPackets;
	std::queue<RoutingPacket> m_outgoingPackets;

private:
	void ProcessOOB(NetAddress& from, char* oob, size_t length);

	void ProcessServerMessage(char* buffer, size_t length);

	void ProcessSend();

public:
	virtual uint16_t GetServerNetID();

	virtual uint16_t GetHostNetID();

	virtual uint32_t GetHostBase();

	virtual void SetBase(uint32_t base);

	virtual void RunFrame();

	virtual void ConnectToServer(const char* hostname, uint16_t port);

	virtual bool DequeueRoutedPacket(char* buffer, size_t* length, uint16_t* netID);

	virtual void RoutePacket(const char* buffer, size_t length, uint16_t netID);

	void EnqueueRoutedPacket(uint16_t netID, std::string packet);

	void SendOutOfBand(NetAddress& address, const char* format, ...);

	void SendData(NetAddress& address, const char* data, size_t length);

	void CreateResources();

	void ProcessPackets();
};