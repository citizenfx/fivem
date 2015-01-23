/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Parser.h>

namespace terminal
{
class L1LegacyMessageParser : public Parser
{
private:
	fwRefContainer<Dispatcher> m_dispatcher;

public:
	virtual void SetDispatcher(fwRefContainer<Dispatcher> dispatcher) override;

	virtual void HandleIncomingMessage(const std::vector<uint8_t>& data) override;
};
}