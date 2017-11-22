/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "V8Debugger.h"

namespace fx
{
	class V8DebuggerImpl : public V8Debugger
	{
	public:
		virtual ~V8DebuggerImpl() override = default;
	};

	V8Debugger* CreateDebugger(v8::Isolate* isolate)
	{
		return new V8DebuggerImpl();
	}
}