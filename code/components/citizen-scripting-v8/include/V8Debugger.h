/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <include/v8.h>
#include <include/v8-debug.h>

namespace fx
{
	class V8Debugger
	{
	public:
		virtual ~V8Debugger() {}
	};

	V8Debugger* CreateDebugger(v8::Isolate* isolate);
}