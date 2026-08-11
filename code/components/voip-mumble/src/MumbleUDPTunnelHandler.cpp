/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

static MumbleMessageHandler handler(MumbleMessageType::UDPTunnel, [] (const uint8_t* data, size_t size)
{
	auto client = MumbleClient::GetCurrent();

	client->HandleVoice(data, size);
});
