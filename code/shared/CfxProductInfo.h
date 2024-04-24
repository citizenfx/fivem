/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if defined(GTA_NY)
#define PRODUCT_NAME L"LibertyM"
#define PRODUCT_NAME_NARROW "LibertyM"
#define CONTENT_NAME "libertym"
#elif defined(GTA_FIVE)
#define PRODUCT_NAME L"FiveM"
#define PRODUCT_NAME_NARROW "FiveM"
#define CONTENT_NAME "fivem"
//FiveM uses legacy name 'fivereborn' when checking updates.
#define UPDATE_NAME "fivereborn"
#elif defined(IS_RDR3)
#define PRODUCT_NAME L"RedM"
#define PRODUCT_NAME_NARROW "RedM"
#define CONTENT_NAME "redm"
#elif defined(IS_FXSERVER)
#define PRODUCT_NAME L"Server"
#define PRODUCT_NAME_NARROW "Server"
#define CONTENT_NAME "fxserver"
#elif defined(IS_LAUNCHER)
#define PRODUCT_NAME L"Cfx.re Launcher"
#define CONTENT_NAME "launcher"
#else
static_assert(false, "No recognized program definition found, are you missing a preprocessor definition?");
#endif
