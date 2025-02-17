/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <rapidjson/writer.h>
#include <MsgpackJson.h>
#include <ScriptInvoker.h>

#include "RuntimeHelpers.h"
#include "MetaFieldFunctions.h"

namespace fx::v8shared
{
	template<class RuntimeType>
	struct V8ScriptNativeContext final : ScriptNativeContext
	{
		inline V8ScriptNativeContext(uint64_t hash, RuntimeType* runtime, v8::Isolate* isolate_)
		: ScriptNativeContext(hash), isolateScope(isolate_), runtime(runtime), isolate(isolate_), cxt(runtime->GetContext())
		{
		}

		void PushArgument(v8::Local<v8::Value> arg)
		{
			if (arg->IsNumber())
			{
				double value = arg->NumberValue(cxt).ToChecked();
				int64_t intValue = static_cast<int64_t>(value);

				if (intValue == value)
				{
					return Push(intValue);
				}
				else
				{
					return Push(value);
				}
			}
			else if (arg->IsBoolean() || arg->IsBooleanObject())
			{
				return Push(arg->BooleanValue(isolate));
			}
			else if (arg->IsString())
			{
				// TODO: Should these be marked as immutable?
				size_t length = 0;
				const char* data = runtime->AssignStringValue(arg, &length);
				return Push(data, length);
			}
			// null/undefined: add '0'
			else if (arg->IsNull() || arg->IsUndefined())
			{
				return Push(0);
			}
			// metafield
			else if (arg->IsExternal())
			{
				uint8_t* ptr = reinterpret_cast<uint8_t*>(v8::Local<v8::External>::Cast(arg)->Value());

				return PushMetaPointer(ptr);
			}
			// placeholder vectors
			else if (arg->IsArray())
			{
				v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(arg);
				float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

				auto getNumber = [&](int idx)
				{
					v8::Local<v8::Value> value;

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
					ScriptError("arrays should be vectors (wrong number of values)");
				}

				if (array->Length() >= 2)
				{
					x = getNumber(0);
					y = getNumber(1);

					if (x == NAN || y == NAN)
					{
						ScriptError("invalid vector array value");
					}

					Push(x);
					Push(y);
				}

				if (array->Length() >= 3)
				{
					z = getNumber(2);

					if (z == NAN)
					{
						ScriptError("invalid vector array value");
					}

					Push(z);
				}

				if (array->Length() >= 4)
				{
					w = getNumber(3);

					if (w == NAN)
					{
						ScriptError("invalid vector array value");
					}

					Push(w);
				}
			}
			else if (arg->IsArrayBufferView())
			{
				v8::Local<v8::ArrayBufferView> abv = arg.As<v8::ArrayBufferView>();
				v8::Local<v8::ArrayBuffer> buffer = abv->Buffer();

				auto abs = buffer->GetBackingStore();
				return Push((uint8_t*)abs->Data() + abv->ByteOffset(), abs->ByteLength());
			}
			// this should be the last entry, I'd guess
			else if (arg->IsObject())
			{
				v8::Local<v8::Object> object = arg->ToObject(cxt).ToLocalChecked();
				v8::Local<v8::Value> data;

				if (!object->Get(cxt, v8::String::NewFromUtf8(isolate, "__data").ToLocalChecked()).ToLocal(&data))
				{
					ScriptError("__data field does not contain a number");
				}

				if (!data.IsEmpty() && data->IsNumber())
				{
					auto n = data->ToNumber(runtime->GetContext());
					v8::Local<v8::Number> number;

					if (n.ToLocal(&number))
					{
						return Push(number->Int32Value(cxt).ToChecked());
					}
				}
				else
				{
					ScriptError("__data field does not contain a number");
				}
			}
			else
			{
				v8::String::Utf8Value str(isolate, arg);
				ScriptErrorf("invalid V8 value: %s", *str);
			}
		}

		template<typename T>
		CSCRC_INLINE v8::Local<v8::Value> ProcessResult(const T& value)
		{
			if constexpr (std::is_same_v<T, bool>)
			{
				return v8::Boolean::New(isolate, value);
			}
			else if constexpr (std::is_same_v<T, int32_t>)
			{
				return v8::Int32::New(isolate, value);
			}
			else if constexpr (std::is_same_v<T, int64_t>)
			{
				return v8::Number::New(isolate, (double) value);
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				return v8::Number::New(isolate, value);
			}
			else if constexpr (std::is_same_v<T, ScrVector>)
			{
				v8::Local<v8::Array> result = v8::Array::New(isolate, 3);

				result->Set(cxt, 0, v8::Number::New(isolate, value.x));
				result->Set(cxt, 1, v8::Number::New(isolate, value.y));
				result->Set(cxt, 2, v8::Number::New(isolate, value.z));

				return result;
			}
			else if constexpr (std::is_same_v<T, const char*>)
			{
				if (value)
				{
					return v8::String::NewFromUtf8(isolate, value).ToLocalChecked();
				}
				else
				{
					return v8::Null(isolate);
				}
			}
			else if constexpr (std::is_same_v<T, ScrString>)
			{
				// FIXME: This wasn't properly supported before. Should it return a Uint8Buffer buffer instead?
				return v8::String::NewFromUtf8(isolate, value.str, v8::NewStringType::kNormal, (int) value.len).ToLocalChecked();
			}
			else if constexpr (std::is_same_v<T, ScrObject>)
			{
				try
				{
					msgpack::unpacked unpacked;
					msgpack::unpack(unpacked, value.data, value.length);

					// and convert to a rapidjson object
					rapidjson::Document document;
					ConvertToJSON(unpacked.get(), document, document.GetAllocator());

					// write as a json string
					rapidjson::StringBuffer sb;
					rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

					if (document.Accept(writer))
					{
						if (sb.GetString() && sb.GetSize())
						{
							auto maybeString = v8::String::NewFromUtf8(isolate, sb.GetString(), v8::NewStringType::kNormal, sb.GetSize());
							v8::Local<v8::String> string;

							if (maybeString.ToLocal(&string))
							{
								auto maybeValue = v8::JSON::Parse(runtime->GetContext(), string);
								v8::Local<v8::Value> value;

								if (maybeValue.ToLocal(&value))
								{
									return value;
								}
							}
						}
					}
				}
				catch (std::exception& /*e*/)
				{
				}

				return v8::Null(isolate);
			}
			else
			{
				static_assert(always_false_v<T>, "Invalid return type");
			}
		}
		
		v8::Isolate* isolate;
		v8::Local<v8::Context> cxt;
		v8::Isolate::Scope isolateScope;
		RuntimeType* runtime;
	};
	
	template<class RuntimeType>
	static void V8_InvokeNative(const v8::FunctionCallbackInfo<v8::Value>& args, uint64_t hash, int baseArgs)
	{
		// get required entries
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);

		// variables to hold state
		V8ScriptNativeContext<RuntimeType> context(hash, runtime, args.GetIsolate());

		// push the current runtime so natives will work as intended
		fx::PushEnvironment fxenv(runtime);

		// get argument count for the loop
		int numArgs = args.Length();
	
		for (int i = baseArgs; i < numArgs; i++)
		{
			context.PushArgument(args[i]);
		}

		context.Invoke();

		// For a single result, return it directly.
		// For multiple results, store them in an array.
		v8::Local<v8::Value> returnValue = Undefined(context.isolate);
		int numResults = 0;

		context.ProcessResults([&](auto&& value) {
			v8::Local<v8::Value> val = context.ProcessResult(value);

			if (numResults == 0)
			{
				returnValue = val;
			}
			else
			{
				if (numResults == 1)
				{
					v8::Local<v8::Array> arrayValue = v8::Array::New(context.isolate);
					arrayValue->Set(context.cxt, 0, returnValue);
					returnValue = arrayValue;
				}

				returnValue.As<v8::Array>()->Set(context.cxt, numResults, val);
			}

			++numResults;
		});

		// and set the return value(s)
		args.GetReturnValue().Set(returnValue);
	}
	
	template<class RuntimeType>
	static void V8_InvokeNativeString(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		V8_TryCatch(args, [](const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1)
			{
				throw std::runtime_error("wrong argument count (needs at least a hash string)");
			}

			v8::String::Utf8Value hashString(args.GetIsolate(), args[0]);
			uint64_t hash = strtoull(*hashString, nullptr, 16);
			return V8_InvokeNative<RuntimeType>(args, hash, 1);
		});
	}

	template<class RuntimeType>
	static void V8_InvokeNativeHash(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		V8_TryCatch(args, [](const v8::FunctionCallbackInfo<v8::Value>& args) {
			if (args.Length() < 1)
			{
				throw std::runtime_error("wrong argument count (needs at least two hash integers)");
			}
			auto scrt = GetScriptRuntimeFromArgs<RuntimeType>(args);
			uint64_t hash = (args[1]->Uint32Value(scrt->GetContext()).ToChecked() | (((uint64_t)args[0]->Uint32Value(scrt->GetContext()).ToChecked()) << 32));
			return V8_InvokeNative<RuntimeType>(args, hash, 2);
		});
	}
}
