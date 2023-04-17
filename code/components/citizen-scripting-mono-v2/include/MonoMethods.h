// explicit include guard as we may include this with different names
#ifndef MONO_METHODS_H
#define MONO_METHODS_H

#include "StdInc.h"
#include <string>

#include <mono/metadata/object.h>
#include <mono/metadata/debug-helpers.h>

#ifndef _WIN32
#define __stdcall
#endif

namespace fx::mono
{
	// these classes are for proper construction, destruction, usage, and convenience.
	class Method
	{
	private:
		MonoMethod* method;

	public:
		Method() : method(nullptr) {}
		Method(std::nullptr_t) : method(nullptr) {}
		Method(MonoMethod* method) : method(method) {}

		~Method()
		{
			if (method)
				mono_free_method(method);
		}

		// Searches the image for the given function and if found turns it into a method, slow operation.
		static Method Find(MonoImage* image, const char* fullName, bool includeNamespace = true)
		{
			MonoMethodDesc* description = mono_method_desc_new(fullName, includeNamespace);
			MonoMethod* method = mono_method_desc_search_in_image(description, image);
			mono_method_desc_free(description);

			return Method(method);
		}

		// Doesn't free param arguments
		inline MonoObject* operator()(void* _this, void** param = nullptr, MonoException** exc = nullptr)
		{
			return mono_runtime_invoke(method, _this, param, (MonoObject**)exc);
		}

		// Doesn't free param arguments, so use { &variable } instead of { new Type() }
		// it is save to use { mono_string_new(domain, str) }, these objects are managed by mono
		inline MonoObject* operator()(void* _this, const std::initializer_list<const void*>& param, MonoException** exc = nullptr)
		{
			return mono_runtime_invoke(method, _this, const_cast<void**>(param.begin()), (MonoObject**)exc);
		}

		// Doesn't free param arguments
		inline MonoObject* operator()(void** param, MonoException** exc = nullptr)
		{
			return mono_runtime_invoke(method, nullptr, param, (MonoObject**)exc);
		}

		// Doesn't free param arguments, so use { &variable } instead of { new Type() }
		// it is save to use { mono_string_new(domain, str) }, these objects are managed by mono
		inline MonoObject* operator()(const std::initializer_list<const void*>& param, MonoException** exc = nullptr)
		{
			return mono_runtime_invoke(method, nullptr, const_cast<void**>(param.begin()), (MonoObject**)exc);
		}

		inline MonoObject* operator()(void* _this, MonoArray* param, MonoException** exc = nullptr)
		{
			return mono_runtime_invoke_array(method, _this, param, (MonoObject**)exc);
		}

		inline MonoObject* operator()(MonoArray* param, MonoException** exc = nullptr)
		{
			return mono_runtime_invoke_array(method, nullptr, param, (MonoObject**)exc);
		}

		inline MonoObject* operator()(MonoException** exc = nullptr)
		{
			return mono_runtime_invoke(method, nullptr, nullptr, (MonoObject**)exc);
		}

		MonoMethod* GetMonoMethod() const
		{
			return method;
		}

		template<typename Ret, typename... Args>
		inline static void AddInternalCall(const char* signature, Ret(__stdcall* function)(Args...))
		{
			mono_add_internal_call(signature, (void*)function);
		}

		Method& operator=(std::nullptr_t)
		{
			if (method)
			{
				mono_free_method(method);
				method = nullptr;
			}

			return *this;
		}

		inline operator bool()
		{
			return method != nullptr;
		}
	};

	template<typename Func>
	class Thunk;

	template<typename Ret, typename... Args> 
	class Thunk<Ret(Args...)>
	{
	public:
		using func_type = Ret(__stdcall*)(Args..., MonoException**);
		using return_type = Ret;

	private:
		func_type func;

		void CreateMonoThunk(const MonoMethod* method)
		{
			func = method == nullptr ? nullptr 
				: reinterpret_cast<func_type>(mono_method_get_unmanaged_thunk(const_cast<MonoMethod*>(method)));
		}

	public:
		Thunk() : func(nullptr) {}
		Thunk(std::nullptr_t) : func(nullptr) {}
		Thunk(func_type func) : func(func) {}

		Thunk(MonoMethod* method)
		{
			CreateMonoThunk(method);
		}

		Thunk(const Method& method)
		{
			CreateMonoThunk(method.GetMonoMethod());
		}
		
		// Searches the image for the given function and if found turns it into a thunk, slow operation.
		static Thunk Find(MonoImage* image, const char* fullName, bool includeNamespace = true)
		{
			Method method = Method::Find(image, fullName, includeNamespace);
			return method ? Thunk(method) : Thunk();
		}

		Thunk& operator=(const Method& copy)
		{
			CreateMonoThunk(copy.GetMonoMethod());
			return *this;
		}

		Thunk& operator=(std::nullptr_t)
		{
			func = nullptr;
			return *this;
		}

		inline return_type operator()(Args... args, MonoException** exc = nullptr)
		{
			return (*func)(args..., exc);
		}

		inline operator bool()
		{
			return func != nullptr;
		}
	};
}

#endif
