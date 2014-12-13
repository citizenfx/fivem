/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleDataHandler.h"
#include "MumbleMessageHandler.h"

void MumbleDataHandler::Reset()
{
	m_messageBuffer = nullptr;
	m_messageType = 0;
	m_readBytes = 0;
	m_totalBytes = 0;
}

void MumbleDataHandler::HandleCurrentPacket()
{
	auto handler = MumbleMessageHandlerBase::GetHandlerFor((MumbleMessageType)m_messageType);

	if (handler)
	{
		handler->HandleMessage(m_messageBuffer, m_totalBytes);
	}
}

void MumbleDataHandler::HandleIncomingData(const uint8_t* data, size_t length)
{
	const uint8_t* origin = data;
	size_t read = length;

	while (read > 0)
	{
		// if this is a new 'packet'
		if (m_readBytes == 0)
		{
			const MumblePacketHeader* header = (const MumblePacketHeader*)origin;

			m_readBytes = 0;
			m_totalBytes = header->GetPacketLength();
			m_messageType = header->GetPacketType();

			m_messageBuffer = new uint8_t[m_totalBytes];

			origin = &origin[sizeof(MumblePacketHeader)];
			read -= sizeof(MumblePacketHeader);
		}

		int copyLength = min(read, (m_totalBytes - m_readBytes));
		memcpy(&m_messageBuffer[m_readBytes], origin, copyLength);

		m_readBytes += copyLength;
		read -= copyLength;
		origin += copyLength;

		if (m_readBytes >= m_totalBytes)
		{
			m_readBytes = 0;

			HandleCurrentPacket();
		}
	}
}