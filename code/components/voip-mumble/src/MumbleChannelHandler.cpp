/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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