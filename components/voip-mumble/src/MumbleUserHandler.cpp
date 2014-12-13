/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

DEFINE_HANDLER(UserState)
{
	auto client = MumbleClient::GetCurrent();

	client->GetState().ProcessUserState(data);
});

DEFINE_HANDLER(UserRemove)
{
	auto client = MumbleClient::GetCurrent();

	client->GetState().ProcessRemoveUser(data.session());
});