/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if defined(COMPILING_SCRBIND_BASE)
#define SCRBIND_EXPORT DLL_EXPORT
#else
#define SCRBIND_EXPORT
#endif

// include game header
#include <ScriptEngine.h>
#include <ScriptSerialization.h>

// class
template<typename TArg>
struct scrBindArgument
{
	static TArg Get(fx::ScriptContext& cxt, int i)
	{
		return cxt.GetArgument<TArg>(i);
	}
};

template<typename TPtr>
struct scrBindArgument<std::shared_ptr<TPtr>>
{
	using TArg = std::shared_ptr<TPtr>;

	static TArg Get(fx::ScriptContext& cxt, int i)
	{
		return *cxt.GetArgument<TArg*>(i);
	}
};

struct scrObjectRef
{
	size_t length;
	char data[];
};

namespace
{
	template<typename TObject>
	auto DeserializeObject(scrObjectRef* ref)
	{
		return msgpack::unpack(ref->data, ref->length).get().as<TObject>();
	}
}

template<typename TObject>
struct scrBindArgument<std::vector<TObject>>
{
	using TArg = std::vector<TObject>;

	static TArg Get(fx::ScriptContext& cxt, int i)
	{
		return DeserializeObject<TArg>(cxt.CheckArgument<scrObjectRef*>(i));
	}
};

template<typename TObject>
struct scrBindArgument<std::vector<TObject>&&>
{
	using TArg = std::vector<TObject>;

	static TArg Get(fx::ScriptContext& cxt, int i)
	{
		return DeserializeObject<TArg>(cxt.CheckArgument<scrObjectRef*>(i));
	}
};

template<typename TArg>
struct scrBindResult
{
	static void Set(fx::ScriptContext& cxt, const TArg& result)
	{
		cxt.SetResult(result);
	}
};

template<typename TPtr>
struct scrBindResult<std::shared_ptr<TPtr>>
{
	static void Set(fx::ScriptContext& cxt, std::shared_ptr<TPtr> ref)
	{
		// TODO: destroy natives?
		cxt.SetResult(new std::shared_ptr<TPtr>(ref));
	}
};

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
		static constexpr int GetArgumentCount()
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
		static constexpr int GetArgumentCount()
		{
			return sizeof...(Args);
		}

		static TRet Call(TObj* object, TFunc fn, Args... args)
		{
			return (object->*fn)(args...);
		}
	};

	template<class TObj, class TRet, typename... Args>
	class scrBindFunc<TRet(TObj::*) (Args...) const>
	{
		typedef TRet(TObj::* TFunc)(Args...) const;

	public:
		static constexpr int GetArgumentCount()
		{
			return sizeof...(Args);
		}

		static TRet Call(const TObj* object, TFunc fn, Args... args)
		{
			return (object->*fn)(args...);
		}
	};

	template<class TObj, class TRet, typename... Args>
	class scrBindFunc<TRet(*) (TObj*, Args...)>
	{
		typedef TRet(*TFunc)(TObj*, Args...);

	public:
		static constexpr int GetArgumentCount()
		{
			return sizeof...(Args);
		}

		static TRet Call(TObj* object, TFunc fn, Args... args)
		{
			return fn(object, args...);
		}
	};

	template<class TFunc, class TClass>
	struct scrBindConstructor
	{

	};

	template<class TClass>
	struct scrBindConstructor<void(*)(), TClass>
	{
		static void Call(fx::ScriptContext& context)
		{
			context.SetResult(new TClass());
		}
	};

	template<class TClass, typename A1>
	struct scrBindConstructor<void(*)(A1), TClass>
	{
		static void Call(fx::ScriptContext& context)
		{
			context.SetResult(new TClass(
				context.GetArgument<A1>(0)
			));
		}
	};

	template<class TClass, typename A1, typename A2>
	struct scrBindConstructor<void(*)(A1, A2), TClass>
	{
		static void Call(fx::ScriptContext& context)
		{
			context.SetResult(new TClass(
				context.GetArgument<A1>(0),
				context.GetArgument<A2>(1)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3>
	struct scrBindConstructor<void(*)(A1, A2, A3), TClass>
	{
		static void Call(fx::ScriptContext& context)
		{
			context.SetResult(new TClass(
				context.GetArgument<A1>(0),
				context.GetArgument<A2>(1),
				context.GetArgument<A3>(2)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3, typename A4>
	struct scrBindConstructor<void(*)(A1, A2, A3, A4), TClass>
	{
		static void Call(fx::ScriptContext& context)
		{
			context.SetResult(new TClass(
				context.GetArgument<A1>(0),
				context.GetArgument<A2>(1),
				context.GetArgument<A3>(2),
				context.GetArgument<A4>(3)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3, typename A4, typename A5>
	struct scrBindConstructor<void(*)(A1, A2, A3, A4, A5), TClass>
	{
		static void Call(fx::ScriptContext& context)
		{
			context.SetResult(new TClass(
				context.GetArgument<A1>(0),
				context.GetArgument<A2>(1),
				context.GetArgument<A3>(2),
				context.GetArgument<A4>(3),
				context.GetArgument<A5>(4)
			));
		}
	};

	template<class TClass, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
	struct scrBindConstructor<void(*)(A1, A2, A3, A4, A5, A6), TClass>
	{
		static void Call(fx::ScriptContext& context)
		{
			context.SetResult(new TClass(
				context.GetArgument<A1>(0),
				context.GetArgument<A2>(1),
				context.GetArgument<A3>(2),
				context.GetArgument<A4>(3),
				context.GetArgument<A5>(4),
				context.GetArgument<A6>(5)
			));
		}
	};

	template<class TFunc, class TClass>
	struct scrBindMethod
	{

	};

	template<class TClass, typename TRet, typename... TArgs>
	struct scrBindMethod<TRet(TClass::*)(TArgs...), TClass>
	{
		typedef TRet(TClass::*TFunc)(TArgs...);

		template<size_t... Is>
		static TRet CallInternal(TClass* obj, TFunc func, fx::ScriptContext& context, std::index_sequence<Is...>)
		{
			return (obj->*func)(
				scrBindArgument<TArgs>::Get(context, Is + 1)...
			);
		}

		static TRet Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			return CallInternal(obj, func, context, std::make_index_sequence<sizeof...(TArgs)>());
		}
	};

	template<class TClass, typename TRet, typename... TArgs>
	struct scrBindMethod<TRet(*)(TClass*, TArgs...), TClass>
	{
		typedef TRet(*TFunc)(TClass*, TArgs...);

		template<size_t... Is>
		static TRet CallInternal(TClass* obj, TFunc func, fx::ScriptContext& context, std::index_sequence<Is...>)
		{
			return func(
				obj,
				scrBindArgument<TArgs>::Get(context, Is + 1)...
				);
		}

		static TRet Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			return CallInternal(obj, func, context, std::make_index_sequence<sizeof...(TArgs)>());
		}
	};

	template<class TFunc>
	struct scrBindGlobalMethod
	{

	};

	template<typename TRet, typename... TArgs>
	struct scrBindGlobalMethod<TRet(*)(TArgs...)>
	{
		typedef TRet(*TFunc)(TArgs...);

		template<size_t... Is>
		static TRet CallInternal(TFunc func, fx::ScriptContext& context, std::index_sequence<Is...>)
		{
			return func(
				scrBindArgument<TArgs>::Get(context, Is)...
			);
		}

		static TRet Call(TFunc func, fx::ScriptContext& context)
		{
			return CallInternal(func, context, std::make_index_sequence<sizeof...(TArgs)>());
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResultSpec
	{
		typedef TRet(TClass::* TFunc)(Args...);

		static void Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindResult<TRet>::Set(context, scrBindMethod<TFunc, TClass>::Call(obj, func, context));
		}
	};

	template<class TClass, typename... Args>
	struct scrBindCallResultSpec<TClass, void, Args...>
	{
		typedef void(TClass::* TFunc)(Args...);

		static void Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindMethod<TFunc, TClass>::Call(obj, func, context);
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallStaticResultSpec
	{
		typedef TRet(*TFunc)(TClass*, Args...);

		static void Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindResult<TRet>::Set(context, scrBindMethod<TFunc, TClass>::Call(obj, func, context));
		}
	};

	template<class TClass, typename... Args>
	struct scrBindCallStaticResultSpec<TClass, void, Args...>
	{
		typedef void(*TFunc)(TClass*, Args...);

		static void Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindMethod<TFunc, TClass>::Call(obj, func, context);
		}
	};

	template<class TRet, typename... Args>
	struct scrBindCallGlobalResultSpec
	{
		typedef TRet(*TFunc)(Args...);

		static void Call(TFunc func, fx::ScriptContext& context)
		{
			scrBindResult<TRet>::Set(context, scrBindGlobalMethod<TFunc>::Call(func, context));
		}
	};

	template<typename... Args>
	struct scrBindCallGlobalResultSpec<void, Args...>
	{
		typedef void(*TFunc)(Args...);

		static void Call(TFunc func, fx::ScriptContext& context)
		{
			scrBindGlobalMethod<TFunc>::Call(func, context);
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

		static void Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindCallResultSpec<TClass, TRet, Args...>::Call(obj, func, context);
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResult<std::shared_ptr<TClass>, TRet(TClass::*)(Args...)>
	{
		typedef TRet(TClass::* TFunc)(Args...);

		static void Call(std::shared_ptr<TClass>* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindCallResultSpec<TClass, TRet, Args...>::Call(obj->get(), func, context);
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResult<TClass, TRet(TClass::*)(Args...) const>
	{
		typedef TRet(TClass::* TFunc)(Args...) const;

		static void Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindCallResultSpec<TClass, TRet, Args...>::Call(obj, reinterpret_cast<TRet(TClass::*)(Args...)>(func), context);
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResult<std::shared_ptr<TClass>, TRet(TClass::*)(Args...) const>
	{
		typedef TRet(TClass::* TFunc)(Args...) const;

		static void Call(std::shared_ptr<TClass>* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindCallResultSpec<TClass, TRet, Args...>::Call(obj->get(), reinterpret_cast<TRet(TClass::*)(Args...)>(func), context);
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResult<TClass, TRet(*)(TClass*, Args...)>
	{
		typedef TRet(* TFunc)(TClass*, Args...);

		static void Call(TClass* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindCallStaticResultSpec<TClass, TRet, Args...>::Call(obj, func, context);
		}
	};

	template<class TClass, class TRet, typename... Args>
	struct scrBindCallResult<std::shared_ptr<TClass>, TRet(*)(TClass*, Args...)>
	{
		typedef TRet(* TFunc)(TClass*, Args...);

		static void Call(std::shared_ptr<TClass>* obj, TFunc func, fx::ScriptContext& context)
		{
			scrBindCallStaticResultSpec<TClass, TRet, Args...>::Call(obj->get(), func, context);
		}
	};

	template<class TFunc>
	struct scrBindCallGlobalResult
	{

	};

	template<class TRet, typename... Args>
	struct scrBindCallGlobalResult<TRet(*)(Args...)>
	{
		typedef TRet(*TFunc)(Args...);

		static void Call(TFunc func, fx::ScriptContext& context)
		{
			scrBindCallGlobalResultSpec<TRet, Args...>::Call(func, context);
		}
	};
}

void SCRBIND_EXPORT scrBindAddSafePointer(void* classPtr);

bool SCRBIND_EXPORT scrBindIsSafePointer(void* classPtr);

typedef void(*scrBindNativeMethodStub)(fx::ScriptContext&, void*);

fx::TNativeHandler SCRBIND_EXPORT scrBindCreateNativeMethodStub(scrBindNativeMethodStub, void*);

template<class TClass, class TSelf>
class scrBindClassBase
{
public:
	// define a constructor for the class
	template<class TFunc>
	TSelf& AddConstructor(const char* constructorName)
	{
		fx::ScriptEngine::RegisterNativeHandler(constructorName, [] (fx::ScriptContext& context)
		{
			if (context.GetArgumentCount() != scrBindFunc<TFunc>::GetArgumentCount() + 1)
			{
				return;
			}

			scrBindConstructor<TFunc, TClass>::Call(context);

			scrBindAddSafePointer(context.GetResult<void*>());
		});

		return *static_cast<TSelf*>(this);
	}

	TSelf& AddDestructor(const char* destructorName)
	{
		fx::ScriptEngine::RegisterNativeHandler(destructorName, [](fx::ScriptContext& context)
		{
			if (context.GetArgumentCount() != 1)
			{
				return;
			}

			auto ptr = context.GetArgument<TClass*>(0);

			if (!scrBindIsSafePointer(ptr))
			{
				return;
			}
			
			delete ptr;
		});

		return *static_cast<TSelf*>(this);
	}

	template<class TFunc>
	TSelf& AddMethod(const char* methodName, TFunc method)
	{
		fx::ScriptEngine::RegisterNativeHandler(methodName, scrBindCreateNativeMethodStub([] (fx::ScriptContext& context, void* ufunc)
		{
			TFunc* udata = (TFunc*)ufunc;

			if (context.GetArgumentCount() != scrBindFunc<TFunc>::GetArgumentCount() + 1)
			{
				return;
			}

			TClass* obj = context.GetArgument<TClass*>(0);

			if (!scrBindIsSafePointer(obj))
			{
				return;
			}

			scrBindCallResult<TClass, TFunc>::Call(obj, *udata, context);
		}, new TFunc(method)));

		return *static_cast<TSelf*>(this);
	}
};

template<class TFunc>
void scrBindGlobal(const char* methodName, TFunc method)
{
	fx::ScriptEngine::RegisterNativeHandler(methodName, scrBindCreateNativeMethodStub([](fx::ScriptContext& context, void* ufunc)
	{
		TFunc* udata = (TFunc*)ufunc;

		if (context.GetArgumentCount() != scrBindFunc<TFunc>::GetArgumentCount())
		{
			return;
		}

		scrBindCallGlobalResult<TFunc>::Call(*udata, context);
	}, new TFunc(method)));
}

template<class TClass>
class scrBindClass
	: public scrBindClassBase<TClass, scrBindClass<TClass>>
{

};

template<class TClass>
class scrBindClass<std::shared_ptr<TClass>>
	: public scrBindClassBase<std::shared_ptr<TClass>, scrBindClass<std::shared_ptr<TClass>>>
{
public:
	scrBindClass& AddDestructor(const char* destructorName)
	{
		fx::ScriptEngine::RegisterNativeHandler(destructorName, [](fx::ScriptContext& context)
		{
			if (context.GetArgumentCount() != 1)
			{
				return;
			}

			auto ptr = context.GetArgument<std::shared_ptr<TClass>*>(0);

			if (!scrBindIsSafePointer(ptr))
			{
				return;
			}

			delete ptr;
		});

		return *this;
	}
};
