/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "StreamingTypes.h"
#include <Hooking.h>

CStreamingTypeManager* streamingTypes;

static HookFunction hookFunc([]()
{
	streamingTypes = *hook::get_pattern<CStreamingTypeManager*>("B8 FF FF 00 00 66 89 46 14", -20);
});
