/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <MumbleTypes.h>
#include <MumbleMessageHandler.h>
#include <WS2tcpip.h>

enum IncomingDataFailReason : uint8_t {
	Success = 0,
	InvalidHeader = 1,
	
	InvalidMessageSize = 5
};

class MumbleDataHandler
{
private:
	size_t m_readBytes = 0;
	size_t m_totalBytes = 0;

	uint16_t m_messageType = 0;

	std::unique_ptr<uint8_t[]> m_messageBuffer;

private:
	void HandleCurrentPacket();

public:
	void Reset();

	IncomingDataFailReason HandleIncomingData(const uint8_t* data, size_t length);
};
