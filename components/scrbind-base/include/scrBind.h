/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if defined(COMPILING_SCRBIND_BASE)
#define SCRBIND_EXPORT __declspec(dllexport)
#else
#define SCRBIND_EXPORT
#endif

// include RAGE header
#include <scrEngine.h>

// class
namespace
{
	template<class TFunc>
	class scrBindFunc
	{
		
	};

	template<class TRet, typename... Args>
	class scrBindFunc<TRet(*) (Args...)>
	{
	typedef TRet(* TFunc)(Args...);

	public:
		// TODO: make constexpr when migrating to VS14
		static int GetArgumentCount()
		{
			return sizeof...(Args);
		}

		static TRet Call(TFunc fn, Args... args)
		{
			return fn(args...);
		}
	};

	template<class TObj, class TRet, typename... Args>
	class scrBindFunc<TRet(TObj::*) (Args...)>
	{
	typedef TRet(TObj::* TFunc)(Args...);

	public:
		// TODO: make constexpr when migrating to VS14
		static int GetArgumentCount()
		{
			return sizeof...(Args);
		}

		static TRet Call(TObj* object, TFunc fn, Args... args)
		{
			return (object->*fn)(args...);
		}
	};

	template<class TFunc, class TClass>
	struct scrBindConstructor
	{

	};

	template<class TClass>
	struct scrBindConstructor<void(*)(), TClass>
	{
		static void Call(rage::scrNativeCallContext* context)
		{
			context->SetResult(0, new TClass());
		}
	};

	template<class TClass, typename A1>
	struct scrBindConstructor<void(*)(A1), TClass>
	{
		static void Call(rage::scrNativeCallContext* context)
		{
			context->SetResult(0, new TClass(
				context->GetArgument<A1>(0)
			));
		}
	};

	template<class TClass, typename A1, typename A2>
	struct scrBindConstructor<void(*)(A1, A2), TClass>
	{
		static void Call(rage::scrNativeCallContext* context)
		{
			context->SetResult(0, new TClass(
				context->GetArgument<A1>(0),
				context->GetArgument<A2>(1)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3>
	struct scrBindConstructor<void(*)(A1, A2, A3), TClass>
	{
		static void Call(rage::scrNativeCallContext* context)
		{
			context->SetResult(0, new TClass(
				context->GetArgument<A1>(0),
				context->GetArgument<A2>(1),
				context->GetArgument<A3>(2)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3, typename A4>
	struct scrBindConstructor<void(*)(A1, A2, A3, A4), TClass>
	{
		static void Call(rage::scrNativeCallContext* context)
		{
			context->SetResult(0, new TClass(
				context->GetArgument<A1>(0),
				context->GetArgument<A2>(1),
				context->GetArgument<A3>(2),
				context->GetArgument<A4>(3)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3, typename A4, typename A5>
	struct scrBindConstructor<void(*)(A1, A2, A3, A4, A5), TClass>
	{
		static void Call(rage::scrNativeCallContext* context)
		{
			context->SetResult(0, new TClass(
				context->GetArgument<A1>(0),
				context->GetArgument<A2>(1),
				context->GetArgument<A3>(2),
				context->GetArgument<A4>(3),
				context->GetArgument<A5>(4)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	struct scrBindConstructor<void(*)(A1, A2, A3, A4, A5, A6), TClass>
	{
		static void Call(rage::scrNativeCallContext* context)
		{
			context->SetResult(0, new TClass(
				context->GetArgument<A1>(0),
				context->GetArgument<A2>(1),
				context->GetArgument<A3>(2),
				context->GetArgument<A4>(3),
				context->GetArgument<A5>(4),
				context->GetArgument<A6>(5)
			));
		}
	};

	template<class TFunc, class TClass>
	struct scrBindMethod
	{

	};

	template<class TClass, typename TRet>
	struct scrBindMethod<TRet(TClass::*)(), TClass>
	{
		typedef TRet(TClass::*TFunc)();

		static TRet Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			return (obj->*func)();
		}
	};

	template<class TClass, typename TRet, typename A1>
	struct scrBindMethod<TRet(TClass::*)(A1), TClass>
	{
		typedef TRet(TClass::*TFunc)(A1);

		static TRet Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			return (obj->*func)(
				context->GetArgument<A1>(1)
			);
		}
	};

	template<class TClass, typename TRet, typename A1, typename A2>
	struct scrBindMethod<TRet(TClass::*)(A1, A2), TClass>
	{
		typedef TRet(TClass::*TFunc)(A1, A2);

		static TRet Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			return (obj->*func)(
				context->GetArgument<A1>(1),
				context->GetArgument<A2>(2)
			);
		}
	};

	template<class TClass, typename TRet, typename A1, typename A2, typename A3>
	struct scrBindMethod<TRet(TClass::*)(A1, A2, A3), TClass>
	{
		typedef TRet(TClass::*TFunc)(A1, A2, A3);

		static TRet Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			return (obj->*func)(
				context->GetArgument<A1>(1),
				context->GetArgument<A2>(2),
				context->GetArgument<A3>(3)
			);
		}
	};

	template<class TClass, typename TRet, typename A1, typename A2, typename A3, typename A4>
	struct scrBindMethod<TRet(TClass::*)(A1, A2, A3, A4), TClass>
	{
		typedef TRet(TClass::*TFunc)(A1, A2, A3, A4);

		static TRet Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			return (obj->*func)(
				context->GetArgument<A1>(1),
				context->GetArgument<A2>(2),
				context->GetArgument<A3>(3),
				context->GetArgument<A4>(4)
			);
		}
	};

	template<class TClass, typename TRet, typename A1, typename A2, typename A3, typename A4, typename A5>
	struct scrBindMethod<TRet(TClass::*)(A1, A2, A3, A4, A5), TClass>
	{
		typedef TRet(TClass::*TFunc)(A1, A2, A3, A4, A5);

		static TRet Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			return (obj->*func)(
				context->GetArgument<A1>(1),
				context->GetArgument<A2>(2),
				context->GetArgument<A3>(3),
				context->GetArgument<A4>(4),
				context->GetArgument<A5>(5)
			);
		}
	};

	template<class TClass, typename TRet, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	struct scrBindMethod<TRet(TClass::*)(A1, A2, A3, A4, A5, A6), TClass>
	{
		typedef TRet(TClass::*TFunc)(A1, A2, A3, A4, A5, A6);

		static TRet Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			return (obj->*func)(
				context->GetArgument<A1>(1),
				context->GetArgument<A2>(2),
				context->GetArgument<A3>(3),
				context->GetArgument<A4>(4),
				context->GetArgument<A5>(5),
				context->GetArgument<A6>(6)
			);
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResultSpec
	{
		typedef TRet(TClass::* TFunc)(Args...);

		static void Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			context->SetResult(0, scrBindMethod<TFunc, TClass>::Call(obj, func, context));
		}
	};

	template<class TClass, typename... Args>
	struct scrBindCallResultSpec<TClass, void, Args...>
	{
		typedef void(TClass::* TFunc)(Args...);

		static void Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			scrBindMethod<TFunc, TClass>::Call(obj, func, context);
		}
	};

	template<class TFunc, class TClass>
	struct scrBindCallResult
	{

	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResult<TClass, TRet(TClass::*)(Args...)>
	{
		typedef TRet(TClass::* TFunc)(Args...);

		static void Call(TClass* obj, TFunc func, rage::scrNativeCallContext* context)
		{
			scrBindCallResultSpec<TClass, TRet, Args...>::Call(obj, func, context);
		}
	};
}

void SCRBIND_EXPORT scrBindAddSafePointer(void* classPtr);

bool SCRBIND_EXPORT scrBindIsSafePointer(void* classPtr);

typedef void(*scrBindNativeMethodStub)(rage::scrNativeCallContext*, void*);

rage::scrEngine::NativeHandler SCRBIND_EXPORT scrBindCreateNativeMethodStub(scrBindNativeMethodStub, void*);

template<class TClass>
class scrBindClass
{
public:
	// define a constructor for the class
	template<class TFunc>
	scrBindClass& AddConstructor(const char* constructorName)
	{
		rage::scrEngine::RegisterNativeHandler(constructorName, [] (rage::scrNativeCallContext* context)
		{
			if (context->GetArgumentCount() != scrBindFunc<TFunc>::GetArgumentCount())
			{
				return;
			}

			scrBindConstructor<TFunc, TClass>::Call(context);

			scrBindAddSafePointer(context->GetResult<void*>(0));
		});

		return *this;
	}

	template<class TFunc>
	scrBindClass& AddMethod(const char* methodName, TFunc method)
	{
		rage::scrEngine::RegisterNativeHandler(methodName, scrBindCreateNativeMethodStub([] (rage::scrNativeCallContext* context, void* ufunc)
		{
			TFunc* udata = (TFunc*)ufunc;

			if (context->GetArgumentCount() != scrBindFunc<TFunc>::GetArgumentCount() + 1)
			{
				return;
			}

			TClass* obj = context->GetArgument<TClass*>(0);

			if (!scrBindIsSafePointer(obj))
			{
				return;
			}

			scrBindCallResult<TClass, TFunc>::Call(obj, *udata, context);
		}, new TFunc(method)));

		return *this;
	}
};