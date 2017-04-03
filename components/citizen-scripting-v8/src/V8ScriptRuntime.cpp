/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "fxScripting.h"

#include <include/v8.h>
#include <include/v8-profiler.h>

#include <V8Platform.h>
#include <V8Debugger.h>

#include <om/OMComponent.h>

#include <fstream>

#include <Error.h>

using namespace v8;

namespace fx
{
static Isolate* GetV8Isolate();

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

class V8ScriptRuntime : public OMClass<V8ScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime>
{
private:
	UniquePersistent<Context> m_context;

	std::function<void()> m_tickRoutine;

	IScriptHost* m_scriptHost;

	int m_instanceId;

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

	inline Local<Context> GetContext()
	{
		return m_context.Get(GetV8Isolate());
	}

	inline void SetTickRoutine(const std::function<void()>& tickRoutine)
	{
		m_tickRoutine = tickRoutine;
	}

	inline OMPtr<IScriptHost> GetScriptHost()
	{
		return m_scriptHost;
	}

	inline PointerField* GetPointerFields()
	{
		return m_pointerFields;
	}

	const char* AssignStringValue(const Local<Value>& value);

	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;

	NS_DECL_ISCRIPTTICKRUNTIME;
};

class V8PushEnvironment
{
private:
	Locker m_locker;

	Isolate::Scope m_isolateScope;

	HandleScope m_handleScope;

	Context::Scope m_contextScope;

	fx::PushEnvironment m_pushEnvironment;

public:
	inline V8PushEnvironment(V8ScriptRuntime* runtime)
		: m_locker(GetV8Isolate()), m_isolateScope(GetV8Isolate()), m_handleScope(GetV8Isolate()), m_contextScope(runtime->GetContext()), m_pushEnvironment(runtime)
	{

	}

	~V8PushEnvironment() = default;
};

static V8ScriptRuntime* GetScriptRuntimeFromArgs(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto external = Local<External>::Cast(args.Data());

	return reinterpret_cast<V8ScriptRuntime*>(external->Value());
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
	ResultAsFloat,
	ResultAsString,
	ResultAsVector,
	Max
};

static uint8_t g_metaFields[(int)V8MetaFields::Max];

static void V8_SetTickFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);

	Local<Function> tickFunction = Local<Function>::Cast(args[0]);
	UniquePersistent<Function> tickFunctionRef(GetV8Isolate(), tickFunction);

	runtime->SetTickRoutine(make_shared_function([tickFunctionRef{ std::move(tickFunctionRef) }] ()
	{
		Local<Function> tickFunction = tickFunctionRef.Get(GetV8Isolate());

		{
			TryCatch eh;

			Local<Value> value = tickFunction->Call(Null(GetV8Isolate()), 0, nullptr);

			if (value.IsEmpty())
			{
				String::Utf8Value str(eh.Exception());
				String::Utf8Value stack(eh.StackTrace());

				trace("Error calling system tick function in resource %s: %s\nstack:\n%s\n", "TODO", *str, *stack);
			}
		}
	}));
}

static void V8_GetTickCount(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	args.GetReturnValue().Set((double)GetTickCount64());
}

const char* V8ScriptRuntime::AssignStringValue(const Local<Value>& value)
{
	auto stringValue = std::make_unique<String::Utf8Value>(value);
	const char* str = **(stringValue.get());

	// take ownership
	m_stringValues[m_curStringValue] = std::move(stringValue);
	
	// increment the string value
	m_curStringValue = (m_curStringValue + 1) % _countof(m_stringValues);

	// return the string
	return str;
}

static void V8_InvokeNative(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	// get required entries
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);
	OMPtr<IScriptHost> scriptHost = runtime->GetScriptHost();

	auto pointerFields = runtime->GetPointerFields();

	// exception thrower
	auto throwException = [&] (const std::string& exceptionString)
	{
		args.GetIsolate()->ThrowException(String::NewFromUtf8(args.GetIsolate(), exceptionString.c_str()));
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
	String::Utf8Value hashString(args[0]);
	uint64_t hash = _strtoui64(*hashString, nullptr, 16);

	context.nativeIdentifier = hash;

	// pushing function
	auto push = [&] (const auto& value)
	{
		*reinterpret_cast<uintptr_t*>(&context.arguments[context.numArguments]) = 0;
		*reinterpret_cast<std::decay_t<decltype(value)>*>(&context.arguments[context.numArguments]) = value;
		context.numArguments++;
	};

	// the big argument loop
	for (int i = 1; i < numArgs; i++)
	{
		// get the type and decide what to do based on it
		auto& arg = args[i];

		// null/undefined: add '0'
		if (arg->IsNull() || arg->IsUndefined())
		{
			push(0);
		}
		else if (arg->IsNumber() || arg->IsNumberObject())
		{
			Local<Number> number = arg->ToNumber();
			double value = number->Value();

			if (floor(value) == value)
			{
				push(static_cast<int>(value));
			}
			else
			{
				push(static_cast<float>(value));
			}
		}
		else if (arg->IsInt32())
		{
			Local<Int32> int32 = arg->ToInt32();
			push(int32->Int32Value());
		}
		else if (arg->IsUint32())
		{
			Local<Uint32> int32 = arg->ToUint32();
			push(int32->Uint32Value());
		}
		else if (arg->IsBoolean() || arg->IsBooleanObject())
		{
			push(arg->BooleanValue());
		}
		else if (arg->IsString())
		{
			push(runtime->AssignStringValue(arg));
		}
		// placeholder vectors
		else if (arg->IsArray())
		{
			Local<Array> array = Local<Array>::Cast(arg);
			float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

			auto getNumber = [&] (int idx)
			{
				Local<Value> value = array->Get(idx);

				if (value.IsEmpty() || !value->IsNumber())
				{
					return NAN;
				}

				return static_cast<float>(value->NumberValue());
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
		// metafield
		else if (arg->IsExternal())
		{
			auto pushPtr = [&] (V8MetaFields metaField)
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
					case V8MetaFields::ResultAsString:
					case V8MetaFields::ResultAsFloat:
					case V8MetaFields::ResultAsVector:
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
		// this should be the last entry, I'd guess
		else if (arg->IsObject())
		{
			Local<Object> object = arg->ToObject();
			Local<Value> data = object->Get(String::NewFromUtf8(GetV8Isolate(), "__data"));

			if (!data.IsEmpty() && data->IsNumber())
			{
				push(data->ToNumber()->Int32Value());
			}
			else
			{
				return throwException("__data field does not contain a number");
			}
		}
		else
		{
			String::Utf8Value str(arg);

			return throwException(va("Invalid V8 value: %s", *str));
		}
	}

	// invoke the native on the script host
	if (!FX_SUCCEEDED(scriptHost->InvokeNative(context)))
	{
		return throwException(va("Execution of native %016x in script host failed.", hash));;
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

	// number of Lua results
	Local<Array> returnValueArray = Array::New(args.GetIsolate());
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
					returnValueArray->Set(0, String::NewFromUtf8(args.GetIsolate(), str));
				}
				else
				{
					returnValueArray->Set(0, Null(args.GetIsolate()));
				}

				break;
			}
			case V8MetaFields::ResultAsFloat:
				returnValueArray->Set(0, Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&context.arguments[0])));
				break;
			case V8MetaFields::ResultAsVector:
			{
				scrVector vector = *reinterpret_cast<scrVector*>(&context.arguments[0]);

				Local<Array> vectorArray = Array::New(args.GetIsolate(), 3);
				vectorArray->Set(0, Number::New(args.GetIsolate(), vector.x));
				vectorArray->Set(1, Number::New(args.GetIsolate(), vector.y));
				vectorArray->Set(2, Number::New(args.GetIsolate(), vector.z));
				
				returnValueArray->Set(0, vectorArray);

				break;
			}
			case V8MetaFields::ResultAsInteger:
				returnValueArray->Set(0, Int32::New(args.GetIsolate(), *reinterpret_cast<int32_t*>(&context.arguments[0])));
				break;
			default:
			{
				int32_t integer = *reinterpret_cast<int32_t*>(&context.arguments[0]);

				if ((integer & 0xFFFFFFFF) == 0)
				{
					returnValueArray->Set(0, Boolean::New(args.GetIsolate(), false));
				}
				else
				{
					returnValueArray->Set(0, Int32::New(args.GetIsolate(), integer));
				}
			}
		}
	}

	// loop over the return value pointers
	{
		int i = 0;

		while (i < numReturnValues)
		{
			switch (rettypes[i])
			{
				case V8MetaFields::PointerValueInt:
					returnValueArray->Set(numResults, Int32::New(args.GetIsolate(), retvals[i]));
					i++;
					break;

				case V8MetaFields::PointerValueFloat:
					returnValueArray->Set(numResults, Number::New(args.GetIsolate(), *reinterpret_cast<float*>(&retvals[i])));
					i++;
					break;

				case V8MetaFields::PointerValueVector:
				{
					scrVector vector = *reinterpret_cast<scrVector*>(&retvals[i]);

					Local<Array> vectorArray = Array::New(args.GetIsolate(), 3);
					vectorArray->Set(0, Number::New(args.GetIsolate(), vector.x));
					vectorArray->Set(1, Number::New(args.GetIsolate(), vector.y));
					vectorArray->Set(2, Number::New(args.GetIsolate(), vector.z));

					returnValueArray->Set(numResults, vectorArray);

					i += 3;
					break;
				}
			}

			numResults++;
		}
	}

	// and set the return value(s)
	if (numResults == 1)
	{
		args.GetReturnValue().Set(returnValueArray->Get(0));
	}
	else
	{
		args.GetReturnValue().Set(returnValueArray);
	}
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
			
			auto& arg = args[0];

			if (MetaField == V8MetaFields::PointerValueFloat)
			{
				float value = static_cast<float>(arg->NumberValue());

				pointerField->value = *reinterpret_cast<uint32_t*>(&value);
			}
			else if (MetaField == V8MetaFields::PointerValueInt)
			{
				intptr_t value = arg->IntegerValue();

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
	CpuProfiler* profiler = args.GetIsolate()->GetCpuProfiler();

	profiler->StartProfiling((args.Length() == 0) ? String::Empty(args.GetIsolate()) : Local<String>::Cast(args[0]), true);
}

static void V8_StopProfiling(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	CpuProfiler* profiler = args.GetIsolate()->GetCpuProfiler();

	CpuProfile* profile = profiler->StopProfiling((args.Length() == 0) ? String::Empty(args.GetIsolate()) : Local<String>::Cast(args[0]));
		
	SYSTEMTIME systemTime;
	GetSystemTime(&systemTime);

	std::string jsonString = SaveProfileToString(profile);

	profile->Delete();

	{
		std::ofstream stream(MakeRelativeCitPath(va(L"v8-%04d%02d%02d-%02d%02d%02d.cpuprofile", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond)));
		stream << jsonString;
	}

	args.GetReturnValue().Set(JSON::Parse(String::NewFromUtf8(args.GetIsolate(), jsonString.c_str(), NewStringType::kNormal, jsonString.size()).ToLocalChecked()));
}

static std::pair<std::string, FunctionCallback> g_citizenFunctions[] =
{
	{ "setTickFunction", V8_SetTickFunction },
	{ "getTickCount", V8_GetTickCount },
	{ "invokeNative", V8_InvokeNative },
	{ "startProfiling", V8_StartProfiling },
	{ "stopProfiling", V8_StopProfiling },
	// metafields
	{ "pointerValueIntInitialized", V8_GetPointerField<V8MetaFields::PointerValueInt> },
	{ "pointerValueFloatInitialized", V8_GetPointerField<V8MetaFields::PointerValueFloat> },
	{ "pointerValueInt", V8_GetMetaField<V8MetaFields::PointerValueInt> },
	{ "pointerValueFloat", V8_GetMetaField<V8MetaFields::PointerValueFloat> },
	{ "pointerValueVector", V8_GetMetaField<V8MetaFields::PointerValueVector> },
	{ "returnResultAnyway", V8_GetMetaField<V8MetaFields::ReturnResultAnyway> },
	{ "resultAsInteger", V8_GetMetaField<V8MetaFields::ResultAsInteger> },
	{ "resultAsFloat", V8_GetMetaField<V8MetaFields::ResultAsFloat> },
	{ "resultAsString", V8_GetMetaField<V8MetaFields::ResultAsString> },
	{ "resultAsVector", V8_GetMetaField<V8MetaFields::ResultAsVector> },
};

static v8::Handle<v8::Value> Throw(v8::Isolate* isolate, const char* message) {
	return isolate->ThrowException(v8::String::NewFromUtf8(isolate, message));
}

static bool ReadFileData(const v8::FunctionCallbackInfo<v8::Value>& args, std::vector<char>* fileData)
{
	V8ScriptRuntime* runtime = GetScriptRuntimeFromArgs(args);
	OMPtr<IScriptHost> scriptHost = runtime->GetScriptHost();

	V8PushEnvironment pushed(runtime);
	String::Utf8Value filename(args[0]);

	OMPtr<fxIStream> stream;
	HRESULT hr = scriptHost->OpenHostFile(*filename, stream.GetAddressOf());

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

	Handle<String> str = String::NewFromUtf8(args.GetIsolate(), fileData.data(), String::kNormalString, fileData.size());
	args.GetReturnValue().Set(str);
}

struct DataAndPersistent {
	std::vector<char> data;
	Persistent<ArrayBuffer> handle;
};

static void ReadBufferWeakCallback(
	const v8::WeakCallbackInfo<DataAndPersistent>& data)
{
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
	data->handle.MarkIndependent();

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

	// create a scope to hold handles we use here
	Locker locker(GetV8Isolate());
	Isolate::Scope isolateScope(GetV8Isolate());
	HandleScope handleScope(GetV8Isolate());

	// create global state
	Local<ObjectTemplate> global = ObjectTemplate::New(GetV8Isolate());

	// add a 'print' function for testing
	global->Set(String::NewFromUtf8(GetV8Isolate(), "print", NewStringType::kNormal).ToLocalChecked(),
				FunctionTemplate::New(GetV8Isolate(), [] (const v8::FunctionCallbackInfo<v8::Value>& args)
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
			v8::String::Utf8Value str(args[i]);
			trace("%s", *str);
		}
		trace("\n");
	}));

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

	// set global IO functions
	for (auto& routine : g_globalFunctions)
	{
		context->Global()->Set(
			String::NewFromUtf8(GetV8Isolate(), routine.first.c_str(), NewStringType::kNormal).ToLocalChecked(),
			Function::New(GetV8Isolate(), routine.second, External::New(GetV8Isolate(), this)));
	}

	// set the 'window' variable to the global itself
	context->Global()->Set(String::NewFromUtf8(GetV8Isolate(), "window", NewStringType::kNormal).ToLocalChecked(), context->Global());

	// run system scripts
	result_t hr;

	if (FX_FAILED(hr = LoadSystemFile("citizen:/scripting/v8/main.js")))
	{
		return hr;
	}

	return FX_S_OK;
}

result_t V8ScriptRuntime::Destroy()
{
	m_tickRoutine.swap(std::function<void()>());
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
		TryCatch eh;
		Local<Script> script = Script::Compile(scriptText, fileName);

		if (script.IsEmpty())
		{
			String::Utf8Value str(eh.Exception());

			trace("Error parsing script %s in resource %s: %s\n", scriptFile, "TODO", *str);

			// TODO: change?
			return FX_E_INVALIDARG;
		}

		*outScript = script;
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

	return LoadFileInternal(stream, scriptFile, outScript);
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
		TryCatch eh;
		Local<Value> value = script->Run();

		if (value.IsEmpty())
		{
			String::Utf8Value str(eh.Exception());
			String::Utf8Value stack(eh.StackTrace());

			trace("Error loading script %s in resource %s: %s\nstack:\n%s\n", scriptName, "TODO", *str, *stack);

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

		m_tickRoutine();
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

	std::vector<char> m_nativesBlob;

	std::vector<char> m_snapshotBlob;

	std::unique_ptr<Platform> m_platform;

	std::unique_ptr<v8::ArrayBuffer::Allocator> m_arrayBufferAllocator;

	std::unique_ptr<V8Debugger> m_debugger;

public:
	V8ScriptGlobals();

	~V8ScriptGlobals();

	inline Isolate* GetIsolate()
	{
		if (Isolate::GetCurrent() != m_isolate)
		{
			m_isolate->Enter();
		}

		return m_isolate;
	}
};

static V8ScriptGlobals g_v8;

static Isolate* GetV8Isolate()
{
	return g_v8.GetIsolate();
}

V8ScriptGlobals::V8ScriptGlobals()
{
	// initialize startup data
	auto readBlob = [=] (const std::wstring& name, std::vector<char>& outBlob)
	{
		std::ifstream scuiFile(MakeRelativeCitPath(L"citizen/scripting/v8/" + name), std::ios::binary);

		outBlob.swap(std::vector<char>(std::istreambuf_iterator<char>(scuiFile), std::istreambuf_iterator<char>()));
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

	// initialize platform
	m_platform = std::make_unique<V8Platform>();
	V8::InitializePlatform(m_platform.get());

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

	// initialize global V8
	V8::Initialize();

	// create an array buffer allocator
	m_arrayBufferAllocator = std::make_unique<ArrayBufferAllocator>();

	// create an isolate
	Isolate::CreateParams params;
	params.array_buffer_allocator = m_arrayBufferAllocator.get();

	m_isolate = Isolate::New(params);
	m_isolate->SetFatalErrorHandler([] (const char* location, const char* message)
	{
		FatalError("V8 error at %s: %s", location, message);
	});

	// initialize the debugger
	m_debugger = std::unique_ptr<V8Debugger>(CreateDebugger(m_isolate));
}

V8ScriptGlobals::~V8ScriptGlobals()
{
	// clean up V8
	/*m_isolate->Dispose();

	V8::Dispose();
	V8::ShutdownPlatform();*/

	// actually don't, this deadlocks from a global destructor
}

// {9C268449-7AF4-4A3B-995A-3B1692E958AC}
FX_DEFINE_GUID(CLSID_V8ScriptRuntime,
			   0x9c268449, 0x7af4, 0x4a3b, 0x99, 0x5a, 0x3b, 0x16, 0x92, 0xe9, 0x58, 0xac);


FX_NEW_FACTORY(V8ScriptRuntime);

FX_IMPLEMENTS(CLSID_V8ScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_V8ScriptRuntime, IScriptFileHandlingRuntime);
}