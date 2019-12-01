/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifndef IS_FXSERVER
#include <minhook.h>

#ifdef _M_AMD64
#include "Hooking.Aux.h"

#include <Error.h>

#include <udis86.h>

static void* FindCallFromAddress(void* methodPtr, ud_mnemonic_code mnemonic = UD_Icall, bool breakOnFirst = false)
{
	// return value holder
	void* retval = nullptr;

	// initialize udis86
	ud_t ud;
	ud_init(&ud);

	// set the correct architecture
	ud_set_mode(&ud, 64);

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(methodPtr));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(methodPtr), INT32_MAX);

	// loop the instructions
	while (true)
	{
		// disassemble the next instruction
		ud_disassemble(&ud);

		// if this is a retn, break from the loop
		if (ud_insn_mnemonic(&ud) == UD_Iint3 || ud_insn_mnemonic(&ud) == UD_Inop)
		{
			break;
		}

		if (ud_insn_mnemonic(&ud) == mnemonic)
		{
			// get the first operand
			auto operand = ud_insn_opr(&ud, 0);

			// if it's a static call...
			if (operand->type == UD_OP_JIMM)
			{
				// ... and there's been no other such call...
				if (retval == nullptr)
				{
					// ... calculate the effective address and store it
					retval = reinterpret_cast<void*>(ud_insn_len(&ud) + ud_insn_off(&ud) + operand->lval.sdword);

					if (breakOnFirst)
					{
						break;
					}
				}
				else
				{
					// return an empty pointer
					retval = nullptr;
					break;
				}
			}
		}
	}

	return retval;
}

struct FUNCTION_TABLE_DATA
{
	DWORD64 TableAddress;
	DWORD64 ImageBase;
	DWORD ImageSize; // field +8 in ZwQueryVirtualMemory class 6
	DWORD Size;
};

static void*(*g_originalLookup)(void*, FUNCTION_TABLE_DATA*);

static FUNCTION_TABLE_DATA g_overriddenTable;
static DWORD64 g_overrideStart;
static DWORD64 g_overrideEnd;

static void* RtlpxLookupFunctionTableOverride(void* exceptionAddress, FUNCTION_TABLE_DATA* outData)
{
	memset(outData, 0, sizeof(*outData));

	void* retval = g_originalLookup(exceptionAddress, outData);

	DWORD64 addressNum = (DWORD64)exceptionAddress;

	if (addressNum >= g_overrideStart && addressNum <= g_overrideEnd)
	{
		if (addressNum != 0)
		{
			*outData = g_overriddenTable;

			retval = (void*)g_overriddenTable.TableAddress;
		}
	}

	return retval;
}

static void*(*g_originalLookupDownLevel)(void*, PDWORD64, PULONG);

static void* RtlpxLookupFunctionTableOverrideDownLevel(void* exceptionAddress, PDWORD64 imageBase, PULONG length)
{
	void* retval = g_originalLookupDownLevel(exceptionAddress, imageBase, length);

	DWORD64 addressNum = (DWORD64)exceptionAddress;

	if (addressNum >= g_overrideStart && addressNum <= g_overrideEnd)
	{
		if (addressNum != 0)
		{
			*imageBase = g_overriddenTable.ImageBase;
			*length = g_overriddenTable.Size;

			retval = (void*)g_overriddenTable.TableAddress;
		}
	}

	return retval;
}

#include <winternl.h>

static PVOID(*g_origRtlImageDirectoryEntryToData)(HMODULE hModule, BOOL a2, WORD directory, ULONG* a4);

static PVOID RtlImageDirectoryEntryToDataStub(HMODULE hModule, BOOL a2, WORD directory, ULONG* size)
{
	if ((DWORD64)hModule == g_overrideStart)
	{
		if (directory == IMAGE_DIRECTORY_ENTRY_EXCEPTION)
		{
			*size = g_overriddenTable.Size;
			return (PVOID)g_overriddenTable.TableAddress;
		}
	}

	return g_origRtlImageDirectoryEntryToData(hModule, a2, directory, size);
}

extern "C" void DLL_EXPORT CoreRT_SetupSEHHandler(void* moduleBase, void* moduleEnd, PRUNTIME_FUNCTION runtimeFunctions, DWORD entryCount)
{
	// store passed data
	g_overrideStart = (DWORD64)moduleBase;
	g_overrideEnd = (DWORD64)moduleEnd;

	g_overriddenTable.ImageBase = g_overrideStart;
	g_overriddenTable.TableAddress = (DWORD64)runtimeFunctions;
	g_overriddenTable.Size = entryCount * sizeof(RUNTIME_FUNCTION);

	if (IsWindows8Point1OrGreater())
	{
		auto ZwQueryVirtualMemory = (NTSTATUS(NTAPI*)(HANDLE, PVOID, INT, PVOID, SIZE_T, PSIZE_T))GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQueryVirtualMemory");
		
		struct
		{
			DWORD64 field0;
			DWORD imageSize;
			DWORD fieldC;
			DWORD64 field10;
		} queryResult;

		ZwQueryVirtualMemory(GetCurrentProcess(), GetModuleHandle(nullptr), 6, &queryResult, sizeof(queryResult), nullptr);

		g_overriddenTable.ImageSize = queryResult.imageSize;
	}

	// find the location to hook (RtlpxLookupFunctionTable from RtlLookupFunctionTable)
	void* baseAddress = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlLookupFunctionTable");

	if (baseAddress)
	{
		void* internalAddress = FindCallFromAddress(baseAddress);

		void* patchFunction = RtlpxLookupFunctionTableOverride;
		void** patchOriginal = (void**)&g_originalLookup;

		// if we couldn't _reliably_ find it, error out
		if (!internalAddress)
		{
			// Windows 8 uses a Rtl*-style call for Rtlpx
			if (!IsWindows8Point1OrGreater())
			{
				internalAddress = FindCallFromAddress(baseAddress, UD_Ijmp);

				patchFunction = RtlpxLookupFunctionTableOverrideDownLevel;
				patchOriginal = (void**)&g_originalLookupDownLevel;
			}

			if (!internalAddress)
			{
				// and 2k3 to 7 don't even _have_ Rtlpx - so we directly hook the Rtl* function
				if (IsWindows8OrGreater())
				{
					FatalError("Could not find RtlpxLookupFunctionTable - hooking RtlLookupFunctionTable directly. This will break on a Win8+ system since RtlpxLookupFunctionTable is supposed to exist!\n");
				}

				internalAddress = baseAddress;

				patchFunction = RtlpxLookupFunctionTableOverrideDownLevel;
				patchOriginal = (void**)&g_originalLookupDownLevel;
			}
		}

		// patch it
		DisableToolHelpScope scope;
		MH_CreateHook(internalAddress, patchFunction, patchOriginal);
		MH_EnableHook(MH_ALL_HOOKS);
	}
	else
	{
		trace("Not running on Windows - no RtlLookupFunctionTable. Is this some fake OS?\n");

		DisableToolHelpScope scope;
		MH_CreateHookApi(L"ntdll.dll", "RtlImageDirectoryEntryToData", RtlImageDirectoryEntryToDataStub, (void**)&g_origRtlImageDirectoryEntryToData);
		MH_EnableHook(MH_ALL_HOOKS);
	}
}

static LONG(*g_exceptionHandler)(EXCEPTION_POINTERS*);
static BOOLEAN(*g_origRtlDispatchException)(EXCEPTION_RECORD* record, CONTEXT* context);

static BOOLEAN RtlDispatchExceptionStub(EXCEPTION_RECORD* record, CONTEXT* context)
{
	// anti-anti-anti-anti-debug
	if (CoreIsDebuggerPresent() && (record->ExceptionCode == 0xc0000008/* || record->ExceptionCode == 0x80000003*/))
	{
		return TRUE;
	}

	BOOLEAN success = g_origRtlDispatchException(record, context);

	if (CoreIsDebuggerPresent())
	{
		return success;
	}

	static bool inExceptionFallback;

	if (!success)
	{
		if (!inExceptionFallback)
		{
			inExceptionFallback = true;

			AddCrashometry("exception_override", "true");

			EXCEPTION_POINTERS ptrs;
			ptrs.ContextRecord = context;
			ptrs.ExceptionRecord = record;

			if (g_exceptionHandler)
			{
				g_exceptionHandler(&ptrs);
			}

			inExceptionFallback = false;
		}
	}

	return success;
}

extern "C" void DLL_EXPORT CoreSetExceptionOverride(LONG(*handler)(EXCEPTION_POINTERS*))
{
	g_exceptionHandler = handler;

	void* baseAddress = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "KiUserExceptionDispatcher");

	if (baseAddress)
	{
		void* internalAddress = FindCallFromAddress(baseAddress, UD_Icall, true);

		{
			DisableToolHelpScope scope;
			MH_CreateHook(internalAddress, RtlDispatchExceptionStub, (void**)&g_origRtlDispatchException);
			MH_EnableHook(MH_ALL_HOOKS);
		}
	}
}
#else
void DLL_EXPORT CoreRT_SetupSEHHandler(...)
{
	// no-op for non-AMD64
}
#endif

struct InitMHWrapper
{
	InitMHWrapper()
	{
		MH_Initialize();
	}

	~InitMHWrapper()
	{
		MH_Uninitialize();
	}
};

InitMHWrapper mh;
#endif
