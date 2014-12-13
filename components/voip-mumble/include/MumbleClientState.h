/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "MumbleTypes.h"

class MumbleClient;

class MumbleChannel
{
private:
	uint32_t m_id;

	MumbleClient* m_client;

	std::wstring m_channelName;

	bool m_hasDescription;

	std::wstring m_channelDescription;

	std::string m_descriptionHash;

	bool m_temporary;

public:
	MumbleChannel(MumbleClient* client, MumbleProto::ChannelState& channelState);

	inline std::wstring GetName() { return m_channelName; }

	inline bool HasDescription() { return m_hasDescription; }

	std::wstring GetDescription();

	inline bool IsTemporary() { return m_temporary; }

	void UpdateChannel(MumbleProto::ChannelState& state);
};

class MumbleUser
{
private:
	MumbleClient* m_client;

	uint32_t m_session;

	std::wstring m_name;

	uint32_t m_currentChannelId;

	bool m_muted;

	bool m_deafened;

	bool m_suppressed;

	bool m_selfMuted;

	bool m_selfDeafened;

public:
	MumbleUser(MumbleClient* client, MumbleProto::UserState& userState);

	inline std::wstring GetName() { return m_name; }

	void UpdateUser(MumbleProto::UserState& state);
};

class MumbleClientState
{
private:
	MumbleClient* m_client;

	std::wstring m_username;

	std::map<uint32_t, MumbleChannel> m_channels;

	std::map<uint32_t, MumbleUser> m_users;

public:
	inline void SetClient(MumbleClient* client) { m_client = client; }

	inline void SetUsername(std::wstring& value) { m_username = value; }

	inline std::wstring GetUsername() { return m_username; }

	void ProcessChannelState(MumbleProto::ChannelState& channelState);

	void ProcessUserState(MumbleProto::UserState& userState);

	void ProcessRemoveChannel(uint32_t id);

	void ProcessRemoveUser(uint32_t id);
};