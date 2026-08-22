#pragma once

#ifdef STATE_RDR3
#include <Client.h>

void CloneRecorder_OnClonePacket(const fx::ClientSharedPtr& client, const uint8_t* data, size_t len);
#endif
