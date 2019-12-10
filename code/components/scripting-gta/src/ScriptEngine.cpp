/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include "Hooking.h"

#include <optick.h>

#include <scrEngine.h>

#include <NativeHandlerLogging.h>

template<typename THandler>
static inline void CallHandler(const THandler& rageHandler, uint64_t nativeIdentifier, rage::scrNativeCallContext& rageContext)
{
	// call the original function
	static void* exceptionAddress;

#ifdef ENABLE_NATIVE_HANDLER_LOGGING
	NativeHandlerLogging::CountNative(nativeIdentifier);
#endif

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
}

extern "C" bool DLL_EXPORT WrapNativeInvoke(rage::scrEngine::NativeHandler handler, uint64_t nativeIdentifier, rage::scrNativeCallContext* context)
{
	static void* exceptionAddress;

	__try
	{
		handler(context);
		return true;
	}
	__except (exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, EXCEPTION_EXECUTE_HANDLER)
	{
		trace("Error executing native %016llx at address %p.\n", nativeIdentifier, exceptionAddress);
		return false;
	}
}

static std::unordered_map<uint64_t, fx::TNativeHandler> g_registeredHandlers;

namespace fx
{
	boost::optional<TNativeHandler> ScriptEngine::GetNativeHandler(uint64_t nativeIdentifier)
	{
		auto it = g_registeredHandlers.find(nativeIdentifier);

		if (it != g_registeredHandlers.end())
		{
			return it->second;
		}

		auto rageHandler = rage::scrEngine::GetNativeHandler(nativeIdentifier);

		if (rageHandler == nullptr)
		{
			return boost::optional<TNativeHandler>();
		}

		return boost::optional<TNativeHandler>([=] (ScriptContext& context)
		{
/*#if USE_OPTICK
			static std::unordered_map<uint64_t, Optick::EventDescription*> staticDescriptions;

			auto it = staticDescriptions.find(nativeIdentifier);

			if (it == staticDescriptions.end())
			{
				it = staticDescriptions.emplace(nativeIdentifier, Optick::EventDescription::Create(va("NativeHandler %llx", nativeIdentifier), __FILE__, __LINE__, Optick::Color::Azure)).first;
			}

			Optick::Event ev(*it->second);
#endif*/

			// push arguments from the original context
			NativeContextRaw rageContext(context.GetArgumentBuffer(), context.GetArgumentCount());

			CallHandler(rageHandler, nativeIdentifier, rageContext);

			// append vector3 result components
			rageContext.SetVectorResults();
		});
	}

	bool __declspec(safebuffers) ScriptEngine::CallNativeHandler(uint64_t nativeIdentifier, ScriptContext& context)
	{
		auto rageHandler = rage::scrEngine::GetNativeHandler(nativeIdentifier);

		if (rageHandler)
		{
			// push arguments from the original context
			NativeContextRaw rageContext(context.GetArgumentBuffer(), context.GetArgumentCount());

			CallHandler(rageHandler, nativeIdentifier, rageContext);

			// append vector3 result components
			rageContext.SetVectorResults();

			return true;
		}

		return false;
	}

	void ScriptEngine::RegisterNativeHandler(const std::string& nativeName, TNativeHandler function)
	{
		RegisterNativeHandler(HashString(nativeName.c_str()), function);
	}

	void ScriptEngine::RegisterNativeHandler(uint64_t nativeIdentifier, TNativeHandler function)
	{
		g_registeredHandlers[nativeIdentifier] = function;

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
			ScriptContextRaw cfxContext(context->GetArgumentBuffer(), context->GetArgumentCount());

			// call the native
			(*handler)(cfxContext);
		});

		rage::scrEngine::RegisterNativeHandler(nativeIdentifier, reinterpret_cast<rage::scrEngine::NativeHandler>(callStub->GetCode()));
#else
#error No stub generator for this architecture.
#endif
	}
}
