/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_CITIZEN_SCRIPTING_CORE
#define CSCRC_EXPORT DLL_EXPORT
#else
#define CSCRC_EXPORT DLL_IMPORT
#endif

#if defined(_MSC_VER)
#define CSCRC_INLINE inline __forceinline
#elif __has_attribute(__always_inline__)
#define CSCRC_INLINE inline __attribute__((__always_inline__))
#else
#define CSCRC_INLINE inline
#endif

namespace fx::invoker
{
template<typename>
inline constexpr bool always_false_v = false;

enum class MetaField : uint8_t
{
	PointerValueInteger,
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

// scrString corresponds to a binary string: may contain null-terminators, i.e,
// lua_pushlstring != lua_pushstring, and/or non-UTF valid characters.
struct ScrString
{
	const char* str;
	size_t len;
	uint32_t magic;

	static const uint32_t Signature = 0xFEED1212;
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

class ScriptNativeHandler
{
public:
	uint64_t hash = 0;
	const uint32_t* type = nullptr;
	size_t cache_index = SIZE_MAX;

	ScriptNativeHandler(const ScriptNativeHandler&) = delete;
	ScriptNativeHandler& operator=(const ScriptNativeHandler&) = delete;

	CSCRC_EXPORT static const ScriptNativeHandler& FromHash(uint64_t hash);
	CSCRC_EXPORT static const ScriptNativeHandler& FromCacheIndex(size_t index);

protected:
	ScriptNativeHandler(uint64_t hash);

	CSCRC_EXPORT static std::vector<const ScriptNativeHandler*> s_cachedNativesArray;
};

class ScriptNativeContext
{
public:
	ScriptNativeContext(uint64_t hash);
	ScriptNativeContext(const ScriptNativeHandler& handler);

	ScriptNativeContext(const ScriptNativeContext&) = delete;
	ScriptNativeContext(ScriptNativeContext&&) = delete;

	size_t ReserveArgs(size_t nargs) const;

	template<typename T>
	void Push(T value);

	template<typename T>
	void Push(T* value, size_t size);

	// Does NOT check for overflow (use ReserveArgs)
	template<typename T>
	void PushAt(size_t index, T value);

	// Does NOT check for overflow (use ReserveArgs)
	template<typename T>
	void PushAt(size_t index, T* value, size_t size);

	CSCRC_EXPORT void PushMetaPointer(uint8_t* ptr);

	CSCRC_EXPORT void Invoke();

	template<typename Visitor>
	void ProcessResults(Visitor&& visitor) const;

	[[noreturn]] CSCRC_EXPORT void ScriptError(const char* error) const;

	template<typename... Args>
	[[noreturn]] void ScriptErrorf(const char* format, const Args&... args) const;

	CSCRC_EXPORT static void* GetMetaField(MetaField field);
	CSCRC_EXPORT static void* GetPointerField(MetaField type, uintptr_t value);

private:
	const ScriptNativeHandler& info;

	// Argument values and types
	uintptr_t arguments[32];
	ArgumentType types[32];

	// Numver of arguments
	size_t numArguments = 0;

	// Are all arguments trivial (no pointers)?
	bool trivial = true;

	// Our allocated isolated buffer region
	uint8_t* isolatedBuffer = nullptr;
	uint8_t* isolatedBufferEnd = nullptr;

	// Primary result value
	uintptr_t results[4] = {};

	// Addresses and types of values to return
	void* retvals[16];
	MetaField rettypes[16];
	size_t numReturnValues = 0;

	// Isolated buffers
	IsolatedBuffer isolatedBuffers[8];
	size_t numIsolatedBuffers = 0;

	void PushRaw(size_t index, uintptr_t value, ArgumentType type);

	void* AllocIsolatedData(const void* input, size_t size);

	void PushReturnValue(MetaField field, const uintptr_t* value);

	template<typename Visitor>
	void ProcessResult(Visitor&& visitor, const void* value, MetaField type) const;

	bool PreInvoke();
	void PostInvoke();

	bool CheckArguments();
	void CheckResults();

	void IsolatePointer(int index);
};

CSCRC_INLINE const ScriptNativeHandler& ScriptNativeHandler::FromCacheIndex(size_t index)
{
	return *s_cachedNativesArray.at(index);
}

CSCRC_INLINE ScriptNativeContext::ScriptNativeContext(uint64_t hash)
	: ScriptNativeContext(ScriptNativeHandler::FromHash(hash))
{
}

CSCRC_INLINE ScriptNativeContext::ScriptNativeContext(const ScriptNativeHandler& handler)
	: info(handler)
{
	retvals[0] = results;
	rettypes[0] = MetaField::Max;
	numReturnValues = 1;
}

template<typename... Args>
inline void ScriptNativeContext::ScriptErrorf(const char* format, const Args&... args) const
{
	ScriptError(vva(format, fmt::make_printf_args(args...)));
}

CSCRC_INLINE size_t ScriptNativeContext::ReserveArgs(size_t nargs) const
{
	size_t index = numArguments;
	size_t space = std::size(arguments) - index;

	if (space < nargs)
	{
		ScriptError("too many arguments");
	}

	return index;
}

CSCRC_INLINE void ScriptNativeContext::PushRaw(size_t index, uintptr_t value, ArgumentType type)
{
	arguments[index] = value;
	types[index] = type;

	if (type.IsPointer)
	{
		trivial = false;
	}

	numArguments = index + 1;
}

template<typename T>
CSCRC_INLINE void ScriptNativeContext::Push(T value)
{
	PushAt(ReserveArgs(1), value);
}

template<typename T>
CSCRC_INLINE void ScriptNativeContext::Push(T* value, size_t size)
{
	PushAt(ReserveArgs(1), value, size);
}

template<typename T>
CSCRC_INLINE void ScriptNativeContext::PushAt(size_t index, T value)
{
	using TVal = std::decay_t<T>;

	uintptr_t raw = 0;

	if constexpr (std::is_integral_v<TVal>)
	{
		raw = (uintptr_t)(int64_t)value; // TODO: Limit native integers to 32 bits.
	}
	else if constexpr (std::is_floating_point_v<TVal>)
	{
		float fvalue = static_cast<float>(value);
		raw = *reinterpret_cast<const uint32_t*>(&fvalue);
	}
	else
	{
		static_assert(always_false_v<T>, "Invalid argument type");
	}

	PushRaw(index, raw, {});
}

template<typename T>
CSCRC_INLINE void ScriptNativeContext::PushAt(size_t index, T* value, size_t size)
{
	uintptr_t raw = reinterpret_cast<uintptr_t>(value);

	ArgumentType type{};

	type.Size = size;
	type.IsIsolated = false;
	type.IsString = std::is_same_v<std::remove_const_t<T>, char>;
	type.IsPointer = true;

	PushRaw(index, raw, type);
}

template<typename Visitor>
CSCRC_INLINE void ScriptNativeContext::ProcessResults(Visitor&& visitor) const
{
	// if no other result was requested, or we need to return the result anyway, push the primary result
	int i = ((numReturnValues == 1) || (rettypes[0] != MetaField::Max)) ? 0 : 1;

	// loop over the return value pointers
	for (; i < numReturnValues; ++i)
	{
		ProcessResult(visitor, retvals[i], rettypes[i]);
	}
}

template<typename Visitor>
CSCRC_INLINE void ScriptNativeContext::ProcessResult(Visitor&& visitor, const void* value, MetaField type) const
{
	// handle the type coercion
	switch (type)
	{
		case MetaField::ResultAsString:
		{
			auto strString = static_cast<const ScrString*>(value);

			if (strString->str && strString->magic == ScrString::Signature)
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
		case MetaField::PointerValueInteger:
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
