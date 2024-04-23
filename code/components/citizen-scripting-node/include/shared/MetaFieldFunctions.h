/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace fx::v8shared
{
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

	template<V8MetaFields MetaField>
	static void V8_GetMetaField(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		args.GetReturnValue().Set(v8::External::New(args.GetIsolate(), &g_metaFields[(int)MetaField]));
	}

	template<V8MetaFields MetaField, class RuntimeType>
	static void V8_GetPointerField(const v8::FunctionCallbackInfo<v8::Value>& args)
	{
		RuntimeType* runtime = GetScriptRuntimeFromArgs<RuntimeType>(args);

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

		args.GetReturnValue().Set(v8::External::New(args.GetIsolate(), (pointerField) ? static_cast<void*>(pointerField) : &dummyOut));
	}
}
