/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <scrEngine.h>

namespace fx
{
	boost::optional<TNativeHandler> ScriptEngine::GetNativeHandler(uint64_t nativeIdentifier)
	{
		auto rageHandler = rage::scrEngine::GetNativeHandler(nativeIdentifier);

		if (rageHandler == nullptr)
		{
			return boost::optional<TNativeHandler>();
		}

		return boost::optional<TNativeHandler>([=] (ScriptContext& context)
		{
			// push arguments from the original context
			NativeContext rageContext;
			
			for (int i = 0; i < context.GetArgumentCount(); i++)
			{
				rageContext.Push(context.GetArgument<uintptr_t>(i));
			}

			// call the original function
			static void* exceptionAddress;

			__try
			{
				rageHandler(&rageContext);
			}
			__except (exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, EXCEPTION_EXECUTE_HANDLER)
			{
				throw std::exception(va("Error executing native 0x%016x at address %p.", nativeIdentifier, exceptionAddress));
			}

			// set return data
			context.SetResult(rageContext.GetResult<scrVector>());
		});
	}
}