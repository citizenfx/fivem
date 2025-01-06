/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "ScriptInvoker.h"

#include <ICoreGameInit.h>

#include <CL2LaunchMode.h>

#include <ScriptEngine.h>

#ifndef IS_FXSERVER
#include <scrEngine.h>
#include <ExceptionToModuleHelper.h>
#include <Error.h>
#endif

#ifndef _WIN32
#include <sys/mman.h>
#endif

#include <stdexcept>

// See codegen_out_pointer_args.lua for type info generation

#define PAS_FLAG_BLOCKED 0x80000000 // Completely block this native
#define PAS_FLAG_TRIVIAL 0x40000000 // No native arguments are unsafe
#define PAS_FLAG_UNSAFE 0x20000000 // Treat all the native arguments as unknown

#define PAS_ARG_POINTER 0x80000000 // This argument is a pointer
#define PAS_ARG_STRING 0x40000000 // This argument is a string
#define PAS_ARG_BUFFER 0x20000000 // This argument is a read-only buffer (and the next argument is its length)
#define PAS_ARG_BOUND 0x10000000 // This argument needs to be bounds checked (non pointer)
#define PAS_ARG_SIZE 0x0FFFFFFF // The size of this argument

// The return type of the function
#define PAS_RET_VOID 0
#define PAS_RET_INT 1
#define PAS_RET_FLOAT 2
#define PAS_RET_LONG 3
#define PAS_RET_VECTOR3 4
#define PAS_RET_STRING 5
#define PAS_RET_OBJECT 7

// We already have a hash, so do something slightly faster than the default hashing function
struct BitMixerHash
{
	size_t operator()(uint64_t value) const
	{
		// Mix the bits upwards, then fold the upper half back into the lower half.
		value *= 0x9E3779B97F4A7C13;

		return static_cast<size_t>(value ^ (value >> 32));
	}
};

static constexpr size_t g_IsolatedBufferSize = 7 * 4096;
static uint8_t* g_IsolatedBuffers[4];
static size_t g_IsolatedBufferIndex = 0;

static bool g_EnforceTypeInfo = false;

// Some existing/popular scripts don't properly follow the native types.
// For now, preserve compatibility by correcting the issues instead of erroring.
// TODO: Add a setting to enable this
static const bool g_StrictTypeInfo = false;

static std::unordered_map<uint64_t, const uint32_t*, BitMixerHash> g_NativeTypes;

extern "C" DLL_IMPORT uint64_t MapNative(uint64_t inNative);

static inline uint64_t SafeMapNative(uint64_t inNative)
{
#ifdef GTA_FIVE
	if (!launch::IsSDK())
	{
		return MapNative(inNative);
	}
#endif

	return inNative;
}

namespace pas
{
struct NativeTypeInfo
{
	uint64_t hash;
	const uint32_t* typeInfo;
};

#if defined(GTA_FIVE)
#include "NativeTypes.h"
#elif defined(IS_RDR3) && 0 // FIXME: RDR3 uses long for some text natives?
#include "NativeTypesRDR.h"
#elif defined(IS_FXSERVER)
#include "NativeTypesServer.h"
#endif
}

namespace fx::invoker
{
struct PointerField
{
	MetaField type;
	uintptr_t value[3];
};

static uint8_t s_metaFields[(size_t)MetaField::Max];
static PointerField s_pointerFields[128];
static size_t s_pointerFieldIndex = 0;

class ScriptNativeHandlerImpl : public ScriptNativeHandler
{
public:
	// FIXME: FXDK uses game builds, but only supports fx native handlers, so we need to support both

#ifndef IS_FXSERVER
	rage::scrEngine::NativeHandler scr_handler = nullptr;
#endif

	fx::TNativeHandler* fx_handler = nullptr;

	ScriptNativeHandlerImpl(uint64_t hash);

	void Invoke(void* arguments, void* results, size_t nargs) const;
};

static std::unordered_map<uint64_t, ScriptNativeHandlerImpl, BitMixerHash> s_cachedNativesMap;
std::vector<const ScriptNativeHandler*> ScriptNativeHandler::s_cachedNativesArray;

static fx::TNativeHandler s_invalidNativeHandler = [](ScriptContext&)
{
	if (g_StrictTypeInfo)
	{
		throw std::runtime_error("native does not exist");
	}
};

CSCRC_INLINE ScriptNativeHandler::ScriptNativeHandler(uint64_t hash)
	: hash(hash)
{
	auto typeInfo = g_NativeTypes.find(SafeMapNative(hash));
	type = (typeInfo != g_NativeTypes.end()) ? typeInfo->second : nullptr;

	cache_index = s_cachedNativesArray.size();
	s_cachedNativesArray.emplace_back(this);
}

CSCRC_INLINE ScriptNativeHandlerImpl::ScriptNativeHandlerImpl(uint64_t hash)
	: ScriptNativeHandler(hash)
{
#ifndef IS_FXSERVER
	if (!launch::IsSDK() && (scr_handler = rage::scrEngine::GetNativeHandler(hash)))
	{
	}
	else
#endif
	if (fx_handler = fx::ScriptEngine::GetNativeHandlerPtr(hash))
	{
	}
	else
	{
		fx_handler = &s_invalidNativeHandler;
	}
}

const ScriptNativeHandler& ScriptNativeHandler::FromHash(uint64_t hash)
{
	auto find = s_cachedNativesMap.find(hash);

	if (find == s_cachedNativesMap.end())
	{
		find = s_cachedNativesMap.emplace_hint(find, hash, hash);
	}

	return find->second;
}

#ifndef IS_FXSERVER
static LONG FilterFunc(PEXCEPTION_POINTERS ep, uint64_t nativeIdentifier)
{
	if (IsErrorException(ep))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// C++ exceptions?
	if (ep->ExceptionRecord->ExceptionCode == 0xE06D7363)
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// INVOKE_FUNCTION_REFERENCE crashing as top-level is usually related to native state corruption,
	// we'll likely want to crash on this instead rather than on an assertion down the chain
	if (nativeIdentifier == HashString("INVOKE_FUNCTION_REFERENCE"))
	{
		return EXCEPTION_CONTINUE_SEARCH;
	}

	return EXCEPTION_EXECUTE_HANDLER;
}
#endif

// Note: MSVC cannot inline functions which use try/__try, or use both try and __try in the same function.
void ScriptNativeHandlerImpl::Invoke(void* arguments, void* results, size_t nargs) const
{
#ifndef IS_FXSERVER
	if (scr_handler)
	{
		NativeContextRaw context(arguments, results, static_cast<int>(nargs));
		void* exceptionAddress = nullptr;

		__try
		{
			scr_handler(&context);
		}
		__except (exceptionAddress = GetExceptionInformation()->ExceptionRecord->ExceptionAddress, FilterFunc(GetExceptionInformation(), hash))
		{
			throw std::runtime_error(va("exception at address %s", FormatModuleAddress(exceptionAddress)));
		}

		context.SetVectorResults();
	}
	else
#endif
	{
		ScriptContextRaw context(arguments, results, static_cast<int>(nargs));
		(*fx_handler)(context);
	}
}

void* ScriptNativeContext::AllocIsolatedData(const void* input, size_t size)
{
	size_t asize = (size + 7) & ~size_t(7);

	if (isolatedBuffer == nullptr)
	{
		isolatedBuffer = g_IsolatedBuffers[g_IsolatedBufferIndex];
		isolatedBufferEnd = isolatedBuffer + g_IsolatedBufferSize;
		g_IsolatedBufferIndex = (g_IsolatedBufferIndex + 1) % std::size(g_IsolatedBuffers);
	}

	if (asize > (size_t)(isolatedBufferEnd - isolatedBuffer))
	{
		ScriptError("too much isolated data");
	}

	uint8_t* result = isolatedBuffer;
	isolatedBuffer += asize;

	if (input)
	{
		std::memcpy(result, input, size);
	}
	else
	{
		std::memset(result, 0, size);
	}

	return result;
}

void ScriptNativeContext::PushReturnValue(MetaField field, const uintptr_t* value)
{
	if (numReturnValues >= std::size(rettypes))
	{
		ScriptError("too many return value arguments");
	}

	int slots = (field == MetaField::PointerValueVector) ? 3 : 1;
	size_t size = slots * sizeof(uintptr_t);

	// COMPAT: Some code uses PointerValue arguments to act as struct output fields, so we need them to be stored contiguously in memory.
	void* data = AllocIsolatedData(value, size);

	retvals[numReturnValues] = data;
	rettypes[numReturnValues] = field;
	++numReturnValues;

	ArgumentType type{};
	type.Size = size;
	type.IsIsolated = true;
	type.IsString = false;
	type.IsPointer = true;

	PushRaw(ReserveArgs(1), reinterpret_cast<uintptr_t>(data), type);
}

void ScriptNativeContext::PushMetaPointer(uint8_t* ptr)
{
	// if the pointer is a metafield
	if (ptr >= s_metaFields && ptr < &s_metaFields[(int)MetaField::Max])
	{
		MetaField metaField = static_cast<MetaField>(ptr - s_metaFields);

		// switch on the metafield
		switch (metaField)
		{
			case MetaField::PointerValueInteger:
			case MetaField::PointerValueFloat:
			case MetaField::PointerValueVector:
			{
				PushReturnValue(metaField, nullptr);
				break;
			}

			case MetaField::ReturnResultAnyway:
				// We are going to return the result anyway if they've given us the type.
				if (rettypes[0] == MetaField::Max)
					rettypes[0] = metaField;
				break;
			case MetaField::ResultAsInteger:
			case MetaField::ResultAsLong:
			case MetaField::ResultAsString:
			case MetaField::ResultAsFloat:
			case MetaField::ResultAsVector:
			case MetaField::ResultAsObject:
				rettypes[0] = metaField;
				break;
			default:
				break;
		}
	}
	// or if the pointer is a runtime pointer field
	else if (ptr >= reinterpret_cast<uint8_t*>(s_pointerFields) && ptr < (reinterpret_cast<uint8_t*>(s_pointerFields + std::size(s_pointerFields))))
	{
		PointerField* field = reinterpret_cast<PointerField*>(ptr);

		if (field->type == MetaField::PointerValueInteger || field->type == MetaField::PointerValueFloat || field->type == MetaField::PointerValueVector)
		{
			PushReturnValue(field->type, field->value);
		}
	}
	else
	{
		ScriptError("unknown userdata pointer");
	}
}

CSCRC_INLINE bool ScriptNativeContext::PreInvoke()
{
	if (!CheckArguments())
	{
		return false;
	}

	// This native might have new arguments added that we don't know about, so zero some extra arguments
	for (size_t i = numArguments, j = std::min(i + 3, std::size(arguments)); i < j; ++i)
	{
		arguments[i] = 0;
	}

	return true;
}

void ScriptNativeContext::Invoke()
{
	if (PreInvoke())
	{
		try
		{
			static_cast<const ScriptNativeHandlerImpl&>(info).Invoke(arguments, results, numArguments);
		}
		catch (const std::exception& e)
		{
			ScriptError(e.what());
		}

		PostInvoke();
	}
}

CSCRC_INLINE void ScriptNativeContext::PostInvoke()
{
	CheckResults();

	// Copy any data back out
	for (int i = 0; i < numIsolatedBuffers; ++i)
	{
		auto& buffer = isolatedBuffers[i];

		std::memcpy(buffer.NativeBuffer, buffer.SafeBuffer, buffer.Size);
	}
}

CSCRC_INLINE bool ScriptNativeContext::CheckArguments()
{
	uint32_t nargs = 0;
	uint32_t nvalid = 0;

	auto typeInfo = info.type;

	if (typeInfo)
	{
		uint32_t info = typeInfo[0];

		if ((info & PAS_FLAG_BLOCKED) && g_EnforceTypeInfo)
		{
			ScriptError("native is blocked");
		}

		nargs = info & 0xFF;

		// Fast path for natives which don't expect any pointers, and none were passed in.
		if ((info & PAS_FLAG_TRIVIAL) && trivial)
		{
			if (nargs == numArguments)
			{
				// We've been given the correct number of arguments, and none are pointers (nor should they be).
				return true;
			}

			// Don't bother checking arguments we already know about, since they aren't pointers.
			nvalid = nargs;
		}

		// Some natives have extra arguments we don't know about, so don't be too strict
		if (numArguments < nargs)
		{
			if (g_StrictTypeInfo)
			{
				ScriptErrorf("not enough arguments (%i < %i)", numArguments, nargs);
			}

			// COMPAT: Some code doesn't pass in enough arguments, so just fill in any missing ones with zero
			for (size_t i = numArguments; i < nargs; ++i)
			{
				Push(0);
			}
		}

		if (info & PAS_FLAG_UNSAFE)
		{
			// Treat all arguments as unknown
			nargs = 0;
			nvalid = 0;
		}
	}
	else if (g_EnforceTypeInfo)
	{
		// TODO: Nullify any unknown natives instead?
		nargs = 0;
	}
	else
	{
		return true;
	}

	for (uint32_t i = nvalid; i < nargs; ++i)
	{
		ArgumentType type = types[i];
		uint32_t expected = typeInfo[i + 1];

		if (!(expected & PAS_ARG_POINTER)) // Argument should not be a pointer
		{
			if (type.IsPointer)
			{
				if (g_StrictTypeInfo)
				{
					ScriptErrorf("arg[%i]: expected non-pointer", i);
				}

				// COMPAT: We aren't expecting a pointer, but got one. Just replace it with a dummy value.
				if (arguments[i] != 0)
				{
					arguments[i] = 0xDEADBEEF7FEDCAFE;
					types[i].IsPointer = false;
				}
			}

			if (expected & PAS_ARG_BOUND)
			{
				uint32_t maxValue = expected & PAS_ARG_SIZE;

				if (arguments[i] > maxValue)
				{
					if (g_StrictTypeInfo)
					{
						ScriptErrorf("arg[%i]: value too large (%i > %i)", i, arguments[i], maxValue);
					}

					// COMPAT: Silently fail
					return false;
				}
			}

			continue;
		}

		if (!type.IsPointer)
		{
			if (arguments[i] != 0) // Argument should be a pointer, but also allow NULL
			{
				ScriptErrorf("arg[%i]: expected pointer, got non-zero integer", i);
			}

			continue;
		}

		if (expected & PAS_ARG_STRING) // Argument should be a string
		{
			if (!type.IsString)
			{
				ScriptErrorf("arg[%i]: expected string", i);
			}

			continue;
		}

		uint32_t minSize = expected & PAS_ARG_SIZE;

		if (type.Size < minSize)
		{
			ScriptErrorf("arg[%i]: buffer too small (%u < %u)", i, type.Size, minSize);
		}

		if (minSize != 0) // We know how large this argument should be, so don't bother isolating it.
		{
			// TODO: Don't trust the pointer size for now. There are too many natives with incorrect pointer types.
			// continue;
		}

		if (expected & PAS_ARG_BUFFER) // Argument is a read-only buffer, followed by its length
		{
			size_t length = arguments[i + 1];

			if (length > type.Size)
			{
				ScriptErrorf("arg[%i]: buffer length too large (%u > %u)", i, length, type.Size);
			}

			continue;
		}

		// We're not really sure how large this pointer should be, so isolate it.
		IsolatePointer(i);
	}

	// Process any unknown arguments which might have been added in later versions, or this is an unknown native.
	for (uint32_t i = nargs; i < numArguments; ++i)
	{
		if (types[i].IsPointer)
		{
			// We have no idea how this pointer might be used, so isolate it.
			IsolatePointer(i);
		}
		else
		{
			// If the argument isn't 0 (might be a NULL pointer), replace the upper bits so it's never a valid pointer.
			if (arguments[i] != 0)
			{
				arguments[i] = 0xDEADBEEF00000000 | (arguments[i] & 0xFFFFFFFF);
			}
		}
	}

	return true;
}

CSCRC_INLINE void ScriptNativeContext::CheckResults()
{
	auto typeInfo = info.type;

	if (!typeInfo)
	{
		if (g_EnforceTypeInfo)
		{
			// Block undocumented usage of long/ScrObject
			switch (rettypes[0])
			{
				case MetaField::ResultAsLong:
				{
					// Hopefully just an undocumented scrBind native, so zero the upper bits
					results[0] &= 0xFFFFFFFF;

					break;
				}

				case MetaField::ResultAsObject:
				{
					ScriptError("undocumented natives cannot return an object");
				}
			}
		}

		return;
	}

	uint32_t rtype = (typeInfo[0] >> 8) & 0xFF;

	switch (rettypes[0])
	{
		case MetaField::ResultAsString:
		{
			if (rtype != PAS_RET_STRING)
			{
				ScriptError("result type is not a string");
			}

			break;
		}

		case MetaField::ResultAsVector:
		{
			if (rtype != PAS_RET_VECTOR3)
			{
				ScriptError("result type is not a vector");
			}

			break;
		}

		case MetaField::ResultAsObject:
		{
			if (rtype != PAS_RET_OBJECT)
			{
				ScriptError("result type is not an object");
			}

			break;
		}

		default:
		{
			if (rtype != PAS_RET_INT && rtype != PAS_RET_FLOAT && rtype != PAS_RET_LONG)
			{
				if (results[0] != 0) // Preserve zero/false
				{
					results[0] = 0xDEADBEEF7FEDCAFE; // Lower 32 bits are NaN
				}
			}

			break;
		}
	}
}

void ScriptNativeContext::IsolatePointer(int index)
{
	ArgumentType& type = types[index];

	if (!type.IsPointer)
	{
		ScriptErrorf("arg[%i]: is not a pointer", index);
	}

	if (type.IsIsolated)
	{
		return;
	}

	if (numIsolatedBuffers >= std::size(isolatedBuffers))
	{
		ScriptError("too many unknown pointers");
	}

	uint8_t*& argument = *reinterpret_cast<uint8_t**>(&arguments[index]);
	uint8_t* storage = (uint8_t*)AllocIsolatedData(argument, (size_t)type.Size + (type.IsString ? 1 : 0));

	auto& buffer = isolatedBuffers[numIsolatedBuffers++];
	buffer.Size = type.Size;
	buffer.NativeBuffer = std::exchange(argument, storage);
	buffer.SafeBuffer = storage;

	type.IsIsolated = true;
}

void ScriptNativeContext::ScriptError(const char* error) const
{
	const char* msg = va("native %016llx: %s", info.hash, error);
	trace("script error in %s\n", msg);
	throw std::runtime_error(msg);
}

void* ScriptNativeContext::GetMetaField(MetaField field)
{
	return &s_metaFields[static_cast<int>(field)];
}

void* ScriptNativeContext::GetPointerField(MetaField type, uintptr_t value)
{
	assert(type == MetaField::PointerValueInteger || type == MetaField::PointerValueFloat || type == MetaField::PointerValueVector);

	PointerField* entry = &s_pointerFields[s_pointerFieldIndex];
	s_pointerFieldIndex = (s_pointerFieldIndex + 1) % std::size(s_pointerFields);

	entry->type = type;
	entry->value[0] = value;
	entry->value[1] = value; // TODO: Add full support for vector fields?
	entry->value[2] = value;

	return entry;
}
}

static InitFunction initFunction([]
{
#ifdef HAVE_NATIVE_TYPES
	const auto load_types = []
	{
		for (const auto& type : pas::native_types)
		{
			g_NativeTypes[SafeMapNative(type.hash)] = type.typeInfo;
		}

		g_EnforceTypeInfo = true;

#ifndef IS_FXSERVER
		Instance<ICoreGameInit>::Get()->OnSetVariable.Connect([](const std::string& name, bool value)
		{
			if (name == "storyMode")
			{
				g_EnforceTypeInfo = !value;
			}
		});
#endif
	};

#ifdef IS_FXSERVER
	load_types();
#else
	rage::scrEngine::OnScriptInit.Connect(load_types);
#endif
#endif

	size_t buffer_size = g_IsolatedBufferSize + 4096;
	size_t total_size = buffer_size * std::size(g_IsolatedBuffers);

	uint8_t* region =
#ifdef _WIN32
	(uint8_t*)VirtualAlloc(NULL, total_size, MEM_COMMIT, PAGE_NOACCESS);
#else
	(uint8_t*)mmap(NULL, total_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif

	assert(region);

	for (auto& buffer : g_IsolatedBuffers)
	{
		buffer = region;
		region += buffer_size;

#ifdef _WIN32
		DWORD oldProtect;
		VirtualProtect(buffer, g_IsolatedBufferSize, PAGE_READWRITE, &oldProtect);
#else
		mprotect(buffer, g_IsolatedBufferSize, PROT_READ | PROT_WRITE);
#endif
	}
});
