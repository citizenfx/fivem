/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "fxNativeContext.h"

// scrString corresponds to a binary string: may contain null-terminators, i.e,
// lua_pushlstring != lua_pushstring, and/or non-UTF valid characters.
#define SCRSTRING_MAGIC_BINARY 0xFEED1212

#ifdef COMPILING_CITIZEN_SCRIPTING_CORE
#define CSCRC_EXPORT DLL_EXPORT
#else
#define CSCRC_EXPORT DLL_IMPORT
#endif

namespace fx::invoker
{
template<typename>
inline constexpr bool always_false_v = false;

enum class MetaField : uint8_t
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

struct ScrObject
{
	const char* data;
	uintptr_t length;
};

struct ScrString
{
	const char* str;
	size_t len;
	uint32_t magic;
};

struct ScrVector
{
	alignas(8) float x;
	alignas(8) float y;
	alignas(8) float z;

	ScrVector()
		: x(0.f), y(0.f), z(0.f)
	{
	}

	ScrVector(float x, float y, float z)
		: x(x), y(y), z(z)
	{
	}
};

struct PointerFieldEntry
{
	bool empty;
	uintptr_t value;
	PointerFieldEntry()
		: empty(true), value(0)
	{
	}
};

struct PointerField
{
	PointerFieldEntry data[64];
};

struct ArgumentType
{
	uint32_t Size : 30;
	uint32_t IsString : 1;
	uint32_t IsPointer : 1;
};

struct IsolatedBuffer
{
	size_t Size;
	uint8_t* SafeBuffer;
	uint8_t* NativeBuffer;
};

struct CSCRC_EXPORT ScriptNativeContext : fxNativeContext
{
	uintptr_t initialArguments[32];
	ArgumentType types[32];

	uint32_t pointerMask = 0;
	const uint32_t* typeInfo = nullptr;

	PointerField* pointerFields;

	int numReturnValues = 0; // return values and their types
	uintptr_t retvals[16];
	MetaField rettypes[16];
	MetaField returnValueCoercion = MetaField::Max; // coercion for the result value

	IsolatedBuffer isolatedBuffers[8];
	int numIsolatedBuffers = 0;

	static uint8_t s_metaFields[(size_t)MetaField::Max];

	ScriptNativeContext(uint64_t hash, PointerField* fields);
	ScriptNativeContext(const ScriptNativeContext&) = delete;
	ScriptNativeContext(ScriptNativeContext&&) = delete;

	virtual void DoSetError(const char* msg) = 0;

	template <typename... Args>
	bool SetError(std::string_view string, const Args&... args);

	bool PushRaw(uintptr_t value, ArgumentType type);

	template<typename T>
	bool Push(const T& value, size_t size = 0);

	bool PushReturnValue(MetaField field, bool zero);
	bool PushMetaPointer(uint8_t* ptr);

	template<typename Visitor>
	bool ProcessResults(Visitor&& visitor);

	template<typename Visitor>
	bool ProcessPrimaryResult(Visitor&& visitor);

	template<typename Visitor>
	bool ProcessExtraResults(Visitor&& visitor);

	bool PreInvoke();
	bool PostInvoke();

	bool CheckArguments();
	bool CheckResults();

	bool IsolatePointer(int index);
};

template<typename... Args>
inline bool ScriptNativeContext::SetError(std::string_view string, const Args&... args)
{
	DoSetError(vva(string, fmt::make_printf_args(args...)));

	return false;
}

template<typename T>
inline bool ScriptNativeContext::Push(const T& value, size_t size)
{
	using TVal = std::decay_t<decltype(value)>;

	uintptr_t raw = 0;

	if constexpr (std::is_pointer_v<TVal>)
	{
		raw = reinterpret_cast<uintptr_t>(value);
	}
	else if constexpr (std::is_integral_v<TVal>)
	{
		raw = (uintptr_t)(int64_t)value; // TODO: Limit native integers to 32 bits.
	}
	else if constexpr (std::is_same_v<TVal, float>)
	{
		raw = *reinterpret_cast<const uint32_t*>(&value);
	}
	else
	{
		static_assert(always_false_v<T>, "Invalid argument type");
	}

	ArgumentType type;
	type.IsPointer = std::is_pointer_v<TVal>;
	type.IsString = std::is_same_v<TVal, char*> || std::is_same_v<TVal, const char*>;
	type.Size = size;

	return PushRaw(raw, type);
}

template<typename Visitor>
inline bool ScriptNativeContext::ProcessResults(Visitor&& visitor)
{
	// if no other result was requested, or we need to return the result anyway, push the result
	if ((numReturnValues == 0) || (returnValueCoercion != MetaField::Max))
	{
		if (!ProcessPrimaryResult(visitor))
		{
			return false;
		}
	}

	if (!ProcessExtraResults(visitor))
	{
		return false;
	}

	return true;
}

template<typename Visitor>
inline bool ScriptNativeContext::ProcessPrimaryResult(Visitor&& visitor)
{
	// handle the type coercion
	switch (returnValueCoercion)
	{
		case MetaField::ResultAsString:
		{
			auto strString = reinterpret_cast<const ScrString*>(&arguments[0]);

			if (strString->str && strString->magic == SCRSTRING_MAGIC_BINARY)
			{
				return visitor(*strString);
			}
			else
			{
				return visitor(strString->str);
			}
		}

		case MetaField::ResultAsFloat:
		{
			return visitor(*reinterpret_cast<const float*>(&arguments[0]));
		}

		case MetaField::ResultAsVector:
		{
			return visitor(*reinterpret_cast<const ScrVector*>(&arguments[0]));
		}

		case MetaField::ResultAsObject:
		{
			return visitor(*reinterpret_cast<const ScrObject*>(&arguments[0]));
		}

		case MetaField::ResultAsInteger:
		{
			return visitor(*reinterpret_cast<const int32_t*>(&arguments[0]));
		}

		case MetaField::ResultAsLong:
		{
			return visitor(*reinterpret_cast<const int64_t*>(&arguments[0]));
		}

		default:
		{
			const int32_t integer = *reinterpret_cast<const int32_t*>(&arguments[0]);

			if (integer == 0)
			{
				return visitor(false);
			}
			else
			{
				return visitor(integer);
			}
		}
	}
}

template<typename Visitor>
inline bool ScriptNativeContext::ProcessExtraResults(Visitor&& visitor)
{
	// loop over the return value pointers
	for (int i = 0; i < numReturnValues;)
	{
		switch (rettypes[i])
		{
			case MetaField::PointerValueInt:
			{
				if (!visitor(*reinterpret_cast<const int32_t*>(&retvals[i])))
				{
					return false;
				}

				i++;
				break;
			}

			case MetaField::PointerValueFloat:
			{
				if (!visitor(*reinterpret_cast<const float*>(&retvals[i])))
				{
					return false;
				}

				i++;
				break;
			}

			case MetaField::PointerValueVector:
			{
				if (!visitor(*reinterpret_cast<const ScrVector*>(&retvals[i])))
				{
					return false;
				}

				i += 3;
				break;
			}

			default:
				continue;
		}
	}

	return true;
}

}
