#pragma once

#if __has_include("CnlEndpointOverride.h")
#include "CnlEndpointOverride.h"

// in case CnlEndpointOverride.h contains only a CNL_ENDPOINT, default CNL_HB_ENDPOINT to the same
#if !defined(CNL_HB_ENDPOINT) && defined(CNL_ENDPOINT)
#define CNL_HB_ENDPOINT CNL_ENDPOINT
#endif
#endif

#ifndef CNL_ENDPOINT
#define CNL_ENDPOINT "https://api.vmp.ir/"
#endif

#ifndef CNL_HB_ENDPOINT
#define CNL_HB_ENDPOINT "https://cnl-hb-live.fivem.net/"
#endif
