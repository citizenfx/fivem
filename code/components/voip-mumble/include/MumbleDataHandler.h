/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <MumbleTypes.h>
#include <MumbleMessageHandler.h>
#include <WS2tcpip.h>

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

	void HandleIncomingData(const uint8_t* data, size_t length);
};
