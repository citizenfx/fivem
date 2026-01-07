/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <shared_mutex>
#include "MumbleTypes.h"

class MumbleClient;

class MumbleChannel
{
private:
	uint32_t m_id;

	MumbleClient* m_client;

	std::string m_channelName;

	bool m_hasDescription;

	std::string m_channelDescription;

	std::string m_descriptionHash;

	bool m_temporary;

public:
	MumbleChannel(MumbleClient* client, MumbleProto::ChannelState& channelState);

	inline std::string GetName() const { return m_channelName; }

	inline bool HasDescription() const { return m_hasDescription; }

	inline std::string GetDescription() const { return m_channelDescription; }

	inline bool IsTemporary() const { return m_temporary; }

	void UpdateChannel(MumbleProto::ChannelState& state);
};

class MumbleUser
{
private:
	MumbleClient* m_client;

	uint32_t m_session;

	uint32_t m_serverId;

	std::string m_name;

	uint32_t m_currentChannelId;

	bool m_muted;

	bool m_deafened;

	bool m_suppressed;

	bool m_selfMuted;

	bool m_selfDeafened;

public:
	MumbleUser(MumbleClient* client, MumbleProto::UserState& userState);

	inline explicit MumbleUser(uint32_t sessionId)
	{
		m_session = sessionId;
	}

	inline uint32_t GetSessionId() const { return m_session; }

	inline uint32_t GetServerId() const { return m_serverId; }

	inline std::string GetName() const { return m_name; }

	inline uint32_t GetChannelId() const { return m_currentChannelId; }

	void UpdateUser(MumbleProto::UserState& state);
};

class MumbleClientState
{
private:
	MumbleClient* m_client;

	uint32_t m_session;

	std::string m_username;

	std::map<uint32_t, MumbleChannel> m_channels;

	std::shared_mutex m_usersMutex;

	std::map<uint32_t, std::shared_ptr<MumbleUser>> m_users;

public:
	inline void Reset()
	{
		m_client = nullptr;
		m_session = 0;
		m_username = "";
		m_channels.clear();
		m_users.clear();
	}

	inline void SetClient(MumbleClient* client) { m_client = client; }

	inline void SetUsername(const std::string& value) { m_username = value; }

	inline std::string GetUsername() { return m_username; }

	inline void SetSession(uint32_t sessionId) { m_session = sessionId; }

	inline uint32_t GetSession() { return m_session; }

	inline std::map<uint32_t, MumbleChannel>& GetChannels() { return m_channels; }

	inline std::vector<std::string> GetUserNamesFromSessionIds(const std::vector<uint32_t>& sessionIds)
	{
		// preallocate because this could be a larger list on bigger servers
		// +1 because in GetTalkers we might add our local client to the talkers list
		std::vector<std::string> userNames(sessionIds.size() + 1);

		std::shared_lock<std::shared_mutex> lock(m_usersMutex);
		for (auto session : sessionIds)
		{
			auto it = m_users.find(session);
			if (it != m_users.end())
			{
				userNames.push_back(it->second->GetName());
			}
		}

		return userNames;
	}

	inline std::shared_ptr<MumbleUser> GetUser(uint32_t id)
	{
		std::shared_lock<std::shared_mutex> lock(m_usersMutex);

		auto it = m_users.find(id);

		return (it != m_users.end()) ? it->second : nullptr;
	}

	inline void ForAllUsers(const std::function<void(const std::shared_ptr<MumbleUser>&)>& cb)
	{
		std::shared_lock<std::shared_mutex> lock(m_usersMutex);

		for (auto& user : m_users)
		{
			cb(user.second);
		}
	}

	void ProcessChannelState(MumbleProto::ChannelState& channelState);

	void ProcessUserState(MumbleProto::UserState& userState);

	void ProcessRemoveChannel(uint32_t id);

	void ProcessRemoveUser(uint32_t id);
};
