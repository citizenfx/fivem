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