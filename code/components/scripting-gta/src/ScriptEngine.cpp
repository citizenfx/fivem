/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <stdexcept>

#include <ScriptEngine.h>

#include <jitasm.h>
#include "Hooking.h"

#include <optick.h>

#include <scrEngine.h>

#include <NativeHandlerLogging.h>
#include <ExceptionToModuleHelper.h>

#include <CL2LaunchMode.h>

#include <Error.h>

static LONG ShouldHandleUnwind(PEXCEPTION_POINTERS ep, DWORD exceptionCode, uint64_t identifier)
{
	if (IsErrorException(ep))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// C++ exceptions?
	if (exceptionCode == 0xE06D7363)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// INVOKE_FUNCTION_REFERENCE crashing as top-level is usually related to native state corruption,
	// we'll likely want to crash on this instead rather than on an assertion down the chain
	if (identifier == HashString("INVOKE_FUNCTION_REFERENCE"))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

template<typename THandler>
static inline void CallHandler(const THandler& rageHandler, uint64_t nativeIdentifier, rage::scrNativeCallContext& rageContext)
{
	// call the original function
	static void* exceptionAddress;

#ifdef ENABLE_NATIVE_HANDLER_LOGGING
	NativeHandlerLogging::CountNative(nativeIdentifier);
#endif

//#ifndef _DEBUG
	__try
	{
//#endif
		rageHandler(&rageContext);
//#ifndef _DEBUG
	}
	__except (exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, ShouldHandleUnwind(GetExceptionInformation(), (GetExceptionInformation())->ExceptionRecord->ExceptionCode, nativeIdentifier))
	{
		throw std::runtime_error(va("Error executing native 0x%016llx at address %s.", nativeIdentifier, FormatModuleAddress(exceptionAddress)));
	}
//#endif
}

static std::string g_lastError;
static void* g_exceptionAddress;

static void SetLastErrorException(uint64_t nativeIdentifier)
{
	g_lastError = fmt::sprintf("Error executing native %016llx at address %s.", nativeIdentifier, FormatModuleAddress(g_exceptionAddress));
}

static bool CallNativeWrapperCppEh(rage::scrEngine::NativeHandler handler, uint64_t nativeIdentifier, rage::scrNativeCallContext* context, char** errorMessage)
{
	try
	{
		handler(context);
		return true;
	}
	catch (const std::exception& e)
	{
		g_lastError = fmt::sprintf("Error executing native %016llx: %s", nativeIdentifier, e.what());
		*errorMessage = const_cast<char*>(g_lastError.c_str());

		return false;
	}
}

static void NullInvocation(uint64_t nativeIdentifier, char** errorMessage)
{
	if (nativeIdentifier == 0)
	{
		auto resourceName = NativeInvoke::Invoke<HashString("GET_CURRENT_RESOURCE_NAME"), const char*>();

		if (!resourceName)
		{
			resourceName = "<unknown>";
		}

		FatalError("Invalid native call\nA script (%s) has invoked an invalid native call (null pointer/null native). This is unsupported behavior, and usually seen as part of a resource attempting to exploit a vulnerability in the Cfx.re platform.", resourceName);
	}

	g_lastError = fmt::sprintf("Invalid native call %016llx", nativeIdentifier);
	*errorMessage = const_cast<char*>(g_lastError.c_str());
}

extern "C" bool DLL_EXPORT WrapNativeInvoke(rage::scrEngine::NativeHandler handler, uint64_t nativeIdentifier, rage::scrNativeCallContext* context, char** errorMessage)
{
	if (!handler)
	{
		NullInvocation(nativeIdentifier, errorMessage);
		return false;
	}

	__try
	{
		return CallNativeWrapperCppEh(handler, nativeIdentifier, context, errorMessage);
	}
	__except (g_exceptionAddress = (GetExceptionInformation())->ExceptionRecord->ExceptionAddress, EXCEPTION_EXECUTE_HANDLER)
	{
		SetLastErrorException(nativeIdentifier);
		*errorMessage = const_cast<char*>(g_lastError.c_str());

		return false;
	}
}

static std::unordered_map<uint64_t, fx::TNativeHandler> g_registeredHandlers;

namespace fx
{
	TNativeHandler ScriptEngine::GetNativeHandler(uint64_t nativeIdentifier)
	{
		auto it = g_registeredHandlers.find(nativeIdentifier);

		if (it != g_registeredHandlers.end())
		{
			return it->second;
		}

		if (launch::IsSDK())
		{
			return nullptr;
		}

		auto rageHandler = rage::scrEngine::GetNativeHandler(nativeIdentifier);

		if (rageHandler == nullptr)
		{
			return nullptr;
		}

		return [=] (ScriptContext& context)
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
			NativeContextRaw rageContext(context.GetArgumentBuffer(), context.GetResultBuffer(), context.GetArgumentCount());

			CallHandler(rageHandler, nativeIdentifier, rageContext);

			// append vector3 result components
			rageContext.SetVectorResults();
		};
	}

	TNativeHandler* ScriptEngine::GetNativeHandlerPtr(uint64_t nativeIdentifier)
	{
		auto it = g_registeredHandlers.find(nativeIdentifier);
		return it != g_registeredHandlers.end() ? &it->second : nullptr;
	}

	static bool __declspec(safebuffers) CallNativeHandlerUniversal(uint64_t nativeIdentifier, ScriptContext& context);

	static bool (*g_callNativeHandler)(uint64_t nativeIdentifier, ScriptContext& context) = CallNativeHandlerUniversal;

	static bool __declspec(safebuffers) CallNativeHandlerSdk(uint64_t nativeIdentifier, ScriptContext& context)
	{
		auto h = ScriptEngine::GetNativeHandlerPtr(nativeIdentifier);

		if (h)
		{
			(*h)(context);

			return true;
		}

		return false;
	}

	static bool __declspec(safebuffers) CallNativeHandlerRage(uint64_t nativeIdentifier, ScriptContext& context)
	{
		auto rageHandler = rage::scrEngine::GetNativeHandler(nativeIdentifier);

		if (rageHandler)
		{
			// push arguments from the original context
			NativeContextRaw rageContext(context.GetArgumentBuffer(), context.GetResultBuffer(), context.GetArgumentCount());

			CallHandler(rageHandler, nativeIdentifier, rageContext);

			// append vector3 result components
			rageContext.SetVectorResults();

			return true;
		}

		return false;
	}

	static bool __declspec(safebuffers) CallNativeHandlerUniversal(uint64_t nativeIdentifier, ScriptContext& context)
	{
		if (launch::IsSDK())
		{
			g_callNativeHandler = CallNativeHandlerSdk;
		}
		else
		{
			g_callNativeHandler = CallNativeHandlerRage;
		}

		return g_callNativeHandler(nativeIdentifier, context);
	}

	bool __declspec(safebuffers) ScriptEngine::CallNativeHandler(uint64_t nativeIdentifier, ScriptContext& context)
	{
		return g_callNativeHandler(nativeIdentifier, context);
	}

	void ScriptEngine::RegisterNativeHandler(const std::string& nativeName, TNativeHandler function)
	{
		RegisterNativeHandler(HashString(nativeName.c_str()), function);
	}

	void ScriptEngine::RegisterNativeHandler(uint64_t nativeIdentifier, TNativeHandler function)
	{
		g_registeredHandlers[nativeIdentifier] = function;

		if (!launch::IsSDK())
		{
#ifdef _M_AMD64
			TNativeHandler* handler = new TNativeHandler(function);

			struct StubGenerator : public jitasm::Frontend
			{
				typedef void (*TFunction)(void*, rage::scrNativeCallContext*);

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
			}* callStub = new StubGenerator(handler, [](void* handlerData, rage::scrNativeCallContext* context)
			{
				// get the handler
				TNativeHandler* handler = reinterpret_cast<TNativeHandler*>(handlerData);

				// turn into a native context
				ScriptContextRaw cfxContext(context->GetArgumentBuffer(), context->GetResultBuffer(), context->GetArgumentCount());

				// call the native
				(*handler)(cfxContext);
			});

			rage::scrEngine::RegisterNativeHandler(nativeIdentifier, reinterpret_cast<rage::scrEngine::NativeHandler>(callStub->GetCode()));
#elif defined(_M_IX86)
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
					push(dword_ptr[esp + 4]);
					push(reinterpret_cast<uintptr_t>(m_handlerData));
					
					mov(eax, reinterpret_cast<uintptr_t>(m_function));
					call(eax);

					add(esp, 8);
					ret();
				}
			}*callStub = new StubGenerator(handler, [](void* handlerData, rage::scrNativeCallContext* context)
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
}
