#pragma once

class NetAddress;

struct RoutingPacket
{
	uint16_t netID;
	std::string payload;
	uint32_t genTime;

	RoutingPacket();
};

class NetLibraryImplBase
{
public:
	virtual ~NetLibraryImplBase() = default;

	//
	// Creates resources.
	//
	virtual void CreateResources() = 0;

	//
	// Runs a single network frame.
	//
	virtual void RunFrame() = 0;

	virtual void Reset() = 0;

	virtual void SendData(const NetAddress& netAddress, const char* data, size_t length) = 0;

	virtual void SendReliableCommand(uint32_t type, const char* buffer, size_t length) = 0;

	virtual void SendConnect(const std::string& connectData) = 0;

	virtual bool HasTimedOut() = 0;

	virtual void Flush() = 0;
};

class INetLibraryInherit
{
public:
	virtual NetAddress GetCurrentServer() = 0;

	virtual void ProcessOOB(const NetAddress& from, const char* oob, size_t length) = 0;

	virtual int GetConnectionState() = 0;

	virtual void SetConnectionState(int state) = 0;

	virtual int GetServerProtocol() = 0;

	virtual INetMetricSink* GetMetricSink() = 0;

	virtual void HandleReliableCommand(uint32_t msgType, const char* buf, size_t length) = 0;

	virtual void HandleConnected(int serverNetID, int hostNetID, int hostBase) = 0;

	virtual void EnqueueRoutedPacket(uint16_t netID, const std::string& packet) = 0;

	virtual bool GetOutgoingPacket(RoutingPacket& packet) = 0;

	virtual void SendOutOfBand(const NetAddress& address, const char* format, ...) = 0;
};