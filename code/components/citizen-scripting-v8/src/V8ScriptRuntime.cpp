/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fxScripting.h"
#include <ManifestVersion.h>

#include <ResourceCallbackComponent.h>

#include <chrono>
#include <sstream>
#include <stack>

#include <CoreConsole.h>

#ifndef IS_FXSERVER
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
#if defined(IS_RDR3)
	{ "rdr3_universal.js", guid_t{ 0 } }
#else
	{ "natives_universal.js", guid_t{ 0 } }
#endif
};
#else
static constexpr std::pair<const char*, ManifestVersion> g_scriptVersionPairs[] = {
	{ "natives_server.js", guid_t{ 0 } }
};
#endif

#include <v8.h>
#include <v8-profiler.h>
#include <libplatform/libplatform.h>

#include <boost/algorithm/string.hpp>

#ifdef IS_FXSERVER
#include <node.h>
#endif

#include "UvLoopManager.h"

#include <V8Debugger.h>

#include <om/OMComponent.h>

#include <Resource.h>

#include <fstream>

#include <tbb/concurrent_queue.h>

#include <Error.h>

extern int g_argc;
extern char** g_argv;

static const char* g_platformScripts[] = {
	"citizen:/scripting/v8/console.js",
	"citizen:/scripting/v8/timer.js",
	"citizen:/scripting/v8/msgpack.js",
	"citizen:/scripting/v8/eventemitter2.js",
	"citizen:/scripting/v8/main.js"
};

using namespace v8;

namespace fx
{
struct V8Boundary
{
	int hint;
};

Isolate* GetV8Isolate();

static Platform* GetV8Platform();

#ifdef IS_FXSERVER
static node::IsolateData* GetNodeIsolate();
#endif

struct PointerFieldEntry
{
	bool empty;
	uintptr_t value;

	PointerFieldEntry()
	{
		empty = true;
	}
};

struct PointerField
{
	PointerFieldEntry data[64];
};

class V8ScriptRuntime : public OMClass<V8ScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptRefRuntime, IScriptStackWalkingRuntime>
{
private:
	typedef std::function<void(const char*, const char*, size_t, const char*)> TEventRoutine;

	typedef std::function<void(int32_t, const char*, size_t, char**, size_t*)> TCallRefRoutine;

	typedef std::function<int32_t(int32_t)> TDuplicateRefRoutine;

	typedef std::function<void(int32_t)> TDeleteRefRoutine;

	typedef std::function<void(void*, void*, char**, size_t*)> TStackTraceRoutine;

private:
	UniquePersistent<Context> m_context;

#ifdef IS_FXSERVER
	node::Environment* m_nodeEnvironment;
#endif

	std::function<void()> m_tickRoutine;

	TEventRoutine m_eventRoutine;

	TCallRefRoutine m_callRefRoutine;

	TDuplicateRefRoutine m_duplicateRefRoutine;

	TDeleteRefRoutine m_deleteRefRoutine;

	IScriptHost* m_scriptHost;

	IScriptHostWithResourceData* m_resourceHost;

	IScriptHostWithManifest* m_manifestHost;

	int m_instanceId;

	TStackTraceRoutine m_stackTraceRoutine;

	void* m_parentObject;

	PointerField m_pointerFields[3];

	// string values, which need to be persisted across calls as well
	std::unique_ptr<String::Utf8Value> m_stringValues[50];

	int m_curStringValue;

private:
	result_t LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile, Local<Script>* outScript);

	result_t LoadHostFileInternal(char* scriptFile, Local<Script>* outScript);

	result_t LoadSystemFileInternal(char* scriptFile, Local<Script>* outScript);

	result_t RunFileInternal(char* scriptFile, std::function<result_t(char*, Local<Script>*)> loadFunction);

	result_t LoadSystemFile(char* scriptFile);

public:
	inline V8ScriptRuntime()
	{
		m_instanceId = rand() ^ 0x3e3;
		m_curStringValue = 0;
	}

	static const OMPtr<V8ScriptRuntime>& GetCurrent();

	inline Local<Context> GetContext()
	{
		return m_context.Get(GetV8Isolate());
	}

	inline void SetTickRoutine(const std::function<void()>& tickRoutine)
	{
		m_tickRoutine = tickRoutine;
	}

	inline void SetEventRoutine(const TEventRoutine& eventRoutine)
	{
		m_eventRoutine = eventRoutine;
	}

	inline void SetCallRefRoutine(const TCallRefRoutine& routine)
	{
		m_callRefRoutine = routine;
	}

	inline void SetDuplicateRefRoutine(const TDuplicateRefRoutine& routine)
	{
		m_duplicateRefRoutine = routine;
	}

	inline void SetDeleteRefRoutine(const TDeleteRefRoutine& routine)
	{
		m_deleteRefRoutine = routine;
	}

	inline void SetStackTraceRoutine(const TStackTraceRoutine& routine)
	{
		if (!m_stackTraceRoutine)
		{
			m_stackTraceRoutine = routine;
		}
	}

	inline OMPtr<IScriptHost> GetScriptHost()
	{
		return m_scriptHost;
	}

	inline PointerField* GetPointerFields()
	{
		return m_pointerFields;
	}

	inline const char* GetResourceName()
	{
		char* resourceName = "";
		m_resourceHost->GetResourceName(&resourceName);

		return resourceName;
	}

	const char* AssignStringValue(const Local<Value>& value);

	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;

	NS_DECL_ISCRIPTTICKRUNTIME;

	NS_DECL_ISCRIPTEVENTRUNTIME;

	NS_DECL_ISCRIPTREFRUNTIME;

	NS_DECL_ISCRIPTSTACKWALKINGRUNTIME;
};

static OMPtr<V8ScriptRuntime> g_currentV8Runtime;

class V8PushEnvironment
{
private:
	fx::PushEnvironment m_pushEnvironment;

	Locker m_locker;

	Isolate::Scope m_isolateScope;

	HandleScope m_handleScope;

	Context::Scope m_contextScope;

	OMPtr<V8ScriptRuntime> m_lastV8Runtime;

public:
	inline V8PushEnvironment(V8ScriptRuntime* runtime)
		: m_pushEnvironment(runtime), m_locker(GetV8Isolate()), m_isolateScope(GetV8Isolate()), m_handleScope(GetV8Isolate()), m_contextScope(runtime->GetContext())
	{
		m_lastV8Runtime = g_currentV8Runtime;
		g_currentV8Runtime = runtime;
	}

	inline ~V8PushEnvironment()
	{
		g_currentV8Runtime = m_lastV8Runtime;
	}
};

#ifdef IS_FXSERVER
static std::unordered_map<const node::Environment*, V8ScriptRuntime*> g_envRuntimes;

static V8ScriptRuntime* GetEnvRuntime(const node::Environment* env)
{
	return g_envRuntimes[env];
}

class BasePushEnvironment
{
public:
	virtual ~BasePushEnvironment() = default;
};

class V8LitePushEnvironment : public BasePushEnvironment
{
private:
	fx::PushEnvironment m_pushEnvironment;

	Locker m_locker;

	Isolate::Scope m_isolateScope;

	OMPtr<V8ScriptRuntime> m_lastV8Runtime;

public:
	inline V8LitePushEnvironment(V8ScriptRuntime* runtime, const node::Environment* env)
		: m_pushEnvironment(runtime), m_locker(node::GetIsolate(env)), m_isolateScope(node::GetIsolate(env))
	{
		m_lastV8Runtime = g_currentV8Runtime;
		g_currentV8Runtime = runtime;
	}

	inline ~V8LitePushEnvironment()
	{
		g_currentV8Runtime = m_lastV8Runtime;
	}
};

class V8LiteNoRuntimePushEnvironment : public BasePushEnvironment
{
private:
	Locker m_locker;

	Isolate::Scope m_isolateScope;

public:
	inline V8LiteNoRuntimePushEnvironment(const node::Environment* env)
		: m_locker(node::GetIsolate(env)), m_isolateScope(node::GetIsolate(env))
	{
	}
};
#endif

static V8ScriptRuntime* GetScriptRuntimeFromArgs(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto external = Local<External>::Cast(args.Data());

	return reinterpret_cast<V8ScriptRuntime*>(external->Value());
}

const OMPtr<V8ScriptRuntime>& V8ScriptRuntime::GetCurrent()
{
#if _DEBUG
	V8ScriptRuntime* v8Runtime;
	OMPtr<IScriptRuntime> runtime;

	assert(FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)));
	assert(v8Runtime = dynamic_cast<V8ScriptRuntime*>(runtime.GetRef()));

	assert(v8Runtime == g_currentV8Runtime.GetRef());
#endif

	return g_currentV8Runtime;
}

void ScriptTraceV(const char* string, fmt::printf_args formatList)
{
	auto t = fmt::vsprintf(string, formatList);
	console::Printf(fmt::sprintf("script:%s", V8ScriptRuntime::GetCurrent()->GetResourceName()), "%s", t);

	V8ScriptRuntime::GetCurrent()->GetScriptHost()->ScriptTrace(const_cast<char*>(t.c_str()));
}

template<typename... TArgs>
void ScriptTrace(const char* string, const TArgs& ... args)
{
	ScriptTraceV(string, fmt::make_printf_args(args...));
}

// blindly copypasted from StackOverflow (to allow std::function to store V8 UniquePersistent types with their move semantics)
template<class F>
struct shared_function
{
	std::shared_ptr<F> f;
	shared_function() = default;
	shared_function(F&& f_) : f(std::make_shared<F>(std::move(f_))) {}
	shared_function(shared_function const&) = default;
	shared_function(shared_function&&) = default;
	shared_function& operator=(shared_function const&) = default;
	shared_function& operator=(shared_function&&) = default;

	template<class...As>
	auto operator()(As&&...as) const
	{
		return (*f)(std::forward<As>(as)...);
	}
};

template<class F>
shared_function<std::decay_t<F>> make_shared_function(F&& f)
{
	return { std::forward<F>(f) };
}

enum class V8MetaFields
{
	PointerValueInt,
	PointerValueFloat,
	PointerValueVector,
	ReturnResultAnyway,
	ResultAsInteger,
	ResultAsLong,
	ResultAsFloat,
	ResultAsString,
	ResultAsVector,
	ResultAsObject,
	Max
};

static uint8_t g_metaFields[(int)V8MetaFields::Max];

static void V8_SetTickFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	Local<Function> tickFunction = Local<Function>::Cast(args[0]);
	UniquePersistent<Function> tickFunctionRef(GetV8Isolate(), tickFunction);

	runtime->SetTickRoutine(make_shared_function([runtime, tickFunctionRef{ std::move(tickFunctionRef) }] ()
	{
		Local<Function> tickFunction = tickFunctionRef.Get(GetV8Isolate());

		{
			TryCatch eh(GetV8Isolate());

			MaybeLocal<Value> value = tickFunction->Call(runtime->GetContext(), Null(GetV8Isolate()), 0, nullptr);

			if (value.IsEmpty())
			{
				String::Utf8Value str(GetV8Isolate(), eh.Exception());
				String::Utf8Value stack(GetV8Isolate(), eh.StackTrace(runtime->GetContext()).ToLocalChecked());

				ScriptTrace("Error calling system tick function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
			}
		}
	}));
}

static void V8_SetEventFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	Local<Function> function = Local<Function>::Cast(args[0]);
	UniquePersistent<Function> functionRef(GetV8Isolate(), function);

	runtime->SetEventRoutine(make_shared_function([runtime, functionRef{ std::move(functionRef) }](const char* eventName, const char* eventPayload, size_t payloadSize, const char* eventSource)
	{
		Local<Function> function = functionRef.Get(GetV8Isolate());

		{
			TryCatch eh(GetV8Isolate());

			Local<ArrayBuffer> inValueBuffer = ArrayBuffer::New(GetV8Isolate(), payloadSize);
			memcpy(inValueBuffer->GetContents().Data(), eventPayload, payloadSize);

			Local<Value> arguments[3];
			arguments[0] = String::NewFromUtf8(GetV8Isolate(), eventName).ToLocalChecked();
			arguments[1] = Uint8Array::New(inValueBuffer, 0, payloadSize);
			arguments[2] = String::NewFromUtf8(GetV8Isolate(), eventSource).ToLocalChecked();

			MaybeLocal<Value> value = function->Call(runtime->GetContext(), Null(GetV8Isolate()), 3, arguments);

			if (eh.HasCaught())
			{
				String::Utf8Value str(GetV8Isolate(), eh.Exception());
				String::Utf8Value stack(GetV8Isolate(), eh.StackTrace(runtime->GetContext()).ToLocalChecked());

				ScriptTrace("Error calling system event handling function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
			}
		}
	}));
}

static void V8_SetCallRefFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	Local<Function> function = Local<Function>::Cast(args[0]);
	UniquePersistent<Function> functionRef(GetV8Isolate(), function);

	runtime->SetCallRefRoutine(make_shared_function([runtime, functionRef{ std::move(functionRef) }](int32_t refId, const char* argsSerialized, size_t argsSize, char** retval, size_t* retvalLength)
	{
		// static array for retval output
		static std::vector<char> retvalArray(32768);

		Local<Function> function = functionRef.Get(GetV8Isolate());

		{
			TryCatch eh(GetV8Isolate());

			Local<ArrayBuffer> inValueBuffer = ArrayBuffer::New(GetV8Isolate(), argsSize);
			memcpy(inValueBuffer->GetContents().Data(), argsSerialized, argsSize);

			Local<Value> arguments[2];
			arguments[0] = Int32::New(GetV8Isolate(), refId);
			arguments[1] = Uint8Array::New(inValueBuffer, 0, argsSize);

			MaybeLocal<Value> maybeValue = function->Call(runtime->GetContext(), Null(GetV8Isolate()), 2, arguments);

			if (eh.HasCaught())
			{
				String::Utf8Value str(GetV8Isolate(), eh.Exception());
				String::Utf8Value stack(GetV8Isolate(), eh.StackTrace(runtime->GetContext()).ToLocalChecked());

				ScriptTrace("Error calling system call ref function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
			}
			else
			{
				Local<Value> value = maybeValue.ToLocalChecked();

				if (!value->IsArrayBufferView())
				{
					return;
				}

				Local<ArrayBufferView> abv = value.As<ArrayBufferView>();
				*retvalLength = abv->ByteLength();

				abv->CopyContents(retvalArray.data(), fwMin(retvalArray.size(), *retvalLength));
				*retval = retvalArray.data();
			}
		}
	}));
}

static void V8_SetDeleteRefFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	Local<Function> function = Local<Function>::Cast(args[0]);
	UniquePersistent<Function> functionRef(GetV8Isolate(), function);

	runtime->SetDeleteRefRoutine(make_shared_function([runtime, functionRef{ std::move(functionRef) }](int32_t refId)
	{
		Local<Function> function = functionRef.Get(GetV8Isolate());

		{
			TryCatch eh(GetV8Isolate());

			Local<Value> arguments[3];
			arguments[0] = Int32::New(GetV8Isolate(), refId);

			MaybeLocal<Value> value = function->Call(runtime->GetContext(), Null(GetV8Isolate()), 1, arguments);

			if (eh.HasCaught())
			{
				String::Utf8Value str(GetV8Isolate(), eh.Exception());
				String::Utf8Value stack(GetV8Isolate(), eh.StackTrace(runtime->GetContext()).ToLocalChecked());

				ScriptTrace("Error calling system delete ref function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
			}
		}
	}));
}

static void V8_SetDuplicateRefFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	Local<Function> function = Local<Function>::Cast(args[0]);
	UniquePersistent<Function> functionRef(GetV8Isolate(), function);

	runtime->SetDuplicateRefRoutine(make_shared_function([runtime, functionRef{ std::move(functionRef) }](int32_t refId)
	{
		Local<Function> function = functionRef.Get(GetV8Isolate());

		{
			TryCatch eh(GetV8Isolate());

			Local<Value> arguments[3];
			arguments[0] = Int32::New(GetV8Isolate(), refId);

			MaybeLocal<Value> value = function->Call(runtime->GetContext(), Null(GetV8Isolate()), 1, arguments);

			if (eh.HasCaught())
			{
				String::Utf8Value str(GetV8Isolate(), eh.Exception());
				String::Utf8Value stack(GetV8Isolate(), eh.StackTrace(runtime->GetContext()).ToLocalChecked());

				ScriptTrace("Error calling system duplicate ref function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
			}
			else
			{
				Local<Value> realValue;

				if (value.ToLocal(&realValue))
				{
					if (realValue->IsInt32())
					{
						return realValue->Int32Value(runtime->GetContext()).ToChecked();
					}
				}
			}

			return -1;
		}
	}));
}

static void V8_SetStackTraceRoutine(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	Local<Function> function = Local<Function>::Cast(args[0]);
	UniquePersistent<Function> functionRef(GetV8Isolate(), function);

	runtime->SetStackTraceRoutine(make_shared_function([runtime, functionRef{ std::move(functionRef) }](void* start, void* end, char** blob, size_t* size)
	{
		// static array for retval output (sadly)
		static std::vector<char> retvalArray(32768);

		Local<Function> function = functionRef.Get(GetV8Isolate());

		{
			TryCatch eh(GetV8Isolate());

			Local<Value> arguments[2];

			// push arguments on the stack
			if (start)
			{
				auto startRef = (V8Boundary*)start;
				arguments[0] = Int32::New(GetV8Isolate(), startRef->hint);
			}
			else
			{
				arguments[0] = Null(GetV8Isolate());
			}

			if (end)
			{
				auto endRef = (V8Boundary*)end;
				arguments[1] = Int32::New(GetV8Isolate(), endRef->hint);
			}
			else
			{
				arguments[1] = Null(GetV8Isolate());
			}

			MaybeLocal<Value> mv = function->Call(runtime->GetContext(), Null(GetV8Isolate()), 2, arguments);

			if (eh.HasCaught())
			{
				String::Utf8Value str(GetV8Isolate(), eh.Exception());
				String::Utf8Value stack(GetV8Isolate(), eh.StackTrace(runtime->GetContext()).ToLocalChecked());

				ScriptTrace("Error calling system stack trace function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
			}
			else
			{
				Local<Value> value = mv.ToLocalChecked();

				if (!value->IsArrayBufferView())
				{
					return;
				}

				Local<ArrayBufferView> abv = value.As<ArrayBufferView>();
				*size = abv->ByteLength();

				abv->CopyContents(retvalArray.data(), fwMin(retvalArray.size(), *size));
				*blob = retvalArray.data();
			}
		}
	}));
}

static void V8_CanonicalizeRef(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	char* refString;
	result_t hr = runtime->GetScriptHost()->CanonicalizeRef(args[0]->Int32Value(runtime->GetContext()).ToChecked(), runtime->GetInstanceId(), &refString);

	args.GetReturnValue().Set(String::NewFromUtf8(GetV8Isolate(), refString).ToLocalChecked());
	fwFree(refString);
}

struct RefAndPersistent {
	fx::FunctionRef ref;
	Persistent<Function> handle;
	fx::OMPtr<V8ScriptRuntime> runtime;
	fx::OMPtr<IScriptHost> host;
};

static tbb::concurrent_queue<RefAndPersistent*> g_cleanUpFuncRefs;

static void V8_InvokeFunctionReference(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto external = Local<External>::Cast(args.Data());
	auto refData = reinterpret_cast<RefAndPersistent*>(external->Value());

	OMPtr<IScriptHost> scriptHost = refData->runtime->GetScriptHost();

	Local<ArrayBufferView> abv = Local<ArrayBufferView>::Cast(args[0]);

	// variables to hold state
	fxNativeContext context = { 0 };

	context.numArguments = 4;
	context.nativeIdentifier = 0xe3551879; // INVOKE_FUNCTION_REFERENCE

	// identifier string
	context.arguments[0] = reinterpret_cast<uintptr_t>(refData->ref.GetRef().c_str());

	// argument data
	size_t argLength;
	std::vector<uint8_t> argsBuffer(abv->ByteLength());

	abv->CopyContents(argsBuffer.data(), argsBuffer.size());

	context.arguments[1] = reinterpret_cast<uintptr_t>(argsBuffer.data());
	context.arguments[2] = static_cast<uintptr_t>(argsBuffer.size());

	// return value length
	size_t retLength = 0;
	context.arguments[3] = reinterpret_cast<uintptr_t>(&retLength);

	// invoke
	scriptHost->InvokeNative(context);

	// get return values
	Local<ArrayBuffer> outValueBuffer = ArrayBuffer::New(GetV8Isolate(), retLength);
	memcpy(outValueBuffer->GetContents().Data(), (const void*)context.arguments[0], retLength);

	Local<Uint8Array> outArray = Uint8Array::New(outValueBuffer, 0, retLength);
	args.GetReturnValue().Set(outArray);
}

static void FnRefWeakCallback(
	const v8::WeakCallbackInfo<RefAndPersistent>& data)
{
	v8::HandleScope handleScope(data.GetIsolate());
	v8::Local<Function> v = data.GetParameter()->handle.Get(data.GetIsolate());

	data.GetParameter()->handle.Reset();

	// defer this to the next tick so that we won't end up deadlocking (the V8 lock is held at GC interrupt time, but the runtime lock is not)
	g_cleanUpFuncRefs.push(data.GetParameter());
}

static void V8_MakeFunctionReference(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	std::string refString;

	if (args[0]->IsString())
	{
		Local<String> string = Local<String>::Cast(args[0]);
		String::Utf8Value utf8{ GetV8Isolate(), string };
		refString = *utf8;
	}
	else if (args[0]->IsUint8Array())
	{
		Local<Uint8Array> arr = Local<Uint8Array>::Cast(args[0]);

		std::vector<uint8_t> data(arr->ByteLength());
		arr->CopyContents(data.data(), data.size());

		refString = std::string(reinterpret_cast<char*>(data.data()), data.size());
	}

	RefAndPersistent* data = new RefAndPersistent;
	data->ref = fx::FunctionRef{ refString };
	data->runtime = runtime;
	data->host = runtime->GetScriptHost();

	MaybeLocal<Function> outFunction = Function::New(runtime->GetContext(), V8_InvokeFunctionReference, External::New(GetV8Isolate(), data));

	Local<Function> outFn;
	
	if (outFunction.ToLocal(&outFn))
	{
		data->handle.Reset(GetV8Isolate(), outFn);
		data->handle.SetWeak(data, FnRefWeakCallback, v8::WeakCallbackType::kParameter);

		args.GetReturnValue().Set(outFn);
	}
	else
	{
		delete data;
	}
}

inline static std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static void V8_GetTickCount(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	args.GetReturnValue().Set((double)msec().count());
}

const char* V8ScriptRuntime::AssignStringValue(const Local<Value>& value)
{
	auto stringValue = std::make_unique<String::Utf8Value>(GetV8Isolate(), value);
	const char* str = **(stringValue.get());

	// take ownership
	m_stringValues[m_curStringValue] = std::move(stringValue);
	
	// increment the string value
	m_curStringValue = (m_curStringValue + 1) % _countof(m_stringValues);

	// return the string
	return str;
}

struct StringHashGetter
{
	static const int BaseArgs = 1;

	uint64_t operator()(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		String::Utf8Value hashString(GetV8Isolate(), args[0]);
		return strtoull(*hashString, nullptr, 16);
	}
};

struct IntHashGetter
{
	static const int BaseArgs = 2;

	uint64_t operator()(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		auto scrt = V8ScriptRuntime::GetCurrent();

		return (args[1]->Uint32Value(scrt->GetContext()).ToChecked() | (((uint64_t)args[0]->Uint32Value(scrt->GetContext()).ToChecked()) << 32));
	}
};

template<typename HashGetter>
static void V8_InvokeNative(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	// get required entries
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);
	OMPtr<IScriptHost> scriptHost = runtime->GetScriptHost();

	v8::Isolate::Scope isolateScope(GetV8Isolate());

	auto pointerFields = runtime->GetPointerFields();
	auto isolate = args.GetIsolate();

	// exception thrower
	auto throwException = [&](const std::string & exceptionString)
	{
		args.GetIsolate()->ThrowException(Exception::Error(String::NewFromUtf8(args.GetIsolate(), exceptionString.c_str()).ToLocalChecked()));
	};

	// variables to hold state
	fxNativeContext context = { 0 };

	// return values and their types
	int numReturnValues = 0;
	uintptr_t retvals[16] = { 0 };
	V8MetaFields rettypes[16];

	// coercion for the result value
	V8MetaFields returnValueCoercion = V8MetaFields::Max;

	// flag to return a result even if a pointer return value is passed
	bool returnResultAnyway = false;

	// get argument count for the loop
	int numArgs = args.Length();

	// verify argument count
	if (numArgs < 1)
	{
		return throwException("wrong argument count (needs at least a hash string)");
	}

	// get the hash
	uint64_t hash = HashGetter()(args);

	context.nativeIdentifier = hash;

	// pushing function
	auto push = [&](const auto & value)
	{
		*reinterpret_cast<uintptr_t*>(&context.arguments[context.numArguments]) = 0;
		*reinterpret_cast<std::decay_t<decltype(value)>*>(&context.arguments[context.numArguments]) = value;
		context.numArguments++;
	};

	auto cxt = runtime->GetContext();

	// the big argument loop
	for (int i = HashGetter::BaseArgs; i < numArgs; i++)
	{
		// get the type and decide what to do based on it
		auto arg = args[i];

		if (arg->IsNumber())
		{
			double value = arg->NumberValue(cxt).ToChecked();
			int64_t intValue = static_cast<int64_t>(value);

			if (intValue == value)
			{
				push(intValue);
			}
			else
			{
				push(static_cast<float>(value));
			}
		}
		else if (arg->IsBoolean() || arg->IsBooleanObject())
		{
			push(arg->BooleanValue(isolate));
		}
		else if (arg->IsString())
		{
			push(runtime->AssignStringValue(arg));
		}
		// null/undefined: add '0'
		else if (arg->IsNull() || arg->IsUndefined())
		{
			push(0);
		}
		// metafield
		else if (arg->IsExternal())
		{
			auto pushPtr = [&](V8MetaFields metaField)
			{
				if (numReturnValues >= _countof(retvals))
				{
					throwException("too many return value arguments");
					return false;
				}

				// push the offset and set the type
				push(&retvals[numReturnValues]);
				rettypes[numReturnValues] = metaField;

				// increment the counter
				if (metaField == V8MetaFields::PointerValueVector)
				{
					numReturnValues += 3;
				}
				else
				{
					numReturnValues += 1;
				}

				return true;
			};

			uint8_t* ptr = reinterpret_cast<uint8_t*>(Local<External>::Cast(arg)->Value());

			// if the pointer is a metafield
			if (ptr >= g_metaFields && ptr < &g_metaFields[(int)V8MetaFields::Max])
			{
				V8MetaFields metaField = static_cast<V8MetaFields>(ptr - g_metaFields);

				// switch on the metafield
				switch (metaField)
				{
				case V8MetaFields::PointerValueInt:
				case V8MetaFields::PointerValueFloat:
				case V8MetaFields::PointerValueVector:
				{
					if (!pushPtr(metaField))
					{
						return;
					}

					break;
				}
				case V8MetaFields::ReturnResultAnyway:
					returnResultAnyway = true;
					break;
				case V8MetaFields::ResultAsInteger:
				case V8MetaFields::ResultAsLong:
				case V8MetaFields::ResultAsString:
				case V8MetaFields::ResultAsFloat:
				case V8MetaFields::ResultAsVector:
				case V8MetaFields::ResultAsObject:
					returnValueCoercion = metaField;
					break;
				}
			}
			// or if the pointer is a runtime pointer field
			else if (ptr >= reinterpret_cast<uint8_t*>(pointerFields) && ptr < (reinterpret_cast<uint8_t*>(pointerFields) + (sizeof(PointerField) * 2)))
			{
				// guess the type based on the pointer field type
				intptr_t ptrField = ptr - reinterpret_cast<uint8_t*>(pointerFields);
				V8MetaFields metaField = static_cast<V8MetaFields>(ptrField / sizeof(PointerField));

				if (metaField == V8MetaFields::PointerValueInt || metaField == V8MetaFields::PointerValueFloat)
				{
					auto ptrFieldEntry = reinterpret_cast<PointerFieldEntry*>(ptr);

					retvals[numReturnValues] = ptrFieldEntry->value;
					ptrFieldEntry->empty = true;

					if (!pushPtr(metaField))
					{
						return;
					}
				}
			}
			else
			{
				push(ptr);
			}
		}
		// placeholder vectors
		else if (arg->IsArray())
		{
			Local<Array> array = Local<Array>::Cast(arg);
			float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

			auto getNumber = [&](int idx)
			{
				Local<Value> value;

				if (!array->Get(cxt, idx).ToLocal(&value))
				{
					return NAN;
				}

				if (value.IsEmpty() || !value->IsNumber())
				{
					return NAN;
				}

				return static_cast<float>(value->NumberValue(cxt).ToChecked());
			};

			if (array->Length() < 2 || array->Length() > 4)
			{
				return throwException("arrays should be vectors (wrong number of values)");
			}

			if (array->Length() >= 2)
			{
				x = getNumber(0);
				y = getNumber(1);

				if (x == NAN || y == NAN)
				{
					return throwException("invalid vector array value");
				}

				push(x);
				push(y);
			}

			if (array->Length() >= 3)
			{
				z = getNumber(2);

				if (z == NAN)
				{
					return throwException("invalid vector array value");
				}

				push(z);
			}

			if (array->Length() >= 4)
			{
				w = getNumber(3);

				if (w == NAN)
				{
					return throwException("invalid vector array value");
				}

				push(w);
			}
		}
		else if (arg->IsArrayBufferView())
		{
			Local<ArrayBufferView> abv = arg.As<ArrayBufferView>();
			Local<ArrayBuffer> buffer = abv->Buffer();

			push((char*)buffer->GetContents().Data() + abv->ByteOffset());
		}
		// this should be the last entry, I'd guess
		else if (arg->IsObject())
		{
			Local<Object> object = arg->ToObject(cxt).ToLocalChecked();
			Local<Value> data;
			
			if (!object->Get(cxt, String::NewFromUtf8(GetV8Isolate(), "__data").ToLocalChecked()).ToLocal(&data))
			{
				return throwException("__data field does not contain a number");
			}

			if (!data.IsEmpty() && data->IsNumber())
			{
				auto n = data->ToNumber(runtime->GetContext());
				v8::Local<v8::Number> number;

				if (n.ToLocal(&number))
				{
					push(number->Int32Value(cxt).ToChecked());
				}
			}
			else
			{
				return throwException("__data field does not contain a number");
			}
		}
		else
		{
			String::Utf8Value str(GetV8Isolate(), arg);

			return throwException(va("Invalid V8 value: %s", *str));
		}
	}

	// invoke the native on the script host
	if (!FX_SUCCEEDED(scriptHost->InvokeNative(context)))
	{
		return throwException(va("Execution of native %016llx in script host failed.", hash));;
	}

	// padded vector struct
	struct scrVector
	{
		float x;

	private:
		uint32_t pad0;

	public:
		float y;

	private:
		uint32_t pad1;

	public:
		float z;

	private:
		uint32_t pad2;
	};

	struct scrObject
	{
		const char* data;
		uintptr_t length;
	};

	// number of Lua results
	Local<Value> returnValue = Undefined(args.GetIsolate());
	int numResults = 0;

	// if no other result was requested, or we need to return the result anyway, push the result
	if (numReturnValues == 0 || returnResultAnyway)
	{
		// increment the result count
		numResults++;

		// handle the type coercion
		switch (returnValueCoercion)
		{
		case V8MetaFields::ResultAsString:
		{
			const char* str = *reinterpret_cast<const char**>(&context.arguments[0]);

			if (str)
			{
				returnValue = String::NewFromUtf8(args.GetIsolate(), str).ToLocalChecked();
			}
			else
			{
				returnValue = Null(args.GetIsolate());
			}

			break;
		}
		case V8MetaFields::ResultAsFloat:
			returnValue = Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&context.arguments[0]));
			break;
		case V8MetaFields::ResultAsVector:
		{
			scrVector vector = *reinterpret_cast<scrVector*>(&context.arguments[0]);

			Local<Array> vectorArray = Array::New(args.GetIsolate(), 3);
			vectorArray->Set(cxt, 0, Number::New(args.GetIsolate(), vector.x));
			vectorArray->Set(cxt, 1, Number::New(args.GetIsolate(), vector.y));
			vectorArray->Set(cxt, 2, Number::New(args.GetIsolate(), vector.z));

			returnValue = vectorArray;

			break;
		}
		case V8MetaFields::ResultAsObject:
		{
			scrObject object = *reinterpret_cast<scrObject*>(&context.arguments[0]);

			Local<ArrayBuffer> outValueBuffer = ArrayBuffer::New(GetV8Isolate(), object.length);
			memcpy(outValueBuffer->GetContents().Data(), object.data, object.length);

			Local<Uint8Array> outArray = Uint8Array::New(outValueBuffer, 0, object.length);

			returnValue = outArray;

			break;
		}
		case V8MetaFields::ResultAsInteger:
			returnValue = Int32::New(args.GetIsolate(), *reinterpret_cast<int32_t*>(&context.arguments[0]));
			break;
		case V8MetaFields::ResultAsLong:
			returnValue = Number::New(args.GetIsolate(), double(*reinterpret_cast<int64_t*>(&context.arguments[0])));
			break;
		default:
		{
			int32_t integer = *reinterpret_cast<int32_t*>(&context.arguments[0]);

			if ((integer & 0xFFFFFFFF) == 0)
			{
				returnValue = Boolean::New(args.GetIsolate(), false);
			}
			else
			{
				returnValue = Int32::New(args.GetIsolate(), integer);
			}
		}
		}
	}

	// loop over the return value pointers
	{
		int i = 0;

		// fast path non-array result
		if (numReturnValues == 1 && numResults == 0)
		{
			switch (rettypes[0])
			{
			case V8MetaFields::PointerValueInt:
				returnValue = Int32::New(args.GetIsolate(), retvals[0]);
				break;

			case V8MetaFields::PointerValueFloat:
				returnValue = Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&retvals[0]));
				break;

			case V8MetaFields::PointerValueVector:
			{
				scrVector vector = *reinterpret_cast<scrVector*>(&retvals[0]);

				Local<Array> vectorArray = Array::New(args.GetIsolate(), 3);
				vectorArray->Set(cxt, 0, Number::New(args.GetIsolate(), vector.x));
				vectorArray->Set(cxt, 1, Number::New(args.GetIsolate(), vector.y));
				vectorArray->Set(cxt, 2, Number::New(args.GetIsolate(), vector.z));

				returnValue = vectorArray;
				break;
			}
			}
		}
		else if (numReturnValues > 0)
		{
			Local<Object> arrayValue = Array::New(args.GetIsolate());

			// transform into array
			{
				Local<Value> oldValue = returnValue;

				returnValue = arrayValue;
				arrayValue->Set(cxt, 0, oldValue);
			}

			while (i < numReturnValues)
			{
				switch (rettypes[i])
				{
				case V8MetaFields::PointerValueInt:
					arrayValue->Set(cxt, numResults, Int32::New(args.GetIsolate(), retvals[i]));
					i++;
					break;

				case V8MetaFields::PointerValueFloat:
					arrayValue->Set(cxt, numResults, Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&retvals[i])));
					i++;
					break;

				case V8MetaFields::PointerValueVector:
				{
					scrVector vector = *reinterpret_cast<scrVector*>(&retvals[i]);

					Local<Array> vectorArray = Array::New(args.GetIsolate(), 3);
					vectorArray->Set(cxt, 0, Number::New(args.GetIsolate(), vector.x));
					vectorArray->Set(cxt, 1, Number::New(args.GetIsolate(), vector.y));
					vectorArray->Set(cxt, 2, Number::New(args.GetIsolate(), vector.z));

					arrayValue->Set(cxt, numResults, vectorArray);

					i += 3;
					break;
				}
				}

				numResults++;
			}
		}
	}

	// and set the return value(s)
	args.GetReturnValue().Set(returnValue);
}

template<V8MetaFields MetaField>
static void V8_GetMetaField(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	args.GetReturnValue().Set(External::New(GetV8Isolate(), &g_metaFields[(int)MetaField]));
}

template<V8MetaFields MetaField>
static void V8_GetPointerField(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	auto pointerFields = runtime->GetPointerFields();
	auto pointerFieldStart = &pointerFields[(int)MetaField];

	static uintptr_t dummyOut;
	PointerFieldEntry* pointerField = nullptr;
	
	for (int i = 0; i < _countof(pointerFieldStart->data); i++)
	{
		if (pointerFieldStart->data[i].empty)
		{
			pointerField = &pointerFieldStart->data[i];
			pointerField->empty = false;
			
			auto arg = args[0];

			if (MetaField == V8MetaFields::PointerValueFloat)
			{
				float value = static_cast<float>(arg->NumberValue(runtime->GetContext()).ToChecked());

				pointerField->value = *reinterpret_cast<uint32_t*>(&value);
			}
			else if (MetaField == V8MetaFields::PointerValueInt)
			{
				intptr_t value = arg->IntegerValue(runtime->GetContext()).ToChecked();

				pointerField->value = value;
			}

			break;
		}
	}

	args.GetReturnValue().Set(External::New(GetV8Isolate(), (pointerField) ? static_cast<void*>(pointerField) : &dummyOut));
}

std::string SaveProfileToString(CpuProfile* profile);

static void V8_StartProfiling(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	/*CpuProfiler* profiler = args.GetIsolate()->GetCpuProfiler();

	profiler->StartProfiling((args.Length() == 0) ? String::Empty(args.GetIsolate()) : Local<String>::Cast(args[0]), true);*/
}

static void V8_StopProfiling(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	/*CpuProfiler* profiler = args.GetIsolate()->GetCpuProfiler();

	CpuProfile* profile = profiler->StopProfiling((args.Length() == 0) ? String::Empty(args.GetIsolate()) : Local<String>::Cast(args[0]));

	std::string jsonString = SaveProfileToString(profile);

	profile->Delete();

#ifdef _WIN32
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);

	{
		std::ofstream stream(MakeRelativeCitPath(va(L"v8-%04d%02d%02d-%02d%02d%02d.cpuprofile", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond)));
		stream << jsonString;
	}
#endif

	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);
	args.GetReturnValue().Set(JSON::Parse(runtime->GetContext(), String::NewFromUtf8(args.GetIsolate(), jsonString.c_str(), NewStringType::kNormal, jsonString.size()).ToLocalChecked()).ToLocalChecked());*/
}

static void V8_Trace(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	bool first = true;

	for (int i = 0; i < args.Length(); i++)
	{
		Locker locker(args.GetIsolate());
		Isolate::Scope isolateScope(args.GetIsolate());
		v8::HandleScope handle_scope(args.GetIsolate());

		if (first)
		{
			first = false;
		}
		else
		{
			printf(" ");
		}

		v8::String::Utf8Value str(GetV8Isolate(), args[i]);
		ScriptTrace("%s", *str);
	}

	ScriptTrace("\n");
}

static void V8_GetResourcePath(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);
	auto path = ((fx::Resource*)runtime->GetParentObject())->GetPath();

	args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), path.c_str(), NewStringType::kNormal, path.size()).ToLocalChecked());
}

static void V8_SubmitBoundaryStart(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	// get required entries
	auto scrt = GetScriptRuntimeFromArgs(args);
	auto scriptHost = scrt->GetScriptHost();

	auto val = args[0]->IntegerValue(scrt->GetContext());

	V8Boundary b;
	b.hint = val.ToChecked();

	scriptHost->SubmitBoundaryStart((char*)& b, sizeof(b));
}

static void V8_SubmitBoundaryEnd(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	// get required entries
	auto scrt = GetScriptRuntimeFromArgs(args);
	auto scriptHost = scrt->GetScriptHost();

	auto val = args[0]->IntegerValue(scrt->GetContext());

	V8Boundary b;
	b.hint = val.ToChecked();

	scriptHost->SubmitBoundaryEnd((char*)&b, sizeof(b));
}

static std::pair<std::string, FunctionCallback> g_citizenFunctions[] =
{
	{ "trace", V8_Trace },
	{ "setTickFunction", V8_SetTickFunction },
	{ "setEventFunction", V8_SetEventFunction },
	// ref stuff
	{ "setCallRefFunction", V8_SetCallRefFunction },
	{ "setDeleteRefFunction", V8_SetDeleteRefFunction },
	{ "setDuplicateRefFunction", V8_SetDuplicateRefFunction },
	{ "canonicalizeRef", V8_CanonicalizeRef },
	{ "makeFunctionReference", V8_MakeFunctionReference },
	// internals
	{ "getTickCount", V8_GetTickCount },
	{ "invokeNative", V8_InvokeNative<StringHashGetter> },
	{ "invokeNativeByHash", V8_InvokeNative<IntHashGetter> },
	{ "startProfiling", V8_StartProfiling },
	{ "stopProfiling", V8_StopProfiling },
	// boundary
	{ "submitBoundaryStart", V8_SubmitBoundaryStart },
	{ "submitBoundaryEnd", V8_SubmitBoundaryEnd },
	{ "setStackTraceFunction", V8_SetStackTraceRoutine },
	// metafields
	{ "pointerValueIntInitialized", V8_GetPointerField<V8MetaFields::PointerValueInt> },
	{ "pointerValueFloatInitialized", V8_GetPointerField<V8MetaFields::PointerValueFloat> },
	{ "pointerValueInt", V8_GetMetaField<V8MetaFields::PointerValueInt> },
	{ "pointerValueFloat", V8_GetMetaField<V8MetaFields::PointerValueFloat> },
	{ "pointerValueVector", V8_GetMetaField<V8MetaFields::PointerValueVector> },
	{ "returnResultAnyway", V8_GetMetaField<V8MetaFields::ReturnResultAnyway> },
	{ "resultAsInteger", V8_GetMetaField<V8MetaFields::ResultAsInteger> },
	{ "resultAsLong", V8_GetMetaField<V8MetaFields::ResultAsLong> },
	{ "resultAsFloat", V8_GetMetaField<V8MetaFields::ResultAsFloat> },
	{ "resultAsString", V8_GetMetaField<V8MetaFields::ResultAsString> },
	{ "resultAsVector", V8_GetMetaField<V8MetaFields::ResultAsVector> },
	{ "resultAsObject", V8_GetMetaField<V8MetaFields::ResultAsObject> },
#ifdef IS_FXSERVER
	{ "getResourcePath", V8_GetResourcePath },
#endif
};

static v8::Handle<v8::Value> Throw(v8::Isolate* isolate, const char* message) {
	return isolate->ThrowException(v8::String::NewFromUtf8(isolate, message).ToLocalChecked());
}

static bool ReadFileData(const v8::FunctionCallbackInfo<v8::Value>& args, std::vector<char>* fileData)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);
	OMPtr<IScriptHost> scriptHost = runtime->GetScriptHost();

	V8PushEnvironment pushed(runtime);
	String::Utf8Value filename(GetV8Isolate(), args[0]);

	OMPtr<fxIStream> stream;
	result_t hr = scriptHost->OpenHostFile(*filename, stream.GetAddressOf());

	if (FX_FAILED(hr))
	{
		Throw(args.GetIsolate(), "Error loading file");
		return false;
	}

	uint64_t length;
	stream->GetLength(&length);

	uint32_t bytesRead;
	fileData->resize(length);
	stream->Read(fileData->data(), length, &bytesRead);

	return true;
}

static void V8_Read(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::vector<char> fileData;

	if (!ReadFileData(args, &fileData))
	{
		return;
	}

	Handle<String> str = String::NewFromUtf8(args.GetIsolate(), fileData.data(), NewStringType::kNormal, fileData.size()).ToLocalChecked();
	args.GetReturnValue().Set(str);
}

struct DataAndPersistent {
	std::vector<char> data;
	Persistent<ArrayBuffer> handle;
};

static void ReadBufferWeakCallback(
	const v8::WeakCallbackInfo<DataAndPersistent>& data)
{
	v8::HandleScope handleScope(data.GetIsolate());
	v8::Local<ArrayBuffer> v = data.GetParameter()->handle.Get(data.GetIsolate());

	size_t byte_length = v->ByteLength();
	data.GetIsolate()->AdjustAmountOfExternalAllocatedMemory(
		-static_cast<intptr_t>(byte_length));
	data.GetParameter()->handle.Reset();
	delete data.GetParameter();
}

static void V8_ReadBuffer(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	std::vector<char> fileData;

	if (!ReadFileData(args, &fileData))
	{
		return;
	}

	auto isolate = args.GetIsolate();

	DataAndPersistent* data = new DataAndPersistent;
	data->data = std::move(fileData);

	Handle<v8::ArrayBuffer> buffer =
		ArrayBuffer::New(isolate, data->data.data(), data->data.size());

	data->handle.Reset(isolate, buffer);
	data->handle.SetWeak(data, ReadBufferWeakCallback, v8::WeakCallbackType::kParameter);

	isolate->AdjustAmountOfExternalAllocatedMemory(data->data.size());
	args.GetReturnValue().Set(buffer);
}

static std::pair<std::string, FunctionCallback> g_globalFunctions[] =
{
	{ "read", V8_Read },
	{ "readbuffer", V8_ReadBuffer },
};

result_t V8ScriptRuntime::Create(IScriptHost* scriptHost)
{
	// assign the script host
	m_scriptHost = scriptHost;

	{
		fx::OMPtr<IScriptHost> ptr(scriptHost);
		fx::OMPtr<IScriptHostWithResourceData> resourcePtr;
		ptr.As(&resourcePtr);

		m_resourceHost = resourcePtr.GetRef();

		fx::OMPtr<IScriptHostWithManifest> manifestPtr;
		ptr.As(&manifestPtr);

		m_manifestHost = manifestPtr.GetRef();
	}

	// create a scope to hold handles we use here
	Locker locker(GetV8Isolate());
	Isolate::Scope isolateScope(GetV8Isolate());
	HandleScope handleScope(GetV8Isolate());

	// create global state
	Local<ObjectTemplate> global = ObjectTemplate::New(GetV8Isolate());

	// add a 'print' function as alias for Citizen.trace for testing
	global->Set(String::NewFromUtf8(GetV8Isolate(), "print", NewStringType::kNormal).ToLocalChecked(),
				FunctionTemplate::New(GetV8Isolate(), V8_Trace));

	// create Citizen call routines
	Local<ObjectTemplate> citizenObject = ObjectTemplate::New(GetV8Isolate());

	// loop through routine list
	for (auto& routine : g_citizenFunctions)
	{
		citizenObject->Set(GetV8Isolate(), routine.first.c_str(), FunctionTemplate::New(GetV8Isolate(), routine.second, External::New(GetV8Isolate(), this)));
	}

	// set the Citizen object
	global->Set(String::NewFromUtf8(GetV8Isolate(), "Citizen", NewStringType::kNormal).ToLocalChecked(),
				citizenObject);

	// create a V8 context and store it
	Local<Context> context = Context::New(GetV8Isolate(), nullptr, global);
	m_context.Reset(GetV8Isolate(), context);

	// run the following entries in the context scope
	Context::Scope scope(context);

#ifdef IS_FXSERVER
#ifdef _WIN32
	std::string selfPath = ToNarrow(MakeRelativeCitPath(_P("FXServer.exe")));
#else
	std::string selfPath = MakeRelativeCitPath(_P("FXServer"));
#endif

	std::string rootPath = selfPath;
	boost::algorithm::replace_all(rootPath, "/opt/cfx-server/FXServer", "");

	auto libPath = fmt::sprintf("%s/usr/lib/v8/:%s/lib/:%s/usr/lib/",
		rootPath,
		rootPath,
		rootPath);

	const char* execArgv[] = {
#ifndef _WIN32
		"--library-path",
		libPath.c_str(),
		"--",
		selfPath.c_str(),
#endif
		"--start-node",
	};

	node::InitializeContext(context);

	auto env = node::CreateEnvironment(GetNodeIsolate(), context, 0, nullptr, std::size(execArgv), execArgv);
	node::LoadEnvironment(env);

	g_envRuntimes[env] = this;

	m_nodeEnvironment = env;
#endif

	// set global IO functions
	for (auto& routine : g_globalFunctions)
	{
		context->Global()->Set(
			context,
			String::NewFromUtf8(GetV8Isolate(), routine.first.c_str(), NewStringType::kNormal).ToLocalChecked(),
			Function::New(context, routine.second, External::New(GetV8Isolate(), this)).ToLocalChecked());
	}

	// set the 'window' variable to the global itself
	context->Global()->Set(context, String::NewFromUtf8(GetV8Isolate(), "window", NewStringType::kNormal).ToLocalChecked(), context->Global());

	std::string nativesBuild = "natives_universal.js";

	{
		for (const auto& versionPair : g_scriptVersionPairs)
		{
			bool isGreater;

			if (FX_SUCCEEDED(m_manifestHost->IsManifestVersionBetween(std::get<ManifestVersion>(versionPair).guid, guid_t{ 0 }, &isGreater)) && isGreater)
			{
				nativesBuild = std::get<const char*>(versionPair);
			}
		}
	}

	// run system scripts
	result_t hr;

	// Loading natives
	if (FX_FAILED(hr = LoadSystemFile(const_cast<char*>(va("citizen:/scripting/v8/%s", nativesBuild)))))
	{
		return hr;
	}

	for (const char* platformScript : g_platformScripts)
	{
		if (FX_FAILED(hr = LoadSystemFile(const_cast<char*>(platformScript))))
		{
			return hr;
		}
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::Destroy()
{
	m_eventRoutine = TEventRoutine();
	m_tickRoutine = std::function<void()>();
	m_callRefRoutine = TCallRefRoutine();
	m_deleteRefRoutine = TDeleteRefRoutine();
	m_duplicateRefRoutine = TDuplicateRefRoutine();

#ifdef IS_FXSERVER
	g_envRuntimes.erase(m_nodeEnvironment);

	node::FreeEnvironment(m_nodeEnvironment);
#endif

	fx::PushEnvironment pushed(this);
	m_context.Reset();

	return FX_S_OK;
}

void* V8ScriptRuntime::GetParentObject()
{
	return m_parentObject;
}

void V8ScriptRuntime::SetParentObject(void* parentObject)
{
	m_parentObject = parentObject;
}

result_t V8ScriptRuntime::LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile, Local<Script>* outScript)
{
	// read file data
	uint64_t length;
	result_t hr;

	if (FX_FAILED(hr = stream->GetLength(&length)))
	{
		return hr;
	}

	std::vector<char> fileData(length + 1);
	if (FX_FAILED(hr = stream->Read(&fileData[0], length, nullptr)))
	{
		return hr;
	}

	fileData[length] = '\0';

	// create the script data
	Local<String> scriptText = String::NewFromUtf8(GetV8Isolate(), &fileData[0], NewStringType::kNormal).ToLocalChecked();
	Local<String> fileName = String::NewFromUtf8(GetV8Isolate(), scriptFile, NewStringType::kNormal).ToLocalChecked();

	// compile the script in a TryCatch
	{
		TryCatch eh(GetV8Isolate());
		v8::ScriptOrigin origin(fileName);
		MaybeLocal<Script> script = Script::Compile(GetContext(), scriptText, &origin);

		if (script.IsEmpty())
		{
			String::Utf8Value str(GetV8Isolate(), eh.Exception());

			ScriptTrace("Error parsing script %s in resource %s: %s\n", scriptFile, GetResourceName(), *str);

			// TODO: change?
			return FX_E_INVALIDARG;
		}

		*outScript = script.ToLocalChecked();
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::LoadHostFileInternal(char* scriptFile, Local<Script>* outScript)
{
	// open the file
	OMPtr<fxIStream> stream;

	result_t hr = m_scriptHost->OpenHostFile(scriptFile, stream.GetAddressOf());

	if (FX_FAILED(hr))
	{
		// TODO: log this?
		return hr;
	}

	char* resourceName;
	m_resourceHost->GetResourceName(&resourceName);

	return LoadFileInternal(stream, (scriptFile[0] != '@') ? const_cast<char*>(fmt::sprintf("@%s/%s", resourceName, scriptFile).c_str()) : scriptFile, outScript);
}

result_t V8ScriptRuntime::LoadSystemFileInternal(char* scriptFile, Local<Script>* outScript)
{
	// open the file
	OMPtr<fxIStream> stream;

	result_t hr = m_scriptHost->OpenSystemFile(scriptFile, stream.GetAddressOf());

	if (FX_FAILED(hr))
	{
		// TODO: log this?
		return hr;
	}

	return LoadFileInternal(stream, scriptFile, outScript);
}

result_t V8ScriptRuntime::RunFileInternal(char* scriptName, std::function<result_t(char*, Local<Script>*)> loadFunction)
{
	V8PushEnvironment pushed(this);

	result_t hr;
	Local<Script> script;

	if (FX_FAILED(hr = loadFunction(scriptName, &script)))
	{
		return hr;
	}

	{
		TryCatch eh(GetV8Isolate());
		MaybeLocal<Value> value = script->Run(GetContext());

		if (value.IsEmpty())
		{
			String::Utf8Value str(GetV8Isolate(), eh.Exception());
			String::Utf8Value stack(GetV8Isolate(), eh.StackTrace(GetContext()).ToLocalChecked());

			ScriptTrace("Error loading script %s in resource %s: %s\nstack:\n%s\n", scriptName, GetResourceName(), *str, *stack);

			// TODO: change?
			return FX_E_INVALIDARG;
		}
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::LoadFile(char* scriptName)
{
	return RunFileInternal(scriptName, std::bind(&V8ScriptRuntime::LoadHostFileInternal, this, std::placeholders::_1, std::placeholders::_2));
}

result_t V8ScriptRuntime::LoadSystemFile(char* scriptName)
{
	return RunFileInternal(scriptName, std::bind(&V8ScriptRuntime::LoadSystemFileInternal, this, std::placeholders::_1, std::placeholders::_2));
}

int V8ScriptRuntime::GetInstanceId()
{
	return m_instanceId;
}

int32_t V8ScriptRuntime::HandlesFile(char* fileName)
{
	return strstr(fileName, ".js") != 0;
}

result_t V8ScriptRuntime::Tick()
{
	if (m_tickRoutine)
	{
		V8PushEnvironment pushed(this);
#ifndef IS_FXSERVER
		v8::platform::PumpMessageLoop(GetV8Platform(), GetV8Isolate());
#endif

		m_tickRoutine();
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::TriggerEvent(char* eventName, char* eventPayload, uint32_t payloadSize, char* eventSource)
{
	if (m_eventRoutine)
	{
		V8PushEnvironment pushed(this);

		m_eventRoutine(eventName, eventPayload, payloadSize, eventSource);
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::CallRef(int32_t refIdx, char* argsSerialized, uint32_t argsLength, char** retvalSerialized, uint32_t* retvalLength)
{
	*retvalLength = 0;
	*retvalSerialized = nullptr;

	if (m_callRefRoutine)
	{
		V8PushEnvironment pushed(this);

		size_t retvalLengthS = 0;
		m_callRefRoutine(refIdx, argsSerialized, argsLength, retvalSerialized, &retvalLengthS);

		*retvalLength = retvalLengthS;
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::DuplicateRef(int32_t refIdx, int32_t* outRefIdx)
{
	*outRefIdx = -1;

	if (m_duplicateRefRoutine)
	{
		V8PushEnvironment pushed(this);

		*outRefIdx = m_duplicateRefRoutine(refIdx);
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::RemoveRef(int32_t refIdx)
{
	if (m_deleteRefRoutine)
	{
		V8PushEnvironment pushed(this);

		m_deleteRefRoutine(refIdx);
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::WalkStack(char* boundaryStart, uint32_t boundaryStartLength, char* boundaryEnd, uint32_t boundaryEndLength, IScriptStackWalkVisitor* visitor)
{
	if (m_stackTraceRoutine)
	{
		V8PushEnvironment pushed(this);

		char* out = nullptr;
		size_t outLen = 0;

		m_stackTraceRoutine(boundaryStart, boundaryEnd, &out, &outLen);

		if (out)
		{
			msgpack::unpacked up = msgpack::unpack(out, outLen);

			auto o = up.get().as<std::vector<msgpack::object>>();

			for (auto& e : o)
			{
				msgpack::sbuffer sb;
				msgpack::pack(sb, e);

				visitor->SubmitStackFrame(sb.data(), sb.size());
			}
		}
	}

	return FX_S_OK;
}

// from the V8 example
class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
public:
	virtual void* Allocate(size_t length) override
	{
		void* data = AllocateUninitialized(length);
		return data == NULL ? data : memset(data, 0, length);
	}
	virtual void* AllocateUninitialized(size_t length) override { return malloc(length); }
	virtual void Free(void* data, size_t) override { free(data); }
};

class V8ScriptGlobals
{
private:
	Isolate* m_isolate;

#ifdef IS_FXSERVER
	node::IsolateData* m_nodeData;
#endif

	std::vector<char> m_nativesBlob;

	std::vector<char> m_snapshotBlob;

	std::unique_ptr<Platform> m_platform;

	std::unique_ptr<v8::ArrayBuffer::Allocator> m_arrayBufferAllocator;

	std::unique_ptr<V8Debugger> m_debugger;

public:
	V8ScriptGlobals();

	~V8ScriptGlobals();

	void Initialize();

	inline Platform* GetPlatform()
	{
		return m_platform.get();
	}

	inline Isolate* GetIsolate()
	{
		return m_isolate;
	}

#ifdef IS_FXSERVER
	inline node::IsolateData* GetNodeIsolate()
	{
		return m_nodeData;
	}
#endif
};

static V8ScriptGlobals g_v8;

Isolate* GetV8Isolate()
{
	return g_v8.GetIsolate();
}

static Platform* GetV8Platform()
{
	return g_v8.GetPlatform();
}

#ifdef IS_FXSERVER
static node::IsolateData* GetNodeIsolate()
{
	return g_v8.GetNodeIsolate();
}
#endif

static void OnMessage(Local<Message> message, Local<Value> error)
{
	v8::String::Utf8Value messageStr(GetV8Isolate(), message->Get());
	v8::String::Utf8Value errorStr(GetV8Isolate(), error);

	std::stringstream stack;
	auto stackTrace = message->GetStackTrace();

	for (int i = 0; i < stackTrace->GetFrameCount(); i++)
	{
		auto frame = stackTrace->GetFrame(GetV8Isolate(), i);

		v8::String::Utf8Value sourceStr(GetV8Isolate(), frame->GetScriptNameOrSourceURL());
		v8::String::Utf8Value functionStr(GetV8Isolate(), frame->GetFunctionName());
		
		stack << *sourceStr << "(" << frame->GetLineNumber() << "," << frame->GetColumn() << "): " << (*functionStr ? *functionStr : "") << "\n";
	}

	ScriptTrace("%s\n%s\n%s\n", *messageStr, stack.str(), *errorStr);
}

V8ScriptGlobals::V8ScriptGlobals()
{
}

void V8ScriptGlobals::Initialize()
{
#ifdef _WIN32
	// initialize startup data
	auto readBlob = [=] (const std::wstring& name, std::vector<char>& outBlob)
	{
		FILE* f = _pfopen(MakeRelativeCitPath(_P("citizen/scripting/v8/" + name)).c_str(), _P("rb"));
		assert(f);

		fseek(f, 0, SEEK_END);
		outBlob.resize(ftell(f));

		fseek(f, 0, SEEK_SET);
		fread(outBlob.data(), 1, outBlob.size(), f);

		fclose(f);
	};

	readBlob(L"natives_blob.bin", m_nativesBlob);
	readBlob(L"snapshot_blob.bin", m_snapshotBlob);

	static StartupData nativesBlob;
	nativesBlob.data = &m_nativesBlob[0];
	nativesBlob.raw_size = m_nativesBlob.size();

	static StartupData snapshotBlob;
	snapshotBlob.data = &m_snapshotBlob[0];
	snapshotBlob.raw_size = m_snapshotBlob.size();
	
	V8::SetNativesDataBlob(&nativesBlob);
	V8::SetSnapshotDataBlob(&snapshotBlob);
#endif

#ifdef IS_FXSERVER
	int eac;
	const char** eav;

	if (g_argc >= 2 && strcmp(g_argv[1], "--start-node") == 0)
	{
		int ec = 0;
		
		// run in a thread so that pthread attributes take effect on musl-based Linux
		// (GNU stack size presets do not seem to work here)
		std::thread([&ec]
		{
			// TODO: code duplication with above
#ifdef _WIN32
			std::string selfPath = ToNarrow(MakeRelativeCitPath(_P("FXServer.exe")));
#else
			std::string selfPath = MakeRelativeCitPath(_P("FXServer"));
#endif

			std::string rootPath = selfPath;
			boost::algorithm::replace_all(rootPath, "/opt/cfx-server/FXServer", "");

			auto libPath = fmt::sprintf("%s/usr/lib/v8/:%s/lib/:%s/usr/lib/",
				rootPath,
				rootPath,
				rootPath);

#ifndef _WIN32
			const char* execArgv[] = {
				"--library-path",
				libPath.c_str(),
				"--",
				selfPath.c_str(),
			};

			ec = node::Start(g_argc, (char**)g_argv, std::size(execArgv), const_cast<char**>(execArgv));
#else
			ec = node::Start(g_argc, (char**)g_argv, 0, nullptr);
#endif			
		}).join();

#ifdef _WIN32
		// newer Node won't play nice
		TerminateProcess(GetCurrentProcess(), ec);
#else
		exit(ec);
#endif
	}
#endif

	// initialize platform
#ifndef IS_FXSERVER
	m_platform = std::unique_ptr<v8::Platform>(v8::platform::NewDefaultPlatform(0, platform::IdleTaskSupport::kDisabled, platform::InProcessStackDumping::kDisabled));
	V8::InitializePlatform(m_platform.get());
#else
	auto platform = node::InitializeV8Platform(4);
	m_platform = std::unique_ptr<v8::Platform>(platform);
#endif

#if 0
	// set profiling disabled, but timer event logging enabled
	int argc = 4;
	const char* argv[] =
	{
		"",
		"--noprof-auto",
		"--prof",
		"--log-timer-events"
	};

	V8::SetFlagsFromCommandLine(&argc, (char**)argv, false);
#endif

	const char* flags = "--expose_gc";
	V8::SetFlagsFromString(flags, strlen(flags));

#ifdef _WIN32
#ifdef IS_FXSERVER
	V8::InitializeICUDefaultLocation(ToNarrow(MakeRelativeCitPath(L"dummy")).c_str());
#else
	V8::InitializeICUDefaultLocation(ToNarrow(MakeRelativeCitPath(L"dummy")).c_str(),
		ToNarrow(MakeRelativeCitPath(L"citizen/scripting/v8/icudtl.dat")).c_str());
#endif
#endif

	// initialize global V8
	V8::Initialize();

	// create an array buffer allocator
	m_arrayBufferAllocator = std::make_unique<ArrayBufferAllocator>();

	// create an isolate
	Isolate::CreateParams params;
	params.array_buffer_allocator = m_arrayBufferAllocator.get();

	m_isolate = Isolate::Allocate();
	
#ifdef IS_FXSERVER
	platform->RegisterIsolate(m_isolate, Instance<net::UvLoopManager>::Get()->GetOrCreate(std::string("svMain"))->GetLoop());
#endif

	Isolate::Initialize(m_isolate, params);

	m_isolate->SetFatalErrorHandler([] (const char* location, const char* message)
	{
		FatalError("V8 error at %s: %s", location, message);
	});

	m_isolate->SetCaptureStackTraceForUncaughtExceptions(true);

	m_isolate->AddMessageListener(OnMessage);

	// initialize the debugger
	m_debugger = std::unique_ptr<V8Debugger>(CreateDebugger(m_isolate));

#ifdef IS_FXSERVER
	// initialize Node.js
	Locker locker(m_isolate);
	Isolate::Scope isolateScope(m_isolate);
	v8::HandleScope handle_scope(m_isolate);

	static std::stack<std::unique_ptr<BasePushEnvironment>> envStack;

	node::SetScopeHandler([](const node::Environment* env)
	{
		auto runtime = GetEnvRuntime(env);

		if (runtime)
		{
			envStack.push(std::make_unique<V8LitePushEnvironment>(runtime, env));
		}
		else
		{
			envStack.push(std::make_unique<V8LiteNoRuntimePushEnvironment>(env));
		}
	}, [](const node::Environment* env)
	{
		envStack.pop();
	});

	int argc = 2;
	const char* argv[] = { "", "--expose-internals" };

	node::Init(&argc, argv, &eac, &eav);

	m_nodeData = node::CreateIsolateData(m_isolate, Instance<net::UvLoopManager>::Get()->GetOrCreate(std::string("svMain"))->GetLoop());
#endif
}

static InitFunction initFunction([]()
{
	g_v8.Initialize();

	// trigger removing funcrefs on the *resource manager* so that it'll still happen when a runtime is destroyed
	ResourceManager::OnInitializeInstance.Connect([](ResourceManager* manager)
	{
		manager->OnTick.Connect([]()
		{
			RefAndPersistent* deleteRef;

			while (g_cleanUpFuncRefs.try_pop(deleteRef))
			{
				delete deleteRef;
			}
		});
	});
});

V8ScriptGlobals::~V8ScriptGlobals()
{
	// clean up V8
	/*m_isolate->Dispose();

	V8::Dispose();
	V8::ShutdownPlatform();*/

	// actually don't, this deadlocks from a global destructor
#ifdef IS_FXSERVER
	m_platform.release();
#endif
}

// {9C268449-7AF4-4A3B-995A-3B1692E958AC}
FX_DEFINE_GUID(CLSID_V8ScriptRuntime,
			   0x9c268449, 0x7af4, 0x4a3b, 0x99, 0x5a, 0x3b, 0x16, 0x92, 0xe9, 0x58, 0xac);


FX_NEW_FACTORY(V8ScriptRuntime);

FX_IMPLEMENTS(CLSID_V8ScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_V8ScriptRuntime, IScriptFileHandlingRuntime);
}
