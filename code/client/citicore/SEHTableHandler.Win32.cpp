/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <MinHook.h>
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
	ud_set_mode(&ud, 
#ifdef _M_AMD64
		64
#elif defined(_M_IX86)
		32
#endif
	);

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

#ifdef _M_AMD64
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

	if (baseAddress && GetModuleHandle(L"xtajit64.dll") == nullptr)
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
#else
void DLL_EXPORT CoreRT_SetupSEHHandler(...)
{
	// no-op for non-AMD64
}
#endif


static LONG (*g_exceptionHandler)(EXCEPTION_POINTERS*);
static BOOLEAN(WINAPI *g_origRtlDispatchException)(EXCEPTION_RECORD* record, CONTEXT* context);

static auto g_lastExc0Index = FlsAlloc(NULL);
static auto g_lastExc1Index = FlsAlloc(NULL);

static PIMAGE_SECTION_HEADER GetSection(std::string_view name, int off = 0)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((char*)dosHeader + dosHeader->e_lfanew);
	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

	int matchIdx = -1;

	for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		if (name == (char*)section->Name)
		{
			matchIdx++;

			if (off == matchIdx)
			{
				return section;
			}
		}

		section++;
	}

	return NULL;
}

static uintptr_t moduleBase = (uintptr_t)GetModuleHandle(NULL);
static DWORD moduleSize = 0x60000000;

template<typename T>
static bool IsInSection(T address, PIMAGE_SECTION_HEADER scn)
{
	if (!scn)
	{
		return false;
	}

	auto va = (uintptr_t)GetModuleHandle(NULL) + scn->VirtualAddress;
	auto nva = std::max((uintptr_t)GetModuleHandle(NULL) + scn[1].VirtualAddress, va + scn->Misc.VirtualSize);

	return ((uintptr_t)address >= va && (uintptr_t)address < nva);
}

static std::atomic<int> hardeningOn = 0;

extern "C" void DLL_EXPORT CoreRT_SetHardening(bool hardened)
{
	if (hardened)
	{
		hardeningOn++;
	}
	else
	{
		hardeningOn--;
	}
}

static BOOLEAN WINAPI RtlDispatchExceptionStub(EXCEPTION_RECORD* record, CONTEXT* context)
{
	// anti-anti-anti-anti-debug
	if (CoreIsDebuggerPresent() && (record->ExceptionCode == 0xc0000008 /* || record->ExceptionCode == 0x80000003*/))
	{
		return TRUE;
	}

#if defined(GTA_FIVE) && defined(RWX_TEST)
	if (record->ExceptionCode == 0xC0000005)
	{
		if (record->ExceptionInformation[0] == EXCEPTION_WRITE_FAULT)
		{
			auto textScn = GetSection(".text", 0);
			auto textScnFake = GetSection(".text", 1);
			auto textScnFake2 = GetSection(".text", 2);
			auto tlsScn = GetSection(".tls");
			auto dataScn = GetSection(".data");

			// #TODO: unprotect after a while
			if (IsInSection(record->ExceptionInformation[1], textScn) || IsInSection(record->ExceptionInformation[1], textScnFake) || IsInSection(record->ExceptionInformation[1], textScnFake2) || IsInSection(record->ExceptionInformation[1], tlsScn))
			{
				if (hardeningOn <= 0)
				/* if (IsInSection(record->ExceptionAddress, textScnFake) || IsInSection(record->ExceptionAddress, textScnFake2) ||
					(IsInSection(record->ExceptionAddress, textScn) && 
						(*(uint8_t*)((char*)record->ExceptionAddress + 2) == 0xE9 && *(uint16_t*)record->ExceptionAddress == 0x0289) ||
						(*(uint8_t*)((char*)record->ExceptionAddress + 6) == 0xE9 && *(uint16_t*)record->ExceptionAddress == 0x0589)||
						 *(uint8_t*)((char*)record->ExceptionAddress - 5) == 0xE9)))*/
				{
					DWORD op;
					VirtualProtect((void*)record->ExceptionInformation[1], 0x10000, PAGE_EXECUTE_READWRITE, &op);
					return TRUE;
				}
			}
			else if (IsInSection(record->ExceptionInformation[1], dataScn))
			{
				DWORD op;
				VirtualProtect((void*)record->ExceptionInformation[1], 0x10000, PAGE_READWRITE, &op);
				return TRUE;			
			}
		}
		else if (record->ExceptionInformation[0] == EXCEPTION_EXECUTE_FAULT)
		{
			auto rdataScn = GetSection(".rdata");
			auto pdataScn = GetSection(".pdata");
			auto tlsScn = GetSection(".tls");
			auto bcScn = GetSection("BINKCONS");

			if (IsInSection(record->ExceptionInformation[1], rdataScn) || IsInSection(record->ExceptionInformation[1], pdataScn) || IsInSection(record->ExceptionInformation[1], tlsScn) || IsInSection(record->ExceptionInformation[1], bcScn))
			{
				DWORD op;
				VirtualProtect((void*)record->ExceptionInformation[1], 0x10000, PAGE_EXECUTE_READ, &op);

				if (op == PAGE_READWRITE || op == PAGE_EXECUTE_READWRITE)
				{
					VirtualProtect((void*)record->ExceptionInformation[1], 0x10000, PAGE_EXECUTE_READWRITE, &op);
				}

				return TRUE;
			}
		}
	}
#endif

	FlsSetValue(g_lastExc0Index, record);
	FlsSetValue(g_lastExc1Index, context);

	BOOLEAN success = g_origRtlDispatchException(record, context);

	FlsSetValue(g_lastExc0Index, nullptr);
	FlsSetValue(g_lastExc1Index, nullptr);

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

#ifndef IS_FXSERVER
			AddCrashometry("exception_override", "true");
#endif

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

static NTSTATUS (NTAPI* g_origRtlReportException)(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord, ULONG Flags);

static NTSTATUS NTAPI RtlReportExceptionStub(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord, ULONG Flags)
{
	static bool inExceptionFallback;

	if (!inExceptionFallback && !(Flags & 8))
	{
		inExceptionFallback = true;

#ifndef IS_FXSERVER
		AddCrashometry("exception_override_2", "true");
#endif

		EXCEPTION_POINTERS ptrs = { 0 };
		ptrs.ContextRecord = ContextRecord;
		ptrs.ExceptionRecord = ExceptionRecord;

		if (g_exceptionHandler)
		{
			g_exceptionHandler(&ptrs);
		}

		inExceptionFallback = false;
	}

	return g_origRtlReportException(ExceptionRecord, ContextRecord, Flags);
}

static bool (*_TerminateForException)(PEXCEPTION_POINTERS exc);

static void terminateStub()
{
	auto exc = reinterpret_cast<PEXCEPTION_RECORD>(FlsGetValue(g_lastExc0Index));

	if (exc && exc->ExceptionCode == 0xE06D7363)
	{
		EXCEPTION_POINTERS ptrs;
		ptrs.ExceptionRecord = exc;
		ptrs.ContextRecord = reinterpret_cast<PCONTEXT>(FlsGetValue(g_lastExc1Index));

		_TerminateForException(&ptrs);
	}

	FatalError("UCRT terminate() called");
}

extern "C" void DLL_EXPORT CoreSetExceptionOverride(LONG (*handler)(EXCEPTION_POINTERS*))
{
	_TerminateForException = (decltype(_TerminateForException))GetProcAddress(GetModuleHandle(NULL), "TerminateForException");

	g_exceptionHandler = handler;

	void* baseAddress = GetProcAddress(GetModuleHandle(L"ntdll.dll"), "KiUserExceptionDispatcher");

	if (baseAddress)
	{
		void* internalAddress = FindCallFromAddress(baseAddress, UD_Icall, true);

		{
			DisableToolHelpScope scope;
			MH_CreateHook(internalAddress, RtlDispatchExceptionStub, (void**)&g_origRtlDispatchException);
			MH_CreateHook(GetProcAddress(GetModuleHandle(L"ucrtbase.dll"), "terminate"), terminateStub, NULL);
			MH_CreateHookApi(L"ntdll.dll", "RtlReportException", RtlReportExceptionStub, (void**)&g_origRtlReportException);
			MH_EnableHook(MH_ALL_HOOKS);
		}
	}
}

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
