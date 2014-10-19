#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

static MumbleMessageHandler handler(MumbleMessageType::UDPTunnel, [] (const uint8_t* data, size_t size)
{
	printf("\n");
});