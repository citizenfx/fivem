/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "RuntimeHelpers.h"
#include "SharedFunction.h"

namespace fx::v8shared
{
	template<class RuntimeType>
	static void V8_SetTickFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		v8::Local<v8::Function> tickFunction = v8::Local<v8::Function>::Cast(args[0]);
		v8::UniquePersistent<v8::Function> tickFunctionRef(isolate, tickFunction);

		runtime->SetTickRoutine(make_shared_function([runtime, isolate, tickFunctionRef{ std::move(tickFunctionRef) }]()
			{
				v8::Local<v8::Function> tickFunction = tickFunctionRef.Get(isolate);

				{
					v8::TryCatch eh(isolate);

					auto time = v8::Number::New(isolate, (double)msec().count());
					v8::Local<v8::Value> args[] = {
						time.As<v8::Value>()
					};

					v8::MaybeLocal<v8::Value> value = tickFunction->Call(runtime->GetContext(), v8::Null(isolate), std::size(args), args);

					if (value.IsEmpty())
					{
						v8::String::Utf8Value str(isolate, eh.Exception());
						v8::String::Utf8Value stack(isolate, GetStackTrace(eh, runtime));

						ScriptTrace(runtime, "Error calling system tick function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
					}
				}
			}));
	}

	template<class RuntimeType>
	static void V8_SetEventFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();
		
		v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast(args[0]);
		v8::UniquePersistent<v8::Function> functionRef(isolate, function);

		runtime->SetEventRoutine(make_shared_function([runtime, isolate, functionRef{ std::move(functionRef) }](const char* eventName, const char* eventPayload, size_t payloadSize, const char* eventSource)
			{
				v8::Local<v8::Function> function = functionRef.Get(isolate);

				{
					v8::TryCatch eh(isolate);

					v8::Local<v8::ArrayBuffer> inValueBuffer = v8::ArrayBuffer::New(isolate, payloadSize);
					auto abs = inValueBuffer->GetBackingStore();
					memcpy(abs->Data(), eventPayload, payloadSize);

					v8::Local<v8::Value> arguments[3];
					arguments[0] = v8::String::NewFromUtf8(isolate, eventName).ToLocalChecked();
					arguments[1] = v8::Uint8Array::New(inValueBuffer, 0, payloadSize);
					arguments[2] = v8::String::NewFromUtf8(isolate, eventSource).ToLocalChecked();

					v8::MaybeLocal<v8::Value> value = function->Call(runtime->GetContext(), v8::Null(isolate), 3, arguments);

					if (eh.HasCaught())
					{
						v8::String::Utf8Value str(isolate, eh.Exception());
						v8::String::Utf8Value stack(isolate, GetStackTrace(eh, runtime));

						ScriptTrace(runtime, "Error calling system event handling function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
					}
				}
			}));
	}

	template<class RuntimeType>
	static void V8_SetStackTraceRoutine(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast(args[0]);
		v8::UniquePersistent<v8::Function> functionRef(isolate, function);

		runtime->SetStackTraceRoutine(make_shared_function([runtime, isolate, functionRef{ std::move(functionRef) }](void* start, void* end, char** blob, size_t* size)
			{
				// static array for retval output (sadly)
				static std::vector<char> retvalArray(32768);

				v8::Local<v8::Function> function = functionRef.Get(isolate);

				{
					v8::TryCatch eh(isolate);

					v8::Local<v8::Value> arguments[2];

					// push arguments on the stack
					if (start)
					{
						auto startRef = (V8Boundary*)start;
						arguments[0] = v8::Int32::New(isolate, startRef->hint);
					}
					else
					{
						arguments[0] = v8::Null(isolate);
					}

					if (end)
					{
						auto endRef = (V8Boundary*)end;
						arguments[1] = v8::Int32::New(isolate, endRef->hint);
					}
					else
					{
						arguments[1] = v8::Null(isolate);
					}

					v8::MaybeLocal<v8::Value> mv = function->Call(runtime->GetContext(), v8::Null(isolate), 2, arguments);

					if (eh.HasCaught())
					{
						v8::String::Utf8Value str(isolate, eh.Exception());
						v8::String::Utf8Value stack(isolate, GetStackTrace(eh, runtime));

						ScriptTrace(runtime, "Error calling system stack trace function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
					}
					else
					{
						v8::Local<v8::Value> value = mv.ToLocalChecked();

						if (!value->IsArrayBufferView())
						{
							return;
						}

						v8::Local<v8::ArrayBufferView> abv = value.As<v8::ArrayBufferView>();
						*size = abv->ByteLength();

						if (*size > retvalArray.size())
						{
							retvalArray.resize(*size);
						}

						abv->CopyContents(retvalArray.data(), fwMin(retvalArray.size(), *size));
						*blob = retvalArray.data();
					}
				}
			}));
	}

	template<class RuntimeType>
	static void V8_SetUnhandledPromiseRejectionRoutine(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast(args[0]);
		v8::UniquePersistent<v8::Function> functionRef(isolate, function);

		runtime->SetUnhandledPromiseRejectionRoutine(make_shared_function([runtime, isolate, functionRef{ std::move(functionRef) }](v8::PromiseRejectMessage& message)
			{
				v8::Local<v8::Promise> promise = message.GetPromise();
				v8::Isolate* isolate = promise->GetIsolate();
				v8::Local<v8::Value> value = message.GetValue();
				v8::Local<v8::Integer> event = v8::Integer::New(isolate, message.GetEvent());

				if (value.IsEmpty())
				{
					value = Undefined(isolate);
				}

				v8::Local<v8::Function> function = functionRef.Get(isolate);

				{
					v8::TryCatch eh(isolate);

					auto time = v8::Number::New(isolate, (double)msec().count());
					v8::Local<v8::Value> args[] = {
						event, promise, value
					};

					v8::MaybeLocal<v8::Value> value = function->Call(runtime->GetContext(), v8::Null(isolate), std::size(args), args);

					if (value.IsEmpty())
					{
						v8::String::Utf8Value str(isolate, eh.Exception());
						v8::String::Utf8Value stack(isolate, GetStackTrace(eh, runtime));

						ScriptTrace(runtime, "Unhandled error during handling of unhandled promise rejection in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
					}
				}
			}));
	}

	static void V8_GetTickCount(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		args.GetReturnValue().Set((double)msec().count());
	}
	
	template<class RuntimeType>
	static v8::Local<v8::Value> GetStackTrace(v8::TryCatch& eh, RuntimeType* runtime)
	{
		auto stackTraceHandle = eh.StackTrace(runtime->GetContext());

		if (stackTraceHandle.IsEmpty())
		{
			return v8::String::NewFromUtf8(runtime->GetIsolate(), "<empty stack trace>").ToLocalChecked();
		}

		return stackTraceHandle.ToLocalChecked();
	}

	template<class RuntimeType, class PushType>
	static void V8_Trace(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		bool first = true;

		auto runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);

		PushType pushed(args.GetIsolate());

		for (int i = 0; i < args.Length(); i++)
		{
			if (first)
			{
				first = false;
			}
			else
			{
				ScriptTrace(runtime, " ");
			}

			v8::String::Utf8Value str(args.GetIsolate(), args[i]);
			ScriptTrace(runtime, "%s", std::string_view{ *str, size_t(str.length()) });
		}

		ScriptTrace(runtime, "\n");
	}

	template<class RuntimeType>
	static void V8_GetResourcePath(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		auto runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto path = ((fx::Resource*)runtime->GetParentObject())->GetPath();

		args.GetReturnValue().Set(v8::String::NewFromUtf8(args.GetIsolate(), path.c_str(), v8::NewStringType::kNormal, path.size()).ToLocalChecked());
	}
}
