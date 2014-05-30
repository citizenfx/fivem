// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: header file for NPFriends.cpp
//
// Initial author: NTAuthority
// Started: 2012-01-16
// ==========================================================

#pragma once

#include <map>

// friend state for Friends_Register
enum Presence
{
	PresenceOffline,
	PresenceOnline,
	PresenceAway,
	PresenceXA,
};

struct FriendState
{
	NPID npID;
	char name[256];
	Presence presence;
	std::map<std::string, std::string> richPresence;
	char richPresenceString[256];
};

// initialize the friends system
void Friends_Init();

// shut down the friends system
void Friends_Shutdown();

// connect to friends
void Friends_Connect();