/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <console/Console.h>

namespace fx
{
	template<class RuntimeType>
	static RuntimeType* GetScriptRuntimeFromArgs(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		return reinterpret_cast<RuntimeType*>(v8::Local<v8::External>::Cast(args.Data())->Value());
	}

	template<class RuntimeType>
	void ScriptTraceV(RuntimeType* rt, const char* string, fmt::printf_args formatList)
	{
		auto t = fmt::vsprintf(string, formatList);
		console::Printf(fmt::sprintf("script:%s", rt->GetResourceName()), "%s", t);

		rt->GetScriptHost()->ScriptTrace(const_cast<char*>(t.c_str()));
	}

	template<class RuntimeType, typename... TArgs>
	void ScriptTrace(RuntimeType* rt, const char* string, const TArgs& ... args)
	{
		ScriptTraceV(rt, string, fmt::make_printf_args(args...));
	}

	static std::chrono::milliseconds msec()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	}

	template<typename Func>
	static void V8_TryCatch(const v8::FunctionCallbackInfo<v8::Value>& args, Func&& func)
	{
		v8::Local<v8::Value> exception;

		try
		{
			return func(args);
		}
		catch (const std::exception& ex)
		{
			exception = v8::Exception::Error(v8::String::NewFromUtf8(args.GetIsolate(), ex.what()).ToLocalChecked());
		}

		// Throw the error after cleaning up the stack
		args.GetIsolate()->ThrowException(exception);
	}

	struct V8Boundary
	{
		int hint;
	};

	// scope aware nodejs environment setup, from runtime
	template<class C>
	class SharedPushEnvironment
	{
	private:
		v8::Locker m_locker;
		v8::Isolate::Scope m_isolateScope;
		v8::HandleScope m_handleScope;
		v8::Local<v8::Context> m_ctx;
		v8::Context::Scope m_contextScope;

	public:
		SharedPushEnvironment(const C* runtime) :
			m_locker(runtime->GetIsolate()),
			m_isolateScope(runtime->GetIsolate()),
			m_handleScope(runtime->GetIsolate()),
			m_ctx(runtime->GetContext()),
			m_contextScope(m_ctx) {}

		~SharedPushEnvironment() {}
	};

	template<class C>
	class SharedPushEnvironmentNoIsolate
	{
	private:
		v8::HandleScope m_handleScope;
		v8::Local<v8::Context> m_ctx;
		v8::Context::Scope m_contextScope;

	public:
		SharedPushEnvironmentNoIsolate(const C* runtime) :
			m_handleScope(runtime->GetIsolate()),
			m_ctx(runtime->GetContext()),
			m_contextScope(m_ctx) {}

		~SharedPushEnvironmentNoIsolate() {}
	};

	// scope aware nodejs environment setup without context, from isolate
	class SharedPushEnvironmentNoContext
	{
	private:
		v8::Locker m_locker;
		v8::Isolate::Scope m_isolateScope;
		v8::HandleScope m_handleScope;

	public:
		SharedPushEnvironmentNoContext(v8::Isolate* isolate) :
			m_locker(isolate),
			m_isolateScope(isolate),
			m_handleScope(isolate) {}

		~SharedPushEnvironmentNoContext() {}
	};
}
