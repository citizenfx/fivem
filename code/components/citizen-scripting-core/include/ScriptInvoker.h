/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "fxNativeContext.h"

#include "fxScripting.h"

#include <stdexcept>

#ifndef IS_FXSERVER
#include <scrEngine.h>
#endif

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
	uint32_t Size : 29;
	uint32_t IsIsolated : 1;
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

	uint8_t* isolatedBuffer = nullptr;
	uint8_t* isolatedBufferEnd = nullptr;

	void* retvals[16];
	MetaField rettypes[16];
	int numReturnValues = 0; // return values and their types
	MetaField returnValueCoercion = MetaField::Max; // coercion for the result value

	IsolatedBuffer isolatedBuffers[8];
	int numIsolatedBuffers = 0;

	static uint8_t s_metaFields[(size_t)MetaField::Max];

	ScriptNativeContext(uint64_t hash, PointerField* fields);
	ScriptNativeContext(const ScriptNativeContext&) = delete;
	ScriptNativeContext(ScriptNativeContext&&) = delete;

	template<typename... Args>
	[[nodiscard]] auto ScriptError(std::string_view string, const Args&... args);

	template<typename T>
	void Push(const T& value, size_t size = 0);

	void PushMetaPointer(uint8_t* ptr);

	void Invoke(IScriptHost& host);

#ifndef IS_FXSERVER
	void Invoke(rage::scrEngine::NativeHandler handler);
#endif

	template<typename Visitor>
	void ProcessResults(Visitor&& visitor);

private:
	void PushRaw(uintptr_t value, ArgumentType type);

	void* AllocIsolatedData(const void* input, size_t size);

	void PushReturnValue(MetaField field, const uintptr_t* value);

	template<typename Visitor>
	void ProcessResult(Visitor&& visitor, const void* value, MetaField type);

	bool PreInvoke();
	void PostInvoke();

	bool CheckArguments();
	void CheckResults();
	void CheckPointerResult();

	void IsolatePointer(int index);
};

template<typename... Args>
inline auto ScriptNativeContext::ScriptError(std::string_view string, const Args&... args)
{
	return std::runtime_error(va("native %016llx: %s", nativeIdentifier, vva(string, fmt::make_printf_args(args...))));
}

inline void ScriptNativeContext::PushRaw(uintptr_t value, ArgumentType type)
{
	if (numArguments >= 32)
	{
		throw ScriptError("too many arguments");
	}

	arguments[numArguments] = value;
	types[numArguments] = type;
	pointerMask |= (uint32_t)type.IsPointer << numArguments;

	++numArguments;
}

template<typename T>
inline void ScriptNativeContext::Push(const T& value, size_t size)
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

	ArgumentType type{};
	type.Size = size;
	type.IsIsolated = false;
	type.IsString = std::is_same_v<TVal, char*> || std::is_same_v<TVal, const char*>;
	type.IsPointer = std::is_pointer_v<TVal>;

	PushRaw(raw, type);
}

template<typename Visitor>
inline void ScriptNativeContext::ProcessResults(Visitor&& visitor)
{
	// if no other result was requested, or we need to return the result anyway, push the primary result
	if ((numReturnValues == 0) || (returnValueCoercion != MetaField::Max))
	{
		ProcessResult(visitor, arguments, returnValueCoercion);
	}

	// loop over the return value pointers
	for (int i = 0; i < numReturnValues; ++i)
	{
		ProcessResult(visitor, retvals[i], rettypes[i]);
	}
}

template<typename Visitor>
inline void ScriptNativeContext::ProcessResult(Visitor&& visitor, const void* value, MetaField type)
{
	// handle the type coercion
	switch (type)
	{
		case MetaField::ResultAsString:
		{
			auto strString = static_cast<const ScrString*>(value);

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
		case MetaField::PointerValueFloat:
		{
			return visitor(*static_cast<const float*>(value));
		}

		case MetaField::ResultAsVector:
		case MetaField::PointerValueVector:
		{
			return visitor(*static_cast<const ScrVector*>(value));
		}

		case MetaField::ResultAsObject:
		{
			return visitor(*static_cast<const ScrObject*>(value));
		}

		case MetaField::ResultAsInteger:
		case MetaField::PointerValueInt:
		{
			return visitor(*static_cast<const int32_t*>(value));
		}

		case MetaField::ResultAsLong:
		{
			return visitor(*static_cast<const int64_t*>(value));
		}

		default:
		{
			const int32_t integer = *static_cast<const int32_t*>(value);

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

}
