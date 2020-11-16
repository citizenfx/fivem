#pragma once

#include <NetBuffer.h>

#include <GameServerNet.h>

namespace fx
{
	class Client;
}

#ifdef COMPILING_CITIZEN_SERVER_IMPL
#define GSCOMMS_EXPORT DLL_EXPORT
#else
#define GSCOMMS_EXPORT DLL_IMPORT
#endif

GSCOMMS_EXPORT void gscomms_execute_callback_on_main_thread(const std::function<void()>& fn, bool force = false);
GSCOMMS_EXPORT void gscomms_execute_callback_on_net_thread(const std::function<void()>& fn);
GSCOMMS_EXPORT void gscomms_execute_callback_on_sync_thread(const std::function<void()>& fn);

GSCOMMS_EXPORT void gscomms_reset_peer(int peer);
GSCOMMS_EXPORT void gscomms_get_peer(int peer, fx::NetPeerStackBuffer& stackBuffer);

GSCOMMS_EXPORT void gscomms_send_packet(fx::Client* client, int peer, int channel, const net::Buffer& buffer, NetPacketType type);
