// Copyright 2020 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "StdInc.h"
#include "ChannelListener.h"

// init static instance
ChannelListener ChannelListener::s_instance;

ChannelListener::ChannelListener()
	: m_listenerLock(), m_listeningUsers(), m_listenedChannels()
{
}

void ChannelListener::addListenerImpl(unsigned int userSession, int channelID)
{
	std::unique_lock lock(m_listenerLock);

	m_listeningUsers[userSession].insert(channelID);
	m_listenedChannels[channelID].insert(userSession);
}

void ChannelListener::removeListenerImpl(unsigned int userSession, int channelID)
{
	std::unique_lock lock(m_listenerLock);

	m_listeningUsers[userSession].erase(channelID);
	m_listenedChannels[channelID].erase(userSession);
}

bool ChannelListener::isListeningImpl(unsigned int userSession, int channelID) const
{
	std::shared_lock lock(m_listenerLock);

	auto it = m_listenedChannels.find(channelID);

	if (it != m_listenedChannels.end())
	{
		return it->second.find(userSession) != it->second.end();
	}

	return false;
}

bool ChannelListener::isListeningToAnyImpl(unsigned int userSession) const
{
	std::shared_lock lock(m_listenerLock);

	auto it = m_listeningUsers.find(userSession);

	if (it != m_listeningUsers.end())
	{
		return !it->second.empty();
	}

	return false;
}

bool ChannelListener::isListenedByAnyImpl(int channelID) const
{
	std::shared_lock lock(m_listenerLock);

	auto it = m_listenedChannels.find(channelID);

	if (it != m_listenedChannels.end())
	{
		return !it->second.empty();
	}

	return false;
}

const std::set<unsigned int> ChannelListener::getListenersForChannelImpl(int channelID) const
{
	std::shared_lock lock(m_listenerLock);

	auto it = m_listenedChannels.find(channelID);

	return (it != m_listenedChannels.end()) ? it->second : std::set<unsigned int>{};
}

const std::set<int> ChannelListener::getListenedChannelsForUserImpl(unsigned int userSession) const
{
	std::shared_lock lock(m_listenerLock);

	auto it = m_listeningUsers.find(userSession);

	return (it != m_listeningUsers.end()) ? it->second : std::set<int>{};
}

int ChannelListener::getListenerCountForChannelImpl(int channelID) const
{
	std::shared_lock lock(m_listenerLock);

	auto it = m_listenedChannels.find(channelID);

	if (it != m_listenedChannels.end())
	{
		return it->second.size();
	}

	return 0;
}

int ChannelListener::getListenedChannelCountForUserImpl(unsigned int userSession) const
{
	std::shared_lock lock(m_listenerLock);

	auto it = m_listeningUsers.find(userSession);

	if (it != m_listeningUsers.end())
	{
		return it->second.size();
	}

	return 0;
}

void ChannelListener::clearImpl()
{
	{
		std::unique_lock lock(m_listenerLock);
		m_listeningUsers.clear();
		m_listenedChannels.clear();
	}
}

ChannelListener& ChannelListener::get()
{
	return s_instance;
}

void ChannelListener::addListener(unsigned int userSession, int channelID)
{
	get().addListenerImpl(userSession, channelID);
}

void ChannelListener::addListener(const User* user, const Channel* channel)
{
	get().addListenerImpl(user->sessionId, channel->id);
}

void ChannelListener::removeListener(unsigned int userSession, int channelID)
{
	get().removeListenerImpl(userSession, channelID);
}

void ChannelListener::removeListener(const User* user, const Channel* channel)
{
	get().removeListenerImpl(user->sessionId, channel->id);
}

bool ChannelListener::isListening(unsigned int userSession, int channelID)
{
	return get().isListeningImpl(userSession, channelID);
}

bool ChannelListener::isListening(const User* user, const Channel* channel)
{
	return get().isListeningImpl(user->sessionId, channel->id);
}

bool ChannelListener::isListeningToAny(unsigned int userSession)
{
	return get().isListeningToAnyImpl(userSession);
}

bool ChannelListener::isListeningToAny(const User* user)
{
	return get().isListeningToAnyImpl(user->sessionId);
}

bool ChannelListener::isListenedByAny(int channelID)
{
	return get().isListenedByAnyImpl(channelID);
}

bool ChannelListener::isListenedByAny(const Channel* channel)
{
	return get().isListenedByAnyImpl(channel->id);
}

const std::set<unsigned int> ChannelListener::getListenersForChannel(int channelID)
{
	return get().getListenersForChannelImpl(channelID);
}

const std::set<unsigned int> ChannelListener::getListenersForChannel(const Channel* channel)
{
	return get().getListenersForChannelImpl(channel->id);
}

const std::set<int> ChannelListener::getListenedChannelsForUser(unsigned int userSession)
{
	return get().getListenedChannelsForUserImpl(userSession);
}

const std::set<int> ChannelListener::getListenedChannelsForUser(const User* user)
{
	return get().getListenedChannelsForUserImpl(user->sessionId);
}

int ChannelListener::getListenerCountForChannel(int channelID)
{
	return get().getListenerCountForChannelImpl(channelID);
}

int ChannelListener::getListenerCountForChannel(const Channel* channel)
{
	return get().getListenerCountForChannelImpl(channel->id);
}

int ChannelListener::getListenedChannelCountForUser(unsigned int userSession)
{
	return get().getListenedChannelCountForUserImpl(userSession);
}

int ChannelListener::getListenedChannelCountForUser(const User* user)
{
	return get().getListenedChannelCountForUserImpl(user->sessionId);
}

void ChannelListener::clear()
{
	get().clearImpl();
}
