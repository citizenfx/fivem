/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include "Hooking.h"

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

#ifndef _DEBUG
			__try
			{
#endif
				rageHandler(&rageContext);
#ifndef _DEBUG
			}
			__except (exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, EXCEPTION_EXECUTE_HANDLER)
			{
				throw std::exception(va("Error executing native 0x%016llx at address %p.", nativeIdentifier, exceptionAddress));
			}
#endif

			// append vector3 result components
			rageContext.SetVectorResults();

			// set return data
			context.SetResult(rageContext.GetResult<scrVector>());
		});
	}

	void ScriptEngine::RegisterNativeHandler(const std::string& nativeName, TNativeHandler function)
	{
		RegisterNativeHandler(HashString(nativeName.c_str()), function);
	}

	void ScriptEngine::RegisterNativeHandler(uint64_t nativeIdentifier, TNativeHandler function)
	{
#ifdef _M_AMD64
		TNativeHandler* handler = new TNativeHandler(function);

		struct StubGenerator : public jitasm::Frontend
		{
			typedef void(*TFunction)(void*, rage::scrNativeCallContext*);

		private:
			void* m_handlerData;

			TFunction m_function;

		public:
			StubGenerator(void* handlerData, TFunction function)
				: m_handlerData(handlerData), m_function(function)
			{

			}

			virtual void InternalMain() override
			{
				mov(rdx, rcx);
				mov(rcx, reinterpret_cast<uintptr_t>(m_handlerData));

				mov(rax, reinterpret_cast<uintptr_t>(m_function));

				jmp(rax);
			}
		}* callStub = new StubGenerator(handler, [] (void* handlerData, rage::scrNativeCallContext* context)
		{
			// get the handler
			TNativeHandler* handler = reinterpret_cast<TNativeHandler*>(handlerData);

			// turn into a native context
			ScriptContext cfxContext;
			
			for (int i = 0; i < context->GetArgumentCount(); i++)
			{
				cfxContext.Push(context->GetArgument<uintptr_t>(i));
			}

			// call the native
			(*handler)(cfxContext);

			// push the (single) result
			context->SetResult(0, cfxContext.GetResult<scrVector>());
		});

		rage::scrEngine::RegisterNativeHandler(nativeIdentifier, reinterpret_cast<rage::scrEngine::NativeHandler>(callStub->GetCode()));
#else
#error No stub generator for this architecture.
#endif
	}
}