/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
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