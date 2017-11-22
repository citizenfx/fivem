/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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