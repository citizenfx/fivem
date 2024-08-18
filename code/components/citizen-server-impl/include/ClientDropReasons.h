#pragma once

namespace fx
{
enum class ClientDropReason: uint32_t
{
	// resource dropped the client
	RESOURCE = 1,
	// client initiated a disconnect
	CLIENT,
	// server initiated a disconnect
	SERVER,
	// client with same guid connected and kicks old client
	CLIENT_REPLACED,
	// server -> client connection timed out
	CLIENT_CONNECTION_TIMED_OUT,
	// server -> client connection timed out with pending commands
	CLIENT_CONNECTION_TIMED_OUT_WITH_PENDING_COMMANDS,
	// server shutdown triggered the client drop
	SERVER_SHUTDOWN,
	// state bag rate limit exceeded
	STATE_BAG_RATE_LIMIT,
	// net event rate limit exceeded
	NET_EVENT_RATE_LIMIT,
	// latent net event rate limit exceeded
	LATENT_NET_EVENT_RATE_LIMIT,
	// command rate limit exceeded
	COMMAND_RATE_LIMIT,
	// too many missed frames in OneSync
	ONE_SYNC_TOO_MANY_MISSED_FRAMES
};

constexpr const char* clientDropResourceName = "__cfx_internal:client";
constexpr const char* serverDropResourceName = "__cfx_internal:server";
constexpr const char* serverOneSyncDropResourceName = "__cfx_internal:server_onesync";
}
