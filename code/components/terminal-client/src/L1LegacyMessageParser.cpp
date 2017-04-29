/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/L1LegacyMessageParser.h>

namespace terminal
{
void L1LegacyMessageParser::SetDispatcher(fwRefContainer<Dispatcher> dispatcher)
{
	m_dispatcher = dispatcher;
}

void L1LegacyMessageParser::HandleIncomingMessage(const std::vector<uint8_t>& data)
{
	uint32_t messageType = *(uint32_t*)&data[0];
	uint32_t messageID = *(uint32_t*)&data[sizeof(uint32_t)];

	// copy the original message data
	std::vector<uint8_t> messageData;
	messageData.resize(data.size() - 8);
	memcpy(&messageData[0], &data[8], messageData.size());

	m_dispatcher->DispatchMessage(messageType, messageID, messageData);
}
}