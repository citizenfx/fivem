/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "scrBind.h"
#include "Hooking.h"

#include <ICoreGameInit.h>

scrBindPool g_ScrBindPool{};

static InitFunction initFunction([]()
{
#ifndef IS_FXSERVER
	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_ScrBindPool.Reset();
	});
#endif
});
