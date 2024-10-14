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

// This pool needs to support arbitrary types, support up/down casting, and work across dll boundaries

// Override this to allow upcasting pointers
template <typename T>
struct scrBindParent
{
	using type = void;
};

#ifdef _WIN32
#define SCRBIND_TYPE_NAME(EXPR) typeid(EXPR).raw_name()
#else
#define SCRBIND_TYPE_NAME(EXPR) typeid(EXPR).name()
#endif

struct scrBindPointer
{
	uint32_t Handle;
	std::shared_ptr<void> Pointer;

	scrBindPointer(uint32_t handle, std::shared_ptr<void> pointer)
		: Handle(handle), Pointer(std::move(pointer))
	{}

	virtual ~scrBindPointer() = default;

	virtual void* Cast(std::string_view type) = 0;

	template<typename T>
	std::shared_ptr<T> Cast()
	{
		if (void* ptr = Cast(SCRBIND_TYPE_NAME(T)))
			return std::shared_ptr<T> { Pointer, static_cast<T*>(ptr) };

		return nullptr;
	}
};

template<typename T, typename U = T, typename Parent = typename scrBindParent<U>::type>
struct scrBindPointerT
	: scrBindPointerT<T, Parent>
{
	using scrBindPointerT<T, Parent>::scrBindPointerT;

	void* Cast(std::string_view type) override
	{
		return (type == SCRBIND_TYPE_NAME(U))
			? static_cast<void*>(static_cast<U*>(static_cast<T*>(this->Pointer.get())))
			: scrBindPointerT<T, Parent>::Cast(type);
	}
};

template<typename T>
struct scrBindPointerT<T, void, void>
	: scrBindPointer
{
	using scrBindPointer::scrBindPointer;

	void* Cast(std::string_view type) override
	{
		// One final attempt to cast based on the pointer's actual typeid (might be different if it's polymorphic)
		return (type == SCRBIND_TYPE_NAME(*static_cast<T*>(this->Pointer.get())))
			? this->Pointer.get()
			: nullptr;
	}
};

struct scrBindPool
{
	std::unordered_map<uint32_t, std::unique_ptr<scrBindPointer>> Handles;
	std::unordered_map<void*, scrBindPointer*> Pointers;
	uint32_t UID = 0;

	template<typename T>
	uint32_t MakeHandle(std::shared_ptr<T> value)
	{
		if (value == nullptr)
		{
			return 0;
		}

		auto find = Pointers.find(value.get());

		if (find != Pointers.end())
			return find->second->Handle;

		// "Randomize" the handles to make it more obvious when passing in an incorrect value.
		uint32_t handle = ++UID;
		handle += 0xDEADBEEF;
		handle ^= 0xDEADBEEF;
		handle *= 0x9E3779B9;

		auto data = Handles.emplace(handle, std::make_unique<scrBindPointerT<T>>(handle, value));

		if (!data.second)
		{
			throw std::runtime_error("scrbind handle collision");
		}

		Pointers.emplace(value.get(), data.first->second.get());

		return handle;
	}

	template<typename T>
	std::shared_ptr<T> FromHandle(uint32_t handle)
	{
		if (auto find = Handles.find(handle); find != Handles.end())
		{
			if (auto ptr = find->second->Cast<T>())
				return ptr;
		}

		throw std::runtime_error("invalid handle");
	}

	template <typename T>
	void ErasePointer(std::shared_ptr<T> pointer)
	{
		auto find = Pointers.find(pointer.get());

		if (find != Pointers.end())
		{
			Handles.erase(find->second->Handle);
			Pointers.erase(find);
		}
	}

	void Reset()
	{
		Pointers.clear();
		Handles.clear();
		UID = 0;
	}
};

#ifdef COMPILING_SCRBIND_BASE
DLL_EXPORT
#else
DLL_IMPORT
#endif
extern scrBindPool g_ScrBindPool;

// TODO: Add PAS_ARG_OBJECT type
struct scrObjectRef
{
	size_t length;
	char data[];
};

template<typename TArg, typename = void>
struct scrBindArgument
{
	static TArg Get(fx::ScriptContext& cxt, int i)
	{
		if constexpr (std::is_trivial_v<TArg>)
		{
			return cxt.GetArgument<TArg>(i);
		}
		else
		{
			scrObjectRef* ref = cxt.CheckArgument<scrObjectRef*>(i);

			return msgpack::unpack(ref->data, ref->length).get().as<TArg>();
		}
	}
};

template<typename TPtr>
struct scrBindArgument<TPtr*, std::enable_if_t<std::is_class_v<TPtr>>>
{
	using TArg = TPtr*;

	static TArg Get(fx::ScriptContext& cxt, int i)
	{
		// The shared pointer will be destroyed after returning, but the underlying object should still exist.
		return g_ScrBindPool.FromHandle<TPtr>(cxt.GetArgument<uint32_t>(i)).get();
	}
};

template<typename TPtr>
struct scrBindArgument<std::shared_ptr<TPtr>>
{
	using TArg = std::shared_ptr<TPtr>;

	static TArg Get(fx::ScriptContext& cxt, int i)
	{
		return g_ScrBindPool.FromHandle<TPtr>(cxt.GetArgument<uint32_t>(i));
	}
};

template<typename TArg, typename = void>
struct scrBindResult
{
	static void Set(fx::ScriptContext& cxt, const TArg& result)
	{
		cxt.SetResult(result);
	}
};

template<typename TPtr>
struct scrBindResult<TPtr*, std::enable_if_t<std::is_class_v<TPtr>>>
{
	// Don't allow returning raw object pointers
	static void Set(fx::ScriptContext& cxt, TPtr* ref) = delete;
};

template<typename TPtr>
struct scrBindResult<std::shared_ptr<TPtr>>
{
	using TArg = std::shared_ptr<TPtr>;

	static void Set(fx::ScriptContext& cxt, TArg ref)
	{
		cxt.SetResult(g_ScrBindPool.MakeHandle(ref));
	}
};

template<typename Is, typename... Args>
struct scrBindFunctionBase;

template<size_t... Is, typename... Args>
struct scrBindFunctionBase<std::index_sequence<Is...>, Args...>
{
	template<typename Func>
	static auto Bind(const Func& func)
	{
		return [func](fx::ScriptContext& context)
		{
			if (context.GetArgumentCount() != sizeof...(Args))
			{
				throw std::runtime_error("invalid argument count");
			}

			const auto invoke = [&func, &context]
			{
				return std::invoke(func, scrBindArgument<std::decay_t<Args>>::Get(context, (int)Is)...);
			};

			using Result = decltype(invoke());

			if constexpr (std::is_same_v<Result, void>)
			{
				invoke();
			}
			else
			{
				scrBindResult<Result>::Set(context, invoke());
			}
		};
	}
};

template<typename... Args>
struct scrBindFunction
	: scrBindFunctionBase<std::make_index_sequence<sizeof...(Args)>, Args...>
{

};

template <typename Result, typename... Args>
void scrBindGlobal(const char* methodName, Result(*method)(Args...))
{
	fx::ScriptEngine::RegisterNativeHandler(methodName, scrBindFunction<Args...>::Bind(method));
}

template<class TClass>
class scrBindClass
{
public:
	// define a constructor for the class
	template<typename... Args>
	scrBindClass& AddConstructor(const char* constructorName)
	{
		fx::ScriptEngine::RegisterNativeHandler(constructorName, scrBindFunction<Args...>::Bind([](const Args&... args)
		{
			return std::make_shared<TClass>(args...);
		}));
			
		return *this;
	}

	scrBindClass& AddDestructor(const char* destructorName)
	{
		fx::ScriptEngine::RegisterNativeHandler(destructorName, scrBindFunction<std::shared_ptr<TClass>>::Bind([](std::shared_ptr<TClass> pointer)
		{
			g_ScrBindPool.ErasePointer(std::move(pointer));
		}));

		return *this;
	}

	template<typename Result, typename... Args>
	scrBindClass& AddMethod(const char* methodName, Result (TClass::*method)(Args...))
	{
		fx::ScriptEngine::RegisterNativeHandler(methodName, scrBindFunction<TClass*, Args...>::Bind(method));

		return *this;
	}

	template<typename Result, typename... Args>
	scrBindClass& AddMethod(const char* methodName, Result (TClass::*method)(Args...) const)
	{
		fx::ScriptEngine::RegisterNativeHandler(methodName, scrBindFunction<TClass*, Args...>::Bind(method));

		return *this;
	}

	template<typename Result, typename... Args>
	scrBindClass& AddMethod(const char* methodName, Result(*method)(TClass*, Args...))
	{
		fx::ScriptEngine::RegisterNativeHandler(methodName, scrBindFunction<TClass*, Args...>::Bind(method));

		return *this;
	}
};
