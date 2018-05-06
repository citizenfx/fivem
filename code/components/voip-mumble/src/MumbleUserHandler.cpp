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

DEFINE_HANDLER(ServerSync)
{
	auto client = MumbleClient::GetCurrent();

	client->GetState().SetSession(data.session());

	MumbleProto::UserState state;
	state.set_session(data.session());
	state.set_plugin_context(std::string("Manual placement") + '\0');

	client->Send(MumbleMessageType::UserState, state);

	client->MarkConnected();
});

DEFINE_HANDLER(Ping)
{
	MumbleClient::GetCurrent()->HandlePing(data);
});
