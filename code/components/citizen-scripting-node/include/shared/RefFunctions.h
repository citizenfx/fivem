/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ResourceCallbackComponent.h>
#include <tbb/concurrent_queue.h>

#include "RuntimeHelpers.h"
#include "SharedFunction.h"

namespace fx::v8shared
{
	template<class RuntimeType>
	static void V8_SetCallRefFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast(args[0]);
		v8::UniquePersistent<v8::Function> functionRef(isolate, function);

		runtime->SetCallRefRoutine(make_shared_function([runtime, isolate, functionRef{ std::move(functionRef) }](int32_t refId, const char* argsSerialized, size_t argsSize)
			{
				v8::Local<v8::Function> function = functionRef.Get(isolate);

				{
					v8::TryCatch eh(isolate);

					v8::Local<v8::ArrayBuffer> inValueBuffer = v8::ArrayBuffer::New(isolate, argsSize);
					auto abs = inValueBuffer->GetBackingStore();
					memcpy(abs->Data(), argsSerialized, argsSize);

					v8::Local<v8::Value> arguments[2];
					arguments[0] = v8::Int32::New(isolate, refId);
					arguments[1] = v8::Uint8Array::New(inValueBuffer, 0, argsSize);

					v8::MaybeLocal<v8::Value> maybeValue = function->Call(runtime->GetContext(), v8::Null(isolate), 2, arguments);

					fx::OMPtr<IScriptBuffer> rv;

					if (eh.HasCaught())
					{
						v8::String::Utf8Value str(isolate, eh.Exception());
						v8::String::Utf8Value stack(isolate, GetStackTrace(eh, runtime));

						ScriptTrace(runtime, "Error calling system call ref function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
					}
					else
					{
						v8::Local<v8::Value> value = maybeValue.ToLocalChecked();

						if (value->IsArrayBufferView())
						{
							v8::Local<v8::ArrayBufferView> abv = value.As<v8::ArrayBufferView>();
							rv = fx::MemoryScriptBuffer::Make(abv->ByteLength());
							if (rv.GetRef() && rv->GetBytes())
							{
								abv->CopyContents(rv->GetBytes(), abv->ByteLength());
							}
						}
					}
					return rv;
				}
			}));
	}

	template<class RuntimeType>
	static void V8_SetDeleteRefFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast(args[0]);
		v8::UniquePersistent<v8::Function> functionRef(isolate, function);

		runtime->SetDeleteRefRoutine(make_shared_function([runtime, isolate, functionRef{ std::move(functionRef) }](int32_t refId)
			{
				v8::Local<v8::Function> function = functionRef.Get(isolate);

				{
					v8::TryCatch eh(isolate);

					v8::Local<v8::Value> arguments[3];
					arguments[0] = v8::Int32::New(isolate, refId);

					v8::MaybeLocal<v8::Value> value = function->Call(runtime->GetContext(), v8::Null(isolate), 1, arguments);

					if (eh.HasCaught())
					{
						v8::String::Utf8Value str(isolate, eh.Exception());
						v8::String::Utf8Value stack(isolate, GetStackTrace(eh, runtime));

						ScriptTrace(runtime, "Error calling system delete ref function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
					}
				}
			}));
	}

	template<class RuntimeType>
	static void V8_SetDuplicateRefFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast(args[0]);
		v8::UniquePersistent<v8::Function> functionRef(isolate, function);

		runtime->SetDuplicateRefRoutine(make_shared_function([runtime, isolate, functionRef{ std::move(functionRef) }](int32_t refId)
			{
				v8::Local<v8::Function> function = functionRef.Get(isolate);

				{
					v8::TryCatch eh(isolate);

					v8::Local<v8::Value> arguments[3];
					arguments[0] = v8::Int32::New(isolate, refId);

					v8::MaybeLocal<v8::Value> value = function->Call(runtime->GetContext(), v8::Null(isolate), 1, arguments);

					if (eh.HasCaught())
					{
						v8::String::Utf8Value str(isolate, eh.Exception());
						v8::String::Utf8Value stack(isolate, GetStackTrace(eh, runtime));

						ScriptTrace(runtime, "Error calling system duplicate ref function in resource %s: %s\nstack:\n%s\n", runtime->GetResourceName(), *str, *stack);
					}
					else
					{
						v8::Local<v8::Value> realValue;

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

	template<class RuntimeType>
	static void V8_CanonicalizeRef(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		char* refString;
		runtime->GetScriptHost()->CanonicalizeRef(args[0]->Int32Value(runtime->GetContext()).ToChecked(), runtime->GetInstanceId(), &refString);

		args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, refString).ToLocalChecked());
		fwFree(refString);
	}

	template<class RuntimeType>
	struct RefAndPersistent {
		fx::FunctionRef ref;
		v8::Persistent<v8::Function> handle;
		fx::OMPtr<RuntimeType> runtime;
		fx::OMPtr<IScriptHost> host;
	};

	static tbb::concurrent_queue<void*> g_cleanUpFuncRefs;

	template<class RuntimeType>
	static void V8_InvokeFunctionReference(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		auto external = v8::Local<v8::External>::Cast(args.Data());
		auto refData = reinterpret_cast<RefAndPersistent<RuntimeType>*>(external->Value());

		// push the current runtime
		fx::PushEnvironment fxenv(refData->runtime);

		OMPtr<IScriptHost> scriptHost = refData->runtime->GetScriptHost();

		v8::Local<v8::ArrayBufferView> abv = v8::Local<v8::ArrayBufferView>::Cast(args[0]);

		std::vector<uint8_t> argsBuffer(abv->ByteLength());

		abv->CopyContents(argsBuffer.data(), argsBuffer.size());

		fx::OMPtr<IScriptBuffer> retvalBuffer;
		if (FX_FAILED(scriptHost->InvokeFunctionReference(const_cast<char*>(refData->ref.GetRef().c_str()), reinterpret_cast<char*>(argsBuffer.data()), argsBuffer.size(), retvalBuffer.GetAddressOf())))
		{
			char* error = "Unknown";
			scriptHost->GetLastErrorText(&error);

			auto throwException = [&](const std::string& exceptionString)
				{
					args.GetIsolate()->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(args.GetIsolate(), exceptionString.c_str()).ToLocalChecked()));
				};

			return throwException(error);
		}

		size_t retLength = (retvalBuffer.GetRef()) ? retvalBuffer->GetLength() : 0;

		// get return values
		v8::Local<v8::ArrayBuffer> outValueBuffer = v8::ArrayBuffer::New(args.GetIsolate(), retLength);
		if (retLength > 0)
		{
			auto abs = outValueBuffer->GetBackingStore();
			memcpy(abs->Data(), retvalBuffer->GetBytes(), retLength);
		}

		v8::Local<v8::Uint8Array> outArray = v8::Uint8Array::New(outValueBuffer, 0, retLength);
		args.GetReturnValue().Set(outArray);
	}

	template<class RuntimeType>
	static void FnRefWeakCallback(const v8::WeakCallbackInfo<RefAndPersistent<RuntimeType>>& data)
	{
		v8::HandleScope handleScope(data.GetIsolate());
		v8::Local<v8::Function> v = data.GetParameter()->handle.Get(data.GetIsolate());

		data.GetParameter()->handle.Reset();

		// defer this to the next tick so that we won't end up deadlocking (the V8 lock is held at GC interrupt time, but the runtime lock is not)
		g_cleanUpFuncRefs.push(data.GetParameter());
	}

	template<class RuntimeType>
	static void V8_MakeFunctionReference(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		auto isolate = runtime->GetIsolate();

		std::string refString;

		if (args[0]->IsString())
		{
			v8::Local<v8::String> string = v8::Local<v8::String>::Cast(args[0]);
			v8::String::Utf8Value utf8{ isolate, string };
			refString = *utf8;
		}
		else if (args[0]->IsUint8Array())
		{
			v8::Local<v8::Uint8Array> arr = v8::Local<v8::Uint8Array>::Cast(args[0]);

			std::vector<uint8_t> data(arr->ByteLength());
			arr->CopyContents(data.data(), data.size());

			refString = std::string(reinterpret_cast<char*>(data.data()), data.size());
		}

		RefAndPersistent<RuntimeType>* data = new RefAndPersistent<RuntimeType>;
		data->ref = fx::FunctionRef{ refString };
		data->runtime = runtime;
		data->host = runtime->GetScriptHost();

		v8::MaybeLocal<v8::Function> outFunction = v8::Function::New(runtime->GetContext(), V8_InvokeFunctionReference<RuntimeType>, v8::External::New(isolate, data));

		v8::Local<v8::Function> outFn;

		if (outFunction.ToLocal(&outFn))
		{
			data->handle.Reset(isolate, outFn);
			data->handle.SetWeak(data, FnRefWeakCallback, v8::WeakCallbackType::kParameter);

			args.GetReturnValue().Set(outFn);
		}
		else
		{
			delete data;
		}
	}
}
