/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

static MumbleMessageHandler handler(MumbleMessageType::UDPTunnel, [] (const uint8_t* data, size_t size)
{
	printf("\n");
});