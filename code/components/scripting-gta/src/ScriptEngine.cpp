/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include "Hooking.h"

#include <Brofiler.h>

#include <scrEngine.h>

template<typename THandler>
static inline void CallHandler(const THandler& rageHandler, uint64_t nativeIdentifier, rage::scrNativeCallContext& rageContext)
{
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
}

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
/*#if USE_PROFILER
			static std::unordered_map<uint64_t, Profiler::EventDescription*> staticDescriptions;

			auto it = staticDescriptions.find(nativeIdentifier);

			if (it == staticDescriptions.end())
			{
				it = staticDescriptions.emplace(nativeIdentifier, Profiler::EventDescription::Create(va("NativeHandler %llx", nativeIdentifier), __FILE__, __LINE__, Profiler::Color::Azure)).first;
			}

			Profiler::Event ev(*it->second);
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
