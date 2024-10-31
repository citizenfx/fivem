#pragma once

#include "StdInc.h"

#include "EventCore.h"

namespace cfx::glue
{
/**
 * IPC related to handling of protocol links.
 *
 * When user clicks a protocol link, e.g. fivem://connect/...
 * Windows will launch our executable and LinkProtocolIPC
 * will handle sending of a message to the master process.
 *
 * Master process listens to such events and triggers event listeners
 * for matching message types.
 * 
 * Please see LinkProtocolIPC.cpp for implementation details.
 */
class LinkProtocolIPC
{
public:
	// Sends "auth payload" IPC message to master process.
	static void SendAuthPayload(const std::string& message);
	// Event for handling "auth payload" messages in master process.
	static inline fwEvent<const std::string_view&> OnAuthPayload;

	// Sends "connect to" IPC message to master process.
	static void SendConnectTo(const std::string& message);
	// Event for handling "connect to" messages in master process.
	static inline fwEvent<const std::string_view&> OnConnectTo;

	// Initialize IPC in the master process.
	static void Initialize();

	// Process IPC messages in the master process.
	//
	// It will trigger matching events when message arrives.
	static void ProcessMessages();
};
};
