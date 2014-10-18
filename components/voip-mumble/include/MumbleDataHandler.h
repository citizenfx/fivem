#pragma once

#include <MumbleTypes.h>
#include <MumbleMessageHandler.h>
#include <WS2tcpip.h>

class MumbleDataHandler
{
private:
	int m_readBytes;
	uint32_t m_totalBytes;

	uint16_t m_messageType;

	uint8_t* m_messageBuffer;

private:
	void HandleCurrentPacket();

public:
	void Reset();

	void HandleIncomingData(const uint8_t* data, size_t length);
};