/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CoreConsole.h"
#include "MumbleDataHandler.h"
#include "MumbleMessageHandler.h"

constexpr uint16_t kMaxTcpSize = 8186;

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
		handler->HandleMessage(m_messageBuffer.get(), m_totalBytes);
	}
}

IncomingDataFailReason MumbleDataHandler::HandleIncomingData(const uint8_t* data, size_t length)
{
	const uint8_t* origin = data;
	size_t read = length;

	// TODO: Decide if we want to support mumble servers that don't set no delay
	// currently this will eat packets if there are multiple sent in one payload
	while (read > 0)
	{
		// if this is a new 'packet'
		if (m_readBytes == 0)
		{
			if (read < 6)
			{
				return IncomingDataFailReason::InvalidHeader;
			}

			const MumblePacketHeader* header = (const MumblePacketHeader*)origin;

			m_totalBytes = header->GetPacketLength();
			m_messageType = header->GetPacketType();

			if (m_totalBytes > kMaxTcpSize)
			{
				console::PrintWarning("mumble", "Server sent an oversized packet (%u bytes), dropping connection and not attempting reconnection.\n", m_totalBytes);
				return IncomingDataFailReason::InvalidMessageSize;
			}

			m_messageBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[m_totalBytes]);

			origin = &origin[sizeof(MumblePacketHeader)];
			read -= sizeof(MumblePacketHeader);
		}

		int copyLength = std::min<size_t>(read, (m_totalBytes - m_readBytes));
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

	return IncomingDataFailReason::Success;
}
