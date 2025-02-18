/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace fx::v8shared
{
	template<MetaField field>
	static void V8_GetMetaField(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		args.GetReturnValue().Set(v8::External::New(args.GetIsolate(), ScriptNativeContext::GetMetaField(field)));
	}

	template<MetaField field, class RuntimeType>
	static void V8_GetPointerField(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);

		auto arg = args[0];
		uintptr_t value = 0;

		if constexpr (field == MetaField::PointerValueInteger)
		{
			value = (uint64_t) arg->IntegerValue(runtime->GetContext()).ToChecked();
		}
		else if constexpr (field == MetaField::PointerValueFloat)
		{
			float fvalue = static_cast<float>(arg->NumberValue(runtime->GetContext()).ToChecked());
			value = *reinterpret_cast<uint32_t*>(&value);
		}

		args.GetReturnValue().Set(v8::External::New(args.GetIsolate(), ScriptNativeContext::GetPointerField(field, value)));
	}
}
