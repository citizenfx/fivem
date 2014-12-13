/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

DEFINE_HANDLER(ChannelState)
{
	auto client = MumbleClient::GetCurrent();

	client->GetState().ProcessChannelState(data);
});

DEFINE_HANDLER(ChannelRemove)
{
	auto client = MumbleClient::GetCurrent();

	client->GetState().ProcessRemoveChannel(data.channel_id());
});