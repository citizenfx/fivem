/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/Utils1.h>

namespace terminal
{
Utils1::Utils1(Client* client)
	: m_client(client)
{

}

void Utils1::SendRandomString(const std::string& string)
{
	// get a new message and send it
	fwRefContainer<L1MessageBuilder> builder = m_client->GetBuilder();
	auto message = builder->CreateMessage<msg::RPCStorageSendRandomStringMessage>();

	auto data = message.GetMessage();
	data->set_randomstring(string);

	message.SendOneShot();
}
}