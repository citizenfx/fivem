/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <node/deps/v8/include/v8.h>

namespace fx
{
	class V8Debugger
	{
	public:
		virtual ~V8Debugger() {}
	};

	V8Debugger* CreateDebugger(v8::Isolate* isolate);
}
