/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <rapidjson/writer.h>
#include <MsgpackJson.h>

#include "RuntimeHelpers.h"
#include "MetaFieldFunctions.h"

namespace fx::v8shared
{
	struct StringHashGetter
	{
		static const int BaseArgs = 1;

		uint64_t operator()(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			v8::String::Utf8Value hashString(args.GetIsolate(), args[0]);
			return strtoull(*hashString, nullptr, 16);
		}
	};

	struct IntHashGetter
	{
		static const int BaseArgs = 2;

		uint64_t operator()(const v8::FunctionCallbackInfo<v8::Value>& args)
		{
			auto ctx = args.GetIsolate()->GetCurrentContext();
			return (args[1]->Uint32Value(ctx).ToChecked() | (((uint64_t)args[0]->Uint32Value(ctx).ToChecked()) << 32));
		}
	};

	// Sanitization for string result types
	// Loops through all values given by the ScRT and deny any that equals the result value which isn't of the string type
	template<class RuntimeType, typename HashGetter>
	void NativeStringResultSanitization(const v8::FunctionCallbackInfo<v8::Value>& inputArguments, fxNativeContext& context, decltype(context.arguments)& initialArguments)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		const auto resultValue = context.arguments[0];

		// Step 1: quick compare all values until we found a hit
		// By not switching between all the buffers (incl. input arguments) we'll not introduce unnecessary cache misses.
		for (int a = 0; a < context.numArguments; ++a)
		{
			if (initialArguments[a] == resultValue)
			{
				// Step 2: loop our input list for as many times as `a` was increased
				const int inputSize = inputArguments.Length();
				for (int i = HashGetter::BaseArgs; i < inputSize; ++i)
				{
					const auto& v8Input = inputArguments[i];

					// `a` can be reused by simply decrementing it, we'll go negative when we hit our goal as we decrement before checking (e.g.: `0 - 1 = -1` or `0 - 4 = -4`)
					a -= v8Input->IsArray() ? std::min(v8::Local<v8::Array>::Cast(v8Input)->Length(), 4u) : 1u;

					// string type is allowed
					if (a < 0)
					{
						if (!v8Input->IsString())
						{
							ScriptTrace(runtime, "Warning: Sanitized coerced string result for native %016x.\n", context.nativeIdentifier);
							context.arguments[0] = 0;
						}

						return; // we found our arg, no more to check
					}
				}

				return; // found our value, no more to check
			}
		}
	}

	template<class RuntimeType, typename HashGetter>
	static void V8_InvokeNative(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		// get required entries
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		OMPtr<IScriptHost> scriptHost = runtime->GetScriptHost();

		// push the current runtime so natives will work as intended
		fx::PushEnvironment fxenv(runtime);

		auto isolate = args.GetIsolate();
		v8::Isolate::Scope isolateScope(isolate);

		auto pointerFields = runtime->GetPointerFields();

		// exception thrower
		auto throwException = [&](const std::string& exceptionString)
		{
			args.GetIsolate()->ThrowException(v8::Exception::Error(v8::String::NewFromUtf8(args.GetIsolate(), exceptionString.c_str()).ToLocalChecked()));
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
		if (numArgs < HashGetter::BaseArgs)
		{
			return throwException("wrong argument count (needs at least a hash string)");
		}

		// get the hash
		uint64_t hash = HashGetter()(args);
		
		context.nativeIdentifier = hash;

		// pushing function
		auto push = [&](const auto& value)
		{
			if (context.numArguments >= std::size(context.arguments))
			{
				return;
			}

			*reinterpret_cast<uintptr_t*>(&context.arguments[context.numArguments]) = 0;
			*reinterpret_cast<std::decay_t<decltype(value)>*>(&context.arguments[context.numArguments]) = value;
			context.numArguments++;
		};

		auto cxt = runtime->GetContext();

		// the big argument loop
		v8::Local<v8::Value> a1;
		if (HashGetter::BaseArgs < numArgs)
		{
			a1 = args[HashGetter::BaseArgs];
		}

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

				uint8_t* ptr = reinterpret_cast<uint8_t*>(v8::Local<v8::External>::Cast(arg)->Value());

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
						returnResultAnyway = true;
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
				v8::Local<v8::ArrayBufferView> abv = arg.As<v8::ArrayBufferView>();
				v8::Local<v8::ArrayBuffer> buffer = abv->Buffer();

				auto abs = buffer->GetBackingStore();
				push((char*)abs->Data() + abv->ByteOffset());
			}
			// this should be the last entry, I'd guess
			else if (arg->IsObject())
			{
				v8::Local<v8::Object> object = arg->ToObject(cxt).ToLocalChecked();
				v8::Local<v8::Value> data;

				if (!object->Get(cxt, v8::String::NewFromUtf8(isolate, "__data").ToLocalChecked()).ToLocal(&data))
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
				v8::String::Utf8Value str(isolate, arg);

				return throwException(va("Invalid V8 value: %s", *str));
			}
		}

#if __has_include(<PointerArgumentHints.h>)
		fx::scripting::ResultType resultTypeIntent = fx::scripting::ResultType::None;

		if (!returnResultAnyway)
		{
			resultTypeIntent = fx::scripting::ResultType::Void;
		}
		else if (returnValueCoercion == V8MetaFields::ResultAsString)
		{
			resultTypeIntent = fx::scripting::ResultType::String;
		}
		else if (returnValueCoercion == V8MetaFields::ResultAsInteger || returnValueCoercion == V8MetaFields::ResultAsLong || returnValueCoercion == V8MetaFields::ResultAsFloat || returnValueCoercion == V8MetaFields::Max /* bool */)
		{
			resultTypeIntent = fx::scripting::ResultType::Scalar;
		}
		else if (returnValueCoercion == V8MetaFields::ResultAsVector)
		{
			resultTypeIntent = fx::scripting::ResultType::Vector;
		}
#endif

#ifndef IS_FXSERVER
		bool needsResultCheck = (numReturnValues == 0 || returnResultAnyway);
		bool hadComplexType = !a1.IsEmpty() && (!a1->IsNumber() && !a1->IsBoolean() && !a1->IsNull());

		decltype(context.arguments) initialArguments;
		memcpy(initialArguments, context.arguments, sizeof(context.arguments));
#endif

		// invoke the native on the script host
		if (!FX_SUCCEEDED(scriptHost->InvokeNative(context)))
		{
			char* error = "Unknown";
			scriptHost->GetLastErrorText(&error);

			return throwException(fmt::sprintf("Execution of native %016x in script host failed: %s", hash, error));
		}

#ifndef IS_FXSERVER
		// clean up the result
		if ((needsResultCheck || hadComplexType) && context.numArguments > 0)
		{
			// if this is scrstring but nothing changed (very weird!), fatally fail
			if (static_cast<uint32_t>(context.arguments[2]) == 0xFEED1212 && initialArguments[2] == context.arguments[2])
			{
				FatalError("Invalid native call in V8 resource '%s'. Please see https://aka.cfx.re/scrstring-mitigation for more information.", runtime->GetResourceName());
			}

			// If return coercion is String type
			// Don't allow deref'ing arbitrary pointers passed in any of the arguments.
			if (returnValueCoercion == V8MetaFields::ResultAsString && context.arguments[0])
			{
				NativeStringResultSanitization<HashGetter>(args, context, initialArguments);
			}
			// if the first value (usually result) is the same as the initial argument, clear the result (usually, result was no-op)
			// (if vector results, these aren't directly unsafe, and may get incorrectly seen as complex)
			else if (context.arguments[0] == initialArguments[0] && returnValueCoercion != V8MetaFields::ResultAsVector)
			{
				// complex type in first result means we have to clear that result
				if (hadComplexType)
				{
					context.arguments[0] = 0;
				}

				// if any result is requested and there was *no* change, zero out
				context.arguments[1] = 0;
				context.arguments[2] = 0;
				context.arguments[3] = 0;
			}
		}
#endif

#if __has_include(<PointerArgumentHints.h>)
		if (resultTypeIntent != fx::scripting::ResultType::None)
		{
			fx::scripting::PointerArgumentHints::CleanNativeResult(hash, resultTypeIntent, context.arguments);
		}
#endif

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

		// JS results
		v8::Local<v8::Value> returnValue = Undefined(args.GetIsolate());
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
					returnValue = v8::String::NewFromUtf8(args.GetIsolate(), str).ToLocalChecked();
				}
				else
				{
					returnValue = v8::Null(args.GetIsolate());
				}

				break;
			}
			case V8MetaFields::ResultAsFloat:
				returnValue = v8::Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&context.arguments[0]));
				break;
			case V8MetaFields::ResultAsVector:
			{
				scrVector vector = *reinterpret_cast<scrVector*>(&context.arguments[0]);

				v8::Local<v8::Array> vectorArray = v8::Array::New(args.GetIsolate(), 3);
				vectorArray->Set(cxt, 0, v8::Number::New(args.GetIsolate(), vector.x));
				vectorArray->Set(cxt, 1, v8::Number::New(args.GetIsolate(), vector.y));
				vectorArray->Set(cxt, 2, v8::Number::New(args.GetIsolate(), vector.z));

				returnValue = vectorArray;

				break;
			}
			case V8MetaFields::ResultAsObject:
			{
				scrObject object = *reinterpret_cast<scrObject*>(&context.arguments[0]);

				// try parsing
				returnValue = v8::Null(isolate);

				try
				{
					msgpack::unpacked unpacked;
					msgpack::unpack(unpacked, object.data, object.length);

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
									returnValue = value;
								}
							}
						}
					}
				}
				catch (std::exception&){}

				break;
			}
			case V8MetaFields::ResultAsInteger:
				returnValue = v8::Int32::New(args.GetIsolate(), *reinterpret_cast<int32_t*>(&context.arguments[0]));
				break;
			case V8MetaFields::ResultAsLong:
				returnValue = v8::Number::New(args.GetIsolate(), double(*reinterpret_cast<int64_t*>(&context.arguments[0])));
				break;
			default:
			{
				int32_t integer = *reinterpret_cast<int32_t*>(&context.arguments[0]);

				if ((integer & 0xFFFFFFFF) == 0)
				{
					returnValue = v8::Boolean::New(args.GetIsolate(), false);
				}
				else
				{
					returnValue = v8::Int32::New(args.GetIsolate(), integer);
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
					returnValue = v8::Int32::New(args.GetIsolate(), retvals[0]);
					break;

				case V8MetaFields::PointerValueFloat:
					returnValue = v8::Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&retvals[0]));
					break;

				case V8MetaFields::PointerValueVector:
				{
					scrVector vector = *reinterpret_cast<scrVector*>(&retvals[0]);

					v8::Local<v8::Array> vectorArray = v8::Array::New(args.GetIsolate(), 3);
					vectorArray->Set(cxt, 0, v8::Number::New(args.GetIsolate(), vector.x));
					vectorArray->Set(cxt, 1, v8::Number::New(args.GetIsolate(), vector.y));
					vectorArray->Set(cxt, 2, v8::Number::New(args.GetIsolate(), vector.z));

					returnValue = vectorArray;
					break;
				}
				}
			}
			else if (numReturnValues > 0)
			{
				v8::Local<v8::Object> arrayValue = v8::Array::New(args.GetIsolate());

				// transform into array
				{
					v8::Local<v8::Value> oldValue = returnValue;

					returnValue = arrayValue;
					arrayValue->Set(cxt, 0, oldValue);
				}

				while (i < numReturnValues)
				{
					switch (rettypes[i])
					{
					case V8MetaFields::PointerValueInt:
						arrayValue->Set(cxt, numResults, v8::Int32::New(args.GetIsolate(), retvals[i]));
						i++;
						break;

					case V8MetaFields::PointerValueFloat:
						arrayValue->Set(cxt, numResults, v8::Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&retvals[i])));
						i++;
						break;

					case V8MetaFields::PointerValueVector:
					{
						scrVector vector = *reinterpret_cast<scrVector*>(&retvals[i]);

						v8::Local<v8::Array> vectorArray = v8::Array::New(args.GetIsolate(), 3);
						vectorArray->Set(cxt, 0, v8::Number::New(args.GetIsolate(), vector.x));
						vectorArray->Set(cxt, 1, v8::Number::New(args.GetIsolate(), vector.y));
						vectorArray->Set(cxt, 2, v8::Number::New(args.GetIsolate(), vector.z));

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
}
