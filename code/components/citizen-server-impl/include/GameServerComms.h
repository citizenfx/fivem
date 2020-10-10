#pragma once

#include <NetBuffer.h>

#include <GameServerNet.h>

namespace fx
{
	class Client;
}

void gscomms_execute_callback_on_main_thread(const std::function<void()>& fn, bool force = false);
void gscomms_execute_callback_on_net_thread(const std::function<void()>& fn);
void gscomms_execute_callback_on_sync_thread(const std::function<void()>& fn);

void gscomms_reset_peer(int peer);
void gscomms_get_peer(int peer, fx::NetPeerStackBuffer& stackBuffer);

void gscomms_send_packet(fx::Client* client, int peer, int channel, const net::Buffer& buffer, NetPacketType type);
