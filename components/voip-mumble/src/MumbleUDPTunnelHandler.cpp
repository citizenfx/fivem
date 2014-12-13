/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

static MumbleMessageHandler handler(MumbleMessageType::UDPTunnel, [] (const uint8_t* data, size_t size)
{
	printf("\n");
});