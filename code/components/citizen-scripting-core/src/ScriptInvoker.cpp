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

#ifndef IS_FXSERVER
#include <ExceptionToModuleHelper.h>
#include <Error.h>
#endif

#ifndef _WIN32
#include <sys/mman.h>
#endif

// See codegen_out_pointer_args.lua for type info generation

#define PAS_FLAG_BLOCKED 0x80000000 // Completely block this native
#define PAS_FLAG_TRIVIAL 0x40000000 // No native arguments are unsafe
#define PAS_FLAG_UNSAFE  0x20000000 // Treat all the native arguments as unknown

#define PAS_ARG_POINTER 0x80000000 // This argument is a pointer
#define PAS_ARG_STRING  0x40000000 // This argument is a string
#define PAS_ARG_BUFFER  0x20000000 // This argument is a read-only buffer (and the next argument is its length)
#define PAS_ARG_BOUND	0x10000000 // This argument needs to be bounds checked (non pointer)
#define PAS_ARG_SIZE	0x0FFFFFFF // The size of this argument

// The return type of the function
#define PAS_RET_VOID 0
#define PAS_RET_INT 1
#define PAS_RET_FLOAT 2
#define PAS_RET_LONG 3
#define PAS_RET_VECTOR3 4
#define PAS_RET_STRING 5
#define PAS_RET_SCRSTRING 6
#define PAS_RET_SCROBJECT 7

static constexpr size_t g_IsolatedBufferSize = 7 * 4096;
static uint8_t* g_IsolatedBuffers[4];
static size_t g_IsolatedBufferIndex = 0;

static bool g_EnableTypeInfo = false;
static bool g_EnforceTypeInfo = false;

// Some existing/popular scripts don't properly follow the native types.
// For now, preserve compatibility by correcting the issues instead of erroring.
// TODO: Add a setting to enable this
static bool g_StrictTypeInfo = false;

static std::unordered_map<uint64_t, const uint32_t*> g_NativeTypes;

#ifdef GTA_FIVE
extern "C" DLL_IMPORT uint64_t MapNative(uint64_t inNative);
#else
static uint64_t MapNative(uint64_t inNative)
{
	return inNative;
}
#endif

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
#elif defined(IS_FXSERVER) && 0
#include "NativeTypesServer.h"
#endif
}

namespace fx::invoker
{
uint8_t ScriptNativeContext::s_metaFields[(size_t)MetaField::Max];

ScriptNativeContext::ScriptNativeContext(uint64_t hash, PointerField* fields)
	: pointerFields(fields)
{
	nativeIdentifier = hash;
	numArguments = 0;
	numResults = 0;

	if (g_EnableTypeInfo)
	{
		if (auto find = g_NativeTypes.find(MapNative(hash)); find != g_NativeTypes.end())
		{
			typeInfo = find->second;
		}
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
		throw ScriptError("too much isolated data");
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
		throw ScriptError("too many return value arguments");
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

	PushRaw(reinterpret_cast<uintptr_t>(data), type);
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
			case MetaField::PointerValueInt:
			case MetaField::PointerValueFloat:
			case MetaField::PointerValueVector:
			{
				PushReturnValue(metaField, nullptr);
				break;
			}
			case MetaField::ReturnResultAnyway:
				// We are going to return the result anyway if they've given us the type.
				if (returnValueCoercion == MetaField::Max)
					returnValueCoercion = metaField;
				break;
			case MetaField::ResultAsInteger:
			case MetaField::ResultAsLong:
			case MetaField::ResultAsString:
			case MetaField::ResultAsFloat:
			case MetaField::ResultAsVector:
			case MetaField::ResultAsObject:
				returnValueCoercion = metaField;
				break;
			default:
				break;
		}
	}
	// or if the pointer is a runtime pointer field
	else if (ptr >= reinterpret_cast<uint8_t*>(pointerFields) && ptr < (reinterpret_cast<uint8_t*>(pointerFields) + (sizeof(PointerField) * 2)))
	{
		// guess the type based on the pointer field type
		const intptr_t ptrField = ptr - reinterpret_cast<uint8_t*>(pointerFields);
		const MetaField metaField = static_cast<MetaField>(ptrField / sizeof(PointerField));

		if (metaField == MetaField::PointerValueInt || metaField == MetaField::PointerValueFloat)
		{
			auto ptrFieldEntry = reinterpret_cast<PointerFieldEntry*>(ptr);
			ptrFieldEntry->empty = true;
			PushReturnValue(metaField, &ptrFieldEntry->value);
		}
	}
	else
	{
		throw ScriptError("unknown userdata pointer");
	}
}

bool ScriptNativeContext::PreInvoke()
{
	if (!CheckArguments())
	{
		arguments[0] = 0;
		arguments[1] = 0;
		arguments[2] = 0;

		return false;
	}

	// This native might have new arguments added that we don't know about, so zero some extra arguments
	for (int i = numArguments, j = std::min(i + 3, 32); i < j; ++i)
	{
		arguments[i] = 0;
	}

	// Make a copy of the arguments, so we can do some checks afterwards.
	for (int i = 0; i < numArguments; ++i)
	{
		initialArguments[i] = arguments[i];
	}

	return true;
}

void ScriptNativeContext::Invoke(IScriptHost& host)
{
	if (PreInvoke())
	{
		if (!FX_SUCCEEDED(host.InvokeNative(*this)))
		{
			char* error = "Unknown";
			host.GetLastErrorText(&error);

			throw std::runtime_error(va("Execution of native %016llx in script host failed: %s", nativeIdentifier, error));
		}

		PostInvoke();
	}
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

static void InvokeNativeHandler(fxNativeContext& context, rage::scrEngine::NativeHandler handler)
{
	void* exceptionAddress = nullptr;

	// call the original function
	__try
	{
		NativeContextRaw rageContext(context.arguments, context.numArguments);

		handler(&rageContext);

		// append vector3 result components
		rageContext.SetVectorResults();
	}
	__except (
		exceptionAddress = GetExceptionInformation()->ExceptionRecord->ExceptionAddress,
		FilterFunc(GetExceptionInformation(), context.nativeIdentifier))
	{
		throw std::runtime_error(va("Error executing native 0x%016llx at address %s.", context.nativeIdentifier, FormatModuleAddress(exceptionAddress)));
	}
}

void ScriptNativeContext::Invoke(rage::scrEngine::NativeHandler handler)
{
	if (PreInvoke())
	{
		InvokeNativeHandler(*this, handler);

		PostInvoke();
	}
}
#endif

void ScriptNativeContext::PostInvoke()
{
	CheckPointerResult();

	CheckResults();

	// Copy any data back out
	for (int i = 0; i < numIsolatedBuffers; ++i)
	{
		auto& buffer = isolatedBuffers[i];

		std::memcpy(buffer.NativeBuffer, buffer.SafeBuffer, buffer.Size);
	}
}

bool ScriptNativeContext::CheckArguments()
{
	int nargs = 0;
	int nvalid = 0;

	if (typeInfo)
	{
		uint32_t info = typeInfo[0];

		if ((info & PAS_FLAG_BLOCKED) && g_EnforceTypeInfo)
		{
			throw ScriptError("native is blocked");
		}

		nargs = (int)(info & 0xFF);

		// Fast path for natives which don't expect any pointers, and none were passed in.
		if ((info & PAS_FLAG_TRIVIAL) && (pointerMask == 0))
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
				throw ScriptError("not enough arguments (%i < %i)", numArguments, nargs);
			}

			// COMPAT: Some code doesn't pass in enough arguments, so just fill in any missing ones with zero
			for (int i = numArguments; i < nargs; ++i)
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

	for (int i = nvalid; i < nargs; ++i)
	{
		ArgumentType type = types[i];
		uint32_t expected = typeInfo[i + 1];

		if (!(expected & PAS_ARG_POINTER)) // Argument should not be a pointer
		{
			if (type.IsPointer)
			{
				if (g_StrictTypeInfo)
				{
					throw ScriptError("arg[%i]: expected non-pointer", i);
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
						throw ScriptError("arg[%i]: value too large (%i > %i)", i, arguments[i], maxValue);
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
				throw ScriptError("arg[%i]: expected pointer, got non-zero integer", i);
			}

			continue;
		}

		if (expected & PAS_ARG_STRING) // Argument should be a string
		{
			if (!type.IsString)
			{
				throw ScriptError("arg[%i]: expected string", i);
			}

			continue;
		}

		uint32_t minSize = expected & PAS_ARG_SIZE;

		if (type.Size < minSize)
		{
			throw ScriptError("arg[%i]: buffer too small (%u < %u)", i, type.Size, minSize);
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
				throw ScriptError("arg[%i]: buffer length too large (%u > %u)", i, length, type.Size);
			}

			continue;
		}

		// We're not really sure how large this pointer should be, so isolate it.
		IsolatePointer(i);
	}

	// Process any unknown arguments which might have been added in later versions, or this is an unknown native.
	for (int i = nargs; i < numArguments; ++i)
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

void ScriptNativeContext::CheckResults()
{
	if (!typeInfo)
	{
		if (g_EnforceTypeInfo)
		{
			// Block undocumented usage of long/ScrString/ScrObject
			switch (returnValueCoercion)
			{
				case MetaField::ResultAsString:
				{
					// Clear potential ScrString marker
					arguments[1] = 0;
					arguments[2] = 0;

					break;
				}

				case MetaField::ResultAsLong:
				{
					// Hopefully just an undocumented scrBind native, so zero the upper bits
					arguments[0] &= 0xFFFFFFFF;

					break;
				}

				case MetaField::ResultAsObject:
				{
					throw ScriptError("undocumented natives cannot return an object");
				}
			}
		}

		return;
	}

	uint32_t rtype = (typeInfo[0] >> 8) & 0xFF;

	switch (returnValueCoercion)
	{
		case MetaField::ResultAsLong:
		{
			if (rtype == PAS_RET_INT)
			{
				arguments[0] &= 0xFFFFFFFF;
			}
			else if (rtype != PAS_RET_LONG)
			{
				throw ScriptError("result type is not a long");
			}

			break;
		}

		case MetaField::ResultAsString:
		{
			if (rtype == PAS_RET_STRING)
			{
				// Clear potential ScrString marker
				arguments[1] = 0;
				arguments[2] = 0;
			}
			else if (rtype != PAS_RET_SCRSTRING)
			{
				throw ScriptError("result type is not a string");
			}

			break;
		}

		case MetaField::ResultAsVector:
		{
			if (rtype != PAS_RET_VECTOR3)
			{
				throw ScriptError("result type is not a vector");
			}

			break;
		}

		case MetaField::ResultAsObject:
		{
			if (rtype != PAS_RET_SCROBJECT)
			{
				throw ScriptError("result type is not an object");
			}

			break;
		}

		default:
		{
			if (rtype != PAS_RET_INT && rtype != PAS_RET_FLOAT)
			{
				if (arguments[0] != 0) // Preserve zero/false
				{
					arguments[0] = 0xDEADBEEF7FEDCAFE; // Lower 32 bits are NaN
				}
			}

			break;
		}
	}
}

void ScriptNativeContext::CheckPointerResult()
{
	// Don't allow returning a pointer which was passed in.
	switch (returnValueCoercion)
	{
		case MetaField::ResultAsLong:
		{
			if (uintptr_t result = arguments[0])
			{
				for (int i = 0; i < numArguments; ++i)
				{
					// Avoid leaking the addresses of any pointers
					if (result == initialArguments[i] && types[i].IsPointer)
					{
						throw ScriptError("long result matches a pointer argument");
					}
				}
			}

			break;
		}

		case MetaField::ResultAsString:
		{
			if (uintptr_t result = arguments[0])
			{
				for (int i = 0; i < numArguments; ++i)
				{
					// Avoid reading arguments as pointers
					if (result == initialArguments[i])
					{
						// We are returning a pointer which matches what we passed in.
						// However, allow it if we are just returning a string we already passed in (i.e GET_CONVAR default)
						if (types[i].IsString)
						{
							continue;
						}

						throw ScriptError("string result matches a pointer argument");
					}
				}
			}

			// The caller passed in a ScrString marker, and it's still there.
			if (static_cast<uint32_t>(arguments[2]) == SCRSTRING_MAGIC_BINARY && initialArguments[2] == arguments[2])
			{
				throw ScriptError("unexpected scrstring marker");
			}

			break;
		}

		case MetaField::ResultAsObject:
		{
			if (uintptr_t result = arguments[0])
			{
				for (int i = 0; i < numArguments; ++i)
				{
					// An object should never match one which was passed in
					if (result == initialArguments[i])
					{
						throw ScriptError("object result matches a pointer argument");
					}
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
		throw ScriptError("arg[%i]: is not a pointer", index);
	}

	if (type.IsIsolated)
	{
		return;
	}

	if (numIsolatedBuffers >= std::size(isolatedBuffers))
	{
		throw ScriptError("too many unknown pointers");
	}

	uint8_t*& argument = *reinterpret_cast<uint8_t**>(&arguments[index]);
	uint8_t* storage = (uint8_t*)AllocIsolatedData(argument, (size_t)type.Size + (type.IsString ? 1 : 0));

	auto& buffer = isolatedBuffers[numIsolatedBuffers++];
	buffer.Size = type.Size;
	buffer.NativeBuffer = std::exchange(argument, storage);
	buffer.SafeBuffer = storage;

	type.IsIsolated = true;
}

}

static InitFunction initFunction([]
{
#ifdef HAVE_NATIVE_TYPES
	const auto load_types = [] {
		for (const auto& type : pas::native_types)
		{
			g_NativeTypes[MapNative(type.hash)] = type.typeInfo;
		}

		g_EnableTypeInfo = true;
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
