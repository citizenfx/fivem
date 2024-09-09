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

#if __has_include(<PointerArgumentHints.h>)
#include <PointerArgumentHints.h>
#endif

#define PAS_ARG_POINTER 0x80000000 // This argument is a pointer
#define PAS_ARG_STRING  0x40000000 // This argument is a string
#define PAS_ARG_BUFFER  0x20000000 // This argument is a read-only buffer (and the next argument is its length)
#define PAS_ARG_SIZE	0x1FFFFFFF // The minimum allowable size of this pointer argument

// The return type of the function
#define PAS_RET_VOID 0
#define PAS_RET_INT 1
#define PAS_RET_FLOAT 2
#define PAS_RET_LONG 3
#define PAS_RET_VECTOR3 4
#define PAS_RET_STRING 5
#define PAS_RET_SCRSTRING 6
#define PAS_RET_SCROBJECT 7

static constexpr size_t g_IsolatedBufferSize = 3 * 4096;
static uint8_t* g_IsolatedBuffers[32];
static size_t g_IsolatedBufferIndex = 0;
static bool g_EnforceTypeInfo = false;

namespace fx::invoker
{
uint8_t ScriptNativeContext::s_metaFields[(size_t)MetaField::Max];

ScriptNativeContext::ScriptNativeContext(uint64_t hash, PointerField* fields)
	: pointerFields(fields)
{
	nativeIdentifier = hash;
	numArguments = 0;
	numResults = 0;

#if __has_include(<PointerArgumentHints.h>)
	typeInfo = fx::scripting::GetNativeTypeInfo(hash);
#endif
}

bool ScriptNativeContext::PushRaw(uintptr_t value, ArgumentType type)
{
	if (numArguments >= 32)
	{
		return SetError("too many arguments");
	}

	arguments[numArguments] = value;
	types[numArguments] = type;
	pointerMask |= (uint32_t)type.IsPointer << numArguments;

	++numArguments;

	return true;
}

bool ScriptNativeContext::PushReturnValue(MetaField field, bool zero)
{
	int slots = (field == MetaField::PointerValueVector) ? 3 : 1;

	if (numReturnValues + slots > std::size(retvals))
	{
		return SetError("too many return value arguments");
	}

	uintptr_t* arg = &retvals[numReturnValues];

	if (zero)
	{
		for (int i = 0; i < slots; ++i)
		{
			arg[i] = 0;
		}
	}

	rettypes[numReturnValues] = field;
	numReturnValues += slots;

	return Push(arg, slots * sizeof(uintptr_t));
}

bool ScriptNativeContext::PushMetaPointer(uint8_t* ptr)
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
				return PushReturnValue(metaField, true);
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

			retvals[numReturnValues] = ptrFieldEntry->value;
			ptrFieldEntry->empty = true;

			return PushReturnValue(metaField, false);
		}
	}
	else
	{
		return SetError("unknown userdata pointer");
	}

	return true;
}

bool ScriptNativeContext::PreInvoke()
{
	if (!CheckArguments())
	{
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

bool ScriptNativeContext::PostInvoke()
{
	if (!CheckResults())
	{
		return false;
	}

	// Copy any data back out
	for (int i = 0; i < numIsolatedBuffers; ++i)
	{
		auto& buffer = isolatedBuffers[i];

		std::memcpy(buffer.NativeBuffer, buffer.SafeBuffer, buffer.Size);
	}

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
						return SetError("pointer result matches an argument");
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

						return SetError("pointer result matches an argument");
					}
				}
			}

			// The caller passed in a ScrString marker, and it's still there.
			if (static_cast<uint32_t>(arguments[2]) == SCRSTRING_MAGIC_BINARY && initialArguments[2] == arguments[2])
			{
				return SetError("unexpected scrstring marker");
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
						return SetError("pointer result matches an argument");
					}
				}
			}

			break;
		}
	}

	return true;
}

bool ScriptNativeContext::CheckArguments()
{
	int nargs = 0;
	int nvalid = 0;

	if (typeInfo)
	{
		nargs = (int)typeInfo[0];

		// Nullified native
		if (nargs == -1)
		{
			if (!g_EnforceTypeInfo)
			{
				return true;
			}

			return SetError("native is disabled");
		}

		uint32_t pmask = typeInfo[1];

		// Fast path for natives which don't expect any pointers, and none were passed in.
		if (pmask == 0x00000000 && pmask == pointerMask)
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
			return SetError("not enough arguments (%i < %i)", numArguments, nargs);
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

		uint32_t expected = typeInfo[i + 3];
		
		if (!(expected & PAS_ARG_POINTER)) // Argument should not be a pointer
		{
			if (type.IsPointer)
			{
				return SetError("arg[%i]: expected non-pointer", i);
			}

			continue;
		}

		if (!type.IsPointer)
		{
			if (arguments[i] != 0) // Argument should be a pointer, but also allow NULL
			{
				return SetError("arg[%i]: expected pointer, got non-zero integer", i);
			}

			continue;
		}

		if (expected & PAS_ARG_STRING) // Argument should be a string
		{
			if (!type.IsString)
			{
				return SetError("arg[%i]: expected string", i);
			}

			continue;
		}

		uint32_t minSize = expected & PAS_ARG_SIZE;

		if (type.Size < minSize)
		{
			return SetError("arg[%i]: buffer too small (%u < %u)", i, type.Size, minSize);
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
				return SetError("arg[%i]: buffer length too large (%u > %u)", i, length, type.Size);
			}

			continue;
		}

		// We're not really sure how large this pointer should be, so isolate it.
		if (!IsolatePointer(i))
		{
			return false;
		}
	}

	// Process any unknown arguments which might have been added in later versions, or this is an unknown native.
	for (int i = nargs; i < numArguments; ++i)
	{
		if (types[i].IsPointer)
		{
			// We have no idea how this pointer might be used, so isolate it.
			if (!IsolatePointer(i))
			{
				return false;
			}
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

bool ScriptNativeContext::CheckResults()
{
	if (!typeInfo)
	{
		return true;
	}

	uint32_t rtype = typeInfo[2];

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
				return SetError("result type is not a long");
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
				return SetError("result type is not a string");
			}

			break;
		}

		case MetaField::ResultAsVector:
		{
			if (rtype != PAS_RET_VECTOR3)
			{
				return SetError("result type is not a vector");
			}

			break;
		}

		case MetaField::ResultAsObject:
		{
			if (rtype != PAS_RET_SCROBJECT)
			{
				return SetError("result type is not an object");
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

	return true;
}

bool ScriptNativeContext::IsolatePointer(int index)
{
	ArgumentType type = types[index];

	if (!type.IsPointer)
	{
		return SetError("arg[%i]: is not a pointer", index);
	}

	if (numIsolatedBuffers >= std::size(isolatedBuffers))
	{
		return SetError("too many unknown pointers");
	}

	// Too large!
	if (type.Size >= g_IsolatedBufferSize)
	{
		return SetError("arg[%i]: buffer too large to isolate", index);
	}

	auto& buffer = isolatedBuffers[numIsolatedBuffers++];

	buffer.Size = type.Size;
	buffer.SafeBuffer = g_IsolatedBuffers[g_IsolatedBufferIndex];
	g_IsolatedBufferIndex = (g_IsolatedBufferIndex + 1) % std::size(g_IsolatedBuffers);

	buffer.NativeBuffer = std::exchange(*reinterpret_cast<uint8_t**>(&arguments[index]), buffer.SafeBuffer);

	// Copy in any existing data.
	std::memcpy(buffer.SafeBuffer, buffer.NativeBuffer, buffer.Size);

	// These buffers shouldn't contain any sensitive data, so just add a terminator, don't bother zeroing the rest.
	buffer.SafeBuffer[buffer.Size] = type.IsString ? 0x00 : 0xFF;

	return true;
}

}

static
#ifdef IS_FXSERVER
InitFunction
#else
HookFunction // FIXME: ICoreGameInit isn't registered when our InitFunctions runs
#endif
initFunction([]
{
	if (!launch::IsSDK())
	{
#ifdef GTA_FIVE
		g_EnforceTypeInfo = true;

		Instance<ICoreGameInit>::Get()->OnSetVariable.Connect([](const std::string& name, bool value)
		{
			if (name == "storyMode")
			{
				g_EnforceTypeInfo = !value;
			}
		});
#else
		// TODO: Generate type info for other games
#endif
	}

	size_t total_size = (g_IsolatedBufferSize + 4096) * std::size(g_IsolatedBuffers);

	uint8_t* region = 
#ifdef _WIN32
	(uint8_t*)VirtualAlloc(NULL, total_size, MEM_COMMIT, PAGE_NOACCESS);
#else
	new uint8_t[total_size]{}; // TODO: Use mmap
#endif

	for (auto& buffer : g_IsolatedBuffers)
	{
		buffer = region;
		region += g_IsolatedBufferSize + 4096;

#ifdef _WIN32
		DWORD oldProtect;
		VirtualProtect(buffer, g_IsolatedBufferSize, PAGE_READWRITE, &oldProtect);
#else
		// TODO: Use mprotect
#endif
	}
});
