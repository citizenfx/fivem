/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <v8.h>
//#include <include/v8-debug.h>

namespace fx
{
	class V8Debugger
	{
	public:
		virtual ~V8Debugger() {}
	};

	V8Debugger* CreateDebugger(v8::Isolate* isolate);
}
