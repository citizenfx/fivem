/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "StreamingTypes.h"
#include <Hooking.h>

CStreamingModuleManager* streamingModuleMgr;

static HookFunction hookFunc([]()
{
	streamingModuleMgr = *hook::get_pattern<CStreamingModuleManager*>("B8 FF FF 00 00 66 89 46 14", -20);
});
