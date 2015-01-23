/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Parser.h>
#include <terminal/internal/Socket.h>

#undef SendMessage

namespace terminal
{
class Connection : public fwRefCountable
{
public:
	virtual void SetParser(fwRefContainer<Parser> dispatcher) = 0;

	virtual void BindSocket(fwRefContainer<StreamSocket> socket) = 0;

	virtual void SendMessage(const std::vector<uint8_t>& message) = 0;

	virtual bool HasSocket() = 0;
};
}