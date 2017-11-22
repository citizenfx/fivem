/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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