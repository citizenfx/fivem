/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "RuntimeHelpers.h"

namespace fx::v8shared
{
	static v8::Handle<v8::Value> Throw(v8::Isolate* isolate, const char* message) {
		return isolate->ThrowException(v8::String::NewFromUtf8(isolate, message).ToLocalChecked());
	}

	template<class RuntimeType, class PushType>
	static bool ReadFileData(const v8::FunctionCallbackInfo<v8::Value>& args, std::vector<char>* fileData)
	{
		auto runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);
		OMPtr<IScriptHost> scriptHost = runtime->GetScriptHost();

		PushType pushed(runtime);
		v8::String::Utf8Value filename(args.GetIsolate(), args[0]);

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

	template<class RuntimeType, class PushType>
	static void V8_Read(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		std::vector<char> fileData;

		if (!ReadFileData<RuntimeType, PushType>(args, &fileData))
		{
			return;
		}

		v8::Handle<v8::String> str = v8::String::NewFromUtf8(args.GetIsolate(), fileData.data(), v8::NewStringType::kNormal, fileData.size()).ToLocalChecked();
		args.GetReturnValue().Set(str);
	}

	struct DataAndPersistent
	{
		std::vector<char> data;
		int byte_length;
		v8::Global<v8::ArrayBuffer> handle;
	};

	static void ReadBufferWeakCallback(const v8::WeakCallbackInfo<DataAndPersistent>& data)
	{
		int byte_length = data.GetParameter()->byte_length;
		data.GetIsolate()->AdjustAmountOfExternalAllocatedMemory(-static_cast<intptr_t>(byte_length));

		data.GetParameter()->handle.Reset();
		delete data.GetParameter();
	}

	template<class RuntimeType, class PushType>
	static void V8_ReadBuffer(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		std::vector<char> fileData;

		if (!ReadFileData<RuntimeType, PushType>(args, &fileData))
		{
			return;
		}

		auto isolate = args.GetIsolate();

		v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(isolate, fileData.size());
		auto abs = buffer->GetBackingStore();
		memcpy(abs->Data(), fileData.data(), fileData.size());

		args.GetReturnValue().Set(buffer);
	}

}
