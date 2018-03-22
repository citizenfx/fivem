#pragma once

#include <enet/enet.h>
#include <NetBuffer.h>

void gscomms_execute_callback_on_main_thread(const std::function<void()>& fn);
void gscomms_execute_callback_on_net_thread(const std::function<void()>& fn);

void gscomms_reset_peer(int peer);
void gscomms_send_packet(int peer, int channel, const net::Buffer& buffer, ENetPacketFlag flags);

const ENetPeer* gscomms_get_peer(int peer);