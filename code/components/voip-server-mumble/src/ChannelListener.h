// Copyright 2020 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_CHANNEL_LISTENER_H_
#define MUMBLE_CHANNEL_LISTENER_H_

#include <unordered_map>
#include <set>
#include <shared_mutex>

#include <atomic>

#include "client.h"
#include "channel.h"

using User = client_t;
using Channel = channel_t;

/// This class serves as a namespace for storing information about ChannelListeners. This is a feature
/// that allows a user to listen to a channel without being in it. Kinda similar to linked channels
/// except that this is something each user can do individually.
class ChannelListener
{
protected:
	/// A lock for guarding m_listeningUsers as well as m_listenedChannels
	mutable std::shared_mutex m_listenerLock;
	/// A map between a user's session and a list of IDs of all channels the user is listening to
	std::unordered_map<unsigned int, std::set<int>> m_listeningUsers;
	/// A map between a channel's ID and a list of all user-sessions of users listening to that channel
	std::unordered_map<int, std::set<unsigned int>> m_listenedChannels;

	/// The static ChannelListener instance returned by ChannelListener::get()
	static ChannelListener s_instance;

	/// Constructor
	ChannelListener();

	/// Adds a listener to the channel.
	///
	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	void addListenerImpl(unsigned int userSession, int channelID);

	/// Removes a listener from the channel.
	///
	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	void removeListenerImpl(unsigned int userSession, int channelID);

	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	/// @returns Whether the given user is listening to the given channel
	bool isListeningImpl(unsigned int userSession, int channelID) const;

	/// @param userSession The session ID of the user
	/// @returns Whether this user is listening to any channel via the ChannelListener feature
	bool isListeningToAnyImpl(unsigned int userSession) const;

	/// @param channelID The ID of the channel
	/// @returns Whether any user listens to this channel via the ChannelListener feature
	bool isListenedByAnyImpl(int channelID) const;

	/// @param channelID The ID of the channel
	/// @returns A set of user sessions of users listening to the given channel
	const std::set<unsigned int> getListenersForChannelImpl(int channelID) const;

	/// @param userSession The session ID of the user
	/// @returns A set of channel IDs of channels the given user is listening to
	const std::set<int> getListenedChannelsForUserImpl(unsigned int userSession) const;

	/// @param channelID The ID of the channel
	/// @returns The amount of users that are listening to the given channel
	int getListenerCountForChannelImpl(int channelID) const;

	/// @param userSession The session ID of the user
	/// @returns The amount of channels the given user is listening to
	int getListenedChannelCountForUserImpl(unsigned int userSession) const;

	/// Clears all ChannelListeners and volume adjustments
	void clearImpl();

public:
	/// @returns The static ChannelListener instance
	static ChannelListener& get();

	/// Adds a listener to the channel.
	///
	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	static void addListener(unsigned int userSession, int channelID);

	/// Adds a listener to the channel.
	///
	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	static void addListener(const User* user, const Channel* channel);

	/// Removes a listener from the channel.
	///
	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	static void removeListener(unsigned int userSession, int channelID);

	/// Removes a listener from the channel.
	///
	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	static void removeListener(const User* user, const Channel* channel);

	/// @param userSession The session ID of the user
	/// @param channelID The ID of the channel
	/// @returns Whether the given user is listening to the given channel
	static bool isListening(unsigned int userSession, int channelID);

	/// @param channel A pointer to the channel object
	/// @param user A pointer to the user object
	/// @returns Whether the given user is listening to the given channel
	static bool isListening(const User* user, const Channel* channel);

	/// @param userSession The session ID of the user
	/// @returns Whether this user is listening to any channel via the ChannelListener feature
	static bool isListeningToAny(unsigned int userSession);

	/// @param user A pointer to the user object
	/// @returns Whether this user is listening to any channel via the ChannelListener feature
	static bool isListeningToAny(const User* user);

	/// @param channelID The ID of the channel
	/// @returns Whether any user listens to this channel via the ChannelListener feature
	static bool isListenedByAny(int channelID);

	/// @param channel A pointer to the channel object
	/// @returns Whether any user listens to this channel via the ChannelListener feature
	static bool isListenedByAny(const Channel* channel);

	/// @param channelID The ID of the channel
	/// @returns A set of user sessions of users listening to the given channel
	static const std::set<unsigned int> getListenersForChannel(int channelID);

	/// @param channel A pointer to the channel object
	/// @returns A set of user sessions of users listening to the given channel
	static const std::set<unsigned int> getListenersForChannel(const Channel* channel);

	/// @param userSession The session ID of the user
	/// @returns A set of channel IDs of channels the given user is listening to
	static const std::set<int> getListenedChannelsForUser(unsigned int userSession);

	/// @param user A pointer to the user object
	/// @returns A set of channel IDs of channels the given user is listening to
	static const std::set<int> getListenedChannelsForUser(const User* user);

	/// @param channelID The ID of the channel
	/// @returns The amount of users that are listening to the given channel
	static int getListenerCountForChannel(int channelID);

	/// @param channel A pointer to the channel object
	/// @returns The amount of users that are listening to the given channel
	static int getListenerCountForChannel(const Channel* channel);

	/// @param userSession The session ID of the user
	/// @returns The amount of channels the given user is listening to
	static int getListenedChannelCountForUser(unsigned int userSession);

	/// @param user A pointer to the user object
	/// @returns The amount of channels the given user is listening to
	static int getListenedChannelCountForUser(const User* user);

	/// Clears all ChannelListeners and volume adjustments
	static void clear();
};

#endif // MUMBLE_CHANNEL_LISTENER_H_
