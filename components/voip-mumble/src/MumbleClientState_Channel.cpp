/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "MumbleClientState.h"
#include "MumbleClientImpl.h"
#include "MumbleMessageHandler.h"

MumbleChannel::MumbleChannel(MumbleClient* client, MumbleProto::ChannelState& channelState)
	: m_client(client)
{
	m_hasDescription = false;
	m_temporary = false;

	UpdateChannel(channelState);
}

void MumbleChannel::UpdateChannel(MumbleProto::ChannelState& state)
{
	if (state.has_channel_id())
	{
		m_id = state.channel_id();
	}

	if (state.has_name())
	{
		m_channelName = ConvertFromUTF8(state.name());
	}

	if (state.has_description())
	{
		m_channelDescription = ConvertFromUTF8(state.description());
		m_hasDescription = true;
	}

	if (state.has_description_hash())
	{
		m_descriptionHash = state.description_hash();
		m_hasDescription = false;
	}

	if (state.has_temporary())
	{
		m_temporary = state.temporary();
	}
}

void MumbleClientState::ProcessChannelState(MumbleProto::ChannelState& channelState)
{
	if (channelState.has_channel_id())
	{
		// is this an update to a channel we know?
		auto id = channelState.channel_id();
		auto& channelIt = m_channels.find(id);

		if (channelIt == m_channels.end())
		{
			auto channel = MumbleChannel(m_client, channelState);

			m_channels.insert(std::make_pair(id, channel));

			auto name = channel.GetName();

			trace("New channel: %s\n", std::string(name.begin(), name.end()).c_str());
		}
		else
		{
			channelIt->second.UpdateChannel(channelState);
		}
	}
}

void MumbleClientState::ProcessRemoveChannel(uint32_t id)
{
	m_channels.erase(id);
}