#pragma once

#include <StdInc.h>
#include <ScriptEngine.h>

#include <ServerMumbleVoice.h>
#include "channel.h"

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
			fx::MumbleVoice::CreateChannel(channelId);
		}
	});
});
