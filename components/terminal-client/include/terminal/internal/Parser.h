/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Dispatcher.h>

namespace terminal
{
class Parser : public fwRefCountable
{
public:
	virtual void HandleIncomingMessage(const std::vector<uint8_t>& data) = 0;

	virtual void SetDispatcher(fwRefContainer<Dispatcher> dispatcher) = 0;
};
}