/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
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