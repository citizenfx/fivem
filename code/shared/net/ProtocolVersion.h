#pragma once
#include "StdInc.h"
#if defined(GTA_FIVE) || defined(STATE_FIVE)
constexpr uint16_t kNetworkProtocolVersion = 12;
#else defined(IS_RDR3) || defined(STATE_RDR3)
constexpr uint16_t kNetworkProtocolVersion = 13;
#endif
