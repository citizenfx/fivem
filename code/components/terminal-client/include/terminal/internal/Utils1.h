/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Client.h>

namespace terminal
{
class Utils1 : public IUtils1, public fwRefCountable
{
private:
	Client* m_client;

public:
	Utils1(Client* client);

	virtual void SendRandomString(const std::string& string) override;
};
}