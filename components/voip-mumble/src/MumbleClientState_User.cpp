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

MumbleUser::MumbleUser(MumbleClient* client, MumbleProto::UserState& userState)
	: m_client(client)
{
	m_muted = false;
	m_deafened = false;
	m_session = 0;
	m_selfDeafened = false;
	m_selfMuted = false;
	m_suppressed = false;

	UpdateUser(userState);
}

void MumbleUser::UpdateUser(MumbleProto::UserState& state)
{
	if (state.has_session())
	{
		m_session = state.session();
	}

	if (state.has_name())
	{
		m_name = ConvertFromUTF8(state.name());
	}

	if (state.has_mute())
	{
		m_muted = state.mute();
	}

	if (state.has_deaf())
	{
		m_deafened = state.deaf();
	}

	if (state.has_suppress())
	{
		m_suppressed = state.suppress();
	}

	if (state.has_self_mute())
	{
		m_selfMuted = state.self_mute();
	}

	if (state.has_self_deaf())
	{
		m_selfDeafened = state.self_deaf();
	}

	if (state.has_channel_id())
	{
		m_currentChannelId = state.channel_id();
	}
}

void MumbleClientState::ProcessUserState(MumbleProto::UserState& userState)
{
	if (userState.has_session())
	{
		// is this an update to a channel we know?
		auto id = userState.session();
		auto& userIt = m_users.find(id);

		if (userIt == m_users.end())
		{
			auto user = MumbleUser(m_client, userState);

			m_users.insert(std::make_pair(id, user));

			auto name = user.GetName();

			trace("New user: %s\n", std::string(name.begin(), name.end()).c_str());
		}
		else
		{
			userIt->second.UpdateUser(userState);
		}
	}
}

void MumbleClientState::ProcessRemoveUser(uint32_t id)
{
	m_users.erase(id);
}