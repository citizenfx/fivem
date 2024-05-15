#pragma once

#include <StdInc.h>
#include <ScriptEngine.h>

#include <ServerMumbleVoice.h>
#include "channel.h"
#include "client.h"

extern std::recursive_mutex g_mumbleClientMutex;

// Mumble user names are expected to follow a "[%d] %s" structure
static std::string CreateNamePrefix(int serverNetId)
{
	return fmt::sprintf("[%d]", serverNetId);
}

void fx::MumbleVoice::CreateChannel(const int id)
{
	const std::string channelName = fmt::sprintf("Game Channel %d", id);

	channel_t* parent = Chan_fromId(0);
	if (parent == NULL)
		return;

	channel_t* channel_itr = NULL;
	while (Chan_iterate_siblings(parent, &channel_itr) != NULL) {
		if (strcmp(channel_itr->name, channelName.c_str()) == 0) {
			break;
		}
	}

	if (channel_itr != NULL)
		return;

	channel_t* newChannel = Chan_createChannel(channelName.c_str(), "Permanent channel.");
	//newChannel->position = id;
	newChannel->password = NULL;
	newChannel->noenter = false;
	newChannel->silent = false;

	Chan_addChannel(parent, newChannel);
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("MUMBLE_CREATE_CHANNEL", [](fx::ScriptContext& context)
	{
		int channelId = context.GetArgument<int>(0);

		if (channelId != 0)
		{
			std::lock_guard _(g_mumbleClientMutex);
			fx::MumbleVoice::CreateChannel(channelId);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("MUMBLE_IS_PLAYER_MUTED", [](fx::ScriptContext& context)
	{
		int serverId = context.GetArgument<int>(0);
		auto namePrefix = CreateNamePrefix(serverId);

		std::lock_guard _(g_mumbleClientMutex);
		context.SetResult<bool>(Client_is_player_muted(namePrefix.c_str(), namePrefix.size()));
	});

	fx::ScriptEngine::RegisterNativeHandler("MUMBLE_SET_PLAYER_MUTED", [](fx::ScriptContext& context)
	{
		int serverId = context.GetArgument<int>(0);
		bool toggle = context.GetArgument<bool>(1);
		auto namePrefix = CreateNamePrefix(serverId);

		std::lock_guard _(g_mumbleClientMutex);
		Client_set_player_muted(namePrefix.c_str(), namePrefix.size(), toggle);
	});
});
