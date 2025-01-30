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

#include <CoreConsole.h>

MumbleUser::MumbleUser(MumbleClient* client, MumbleProto::UserState& userState)
	: m_client(client)
{
	m_muted = false;
	m_deafened = false;
	m_session = 0;
	m_selfDeafened = false;
	m_selfMuted = false;
	m_suppressed = false;
	m_currentChannelId = 0;

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
		std::string name = state.name();
		m_name = ConvertFromUTF8(name);
		if (name.length() >= 2)
		{
			m_serverId = atoi(name.substr(1, name.length() - 1).c_str());
		} 
		else
		{
			m_serverId = 0;
		}
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

	console::DPrintf("mumble", "%s joined channel %d\n", ConvertToUTF8(m_name), m_currentChannelId);
}

void MumbleClientState::ProcessUserState(MumbleProto::UserState& userState)
{

	std::shared_ptr<MumbleUser> createdUser;

	if (userState.has_session())
	{
		// is this an update to a channel we know?
		std::unique_lock<std::shared_mutex> lock(m_usersMutex);

		auto id = userState.session();
		auto& userIt = m_users.find(id);

		if (userIt == m_users.end())
		{
			auto user = std::make_shared<MumbleUser>(m_client, userState);
			m_users.emplace(id, user);

			createdUser = user;

			console::DPrintf("Mumble", "New user: %s\n", ToNarrow(user->GetName()));
		}
		else
		{
			userIt->second->UpdateUser(userState);
		}
	}

	if (createdUser)
	{
		m_client->GetOutput().HandleClientConnect(*createdUser);
	}
}

void MumbleClientState::ProcessRemoveUser(uint32_t id)
{
	std::shared_ptr<MumbleUser> removedUser;

	{
		std::unique_lock<std::shared_mutex> lock(m_usersMutex);

		auto it = m_users.find(id);

		if (it != m_users.end())
		{
			removedUser = it->second;
		}

		m_users.erase(id);
	}

	if (removedUser)
	{
		m_client->GetOutput().HandleClientDisconnect(*removedUser);
	}
}
