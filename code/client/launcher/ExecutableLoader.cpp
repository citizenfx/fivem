/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ExecutableLoader.h"

#include <Error.h>
#include <Hooking.h>

ExecutableLoader::ExecutableLoader(const uint8_t* origBinary)
{
	hook::set_base();

	m_origBinary = origBinary;
	m_loadLimit = UINTPTR_MAX;
	
	SetLibraryLoader([] (const char* name)
	{
		return LoadLibraryA(name);
	});

	SetFunctionResolver([] (HMODULE module, const char* name)
	{
		return (LPVOID)GetProcAddress(module, name);
	});
}

void ExecutableLoader::LoadImports(IMAGE_NT_HEADERS* ntHeader)
{
	IMAGE_DATA_DIRECTORY* importDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	auto descriptor = GetTargetRVA<IMAGE_IMPORT_DESCRIPTOR>(importDirectory->VirtualAddress);

	while (descriptor->Name)
	{
		const char* name = GetTargetRVA<char>(descriptor->Name);

		HMODULE module = ResolveLibrary(name);

		if (!module)
		{
			auto errorCode = GetLastError();

			wchar_t errorText[512];
			errorText[0] = L'\0';

			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), errorText, std::size(errorText), nullptr);

			FatalError("Could not load dependent module %s. Error code %i -> %s\n", name, errorCode, ToNarrow(errorText));
		}

		// "don't load"
		if (reinterpret_cast<uint32_t>(module) == 0xFFFFFFFF)
		{
			descriptor++;
			continue;
		}

		auto nameTableEntry = GetTargetRVA<uintptr_t>(descriptor->OriginalFirstThunk);
		auto addressTableEntry = GetTargetRVA<uintptr_t>(descriptor->FirstThunk);

		// GameShield (Payne) uses FirstThunk for original name addresses
		if (!descriptor->OriginalFirstThunk)
		{
			nameTableEntry = GetTargetRVA<uintptr_t>(descriptor->FirstThunk);
		}

		while (*nameTableEntry)
		{
			FARPROC function;
			const char* functionName;

			// is this an ordinal-only import?
			if (IMAGE_SNAP_BY_ORDINAL(*nameTableEntry))
			{
				function = GetProcAddress(module, MAKEINTRESOURCEA(IMAGE_ORDINAL(*nameTableEntry)));
				functionName = va("#%d", IMAGE_ORDINAL(*nameTableEntry));
			}
			else
			{
				auto import = GetTargetRVA<IMAGE_IMPORT_BY_NAME>(*nameTableEntry);

				function = (FARPROC)m_functionResolver(module, import->Name);
				functionName = import->Name;
			}

			if (!function)
			{
				char pathName[MAX_PATH];
				GetModuleFileNameA(module, pathName, sizeof(pathName));

				FatalError("Could not load function %s in dependent module %s (%s).\n", functionName, name, pathName);
			}

			*addressTableEntry = (uintptr_t)function;

			nameTableEntry++;
			addressTableEntry++;
		}

		descriptor++;
	}
}

void ExecutableLoader::LoadSection(IMAGE_SECTION_HEADER* section)
{
	void* targetAddress = GetTargetRVA<uint8_t>(section->VirtualAddress);
	const void* sourceAddress = m_origBinary + section->PointerToRawData;

	if ((uintptr_t)targetAddress >= (m_loadLimit + hook::baseAddressDifference))
	{
		return;
	}

	if (section->SizeOfRawData > 0)
	{
		uint32_t sizeOfData = fwMin(section->SizeOfRawData, section->Misc.VirtualSize);

		memcpy(targetAddress, sourceAddress, sizeOfData);
	}

	DWORD oldProtect;
	VirtualProtect(targetAddress, section->Misc.VirtualSize, PAGE_EXECUTE_READWRITE, &oldProtect);

	DWORD protection = 0;
	if (section->Characteristics & IMAGE_SCN_MEM_NOT_CACHED)
	{
		protection |= PAGE_NOCACHE;
	}

	if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
	{
		if (section->Characteristics & IMAGE_SCN_MEM_READ)
		{
			if (section->Characteristics & IMAGE_SCN_MEM_WRITE)
			{
				protection |= PAGE_EXECUTE_READWRITE;
			}
			else
			{
				protection |= PAGE_EXECUTE_READ;
			}
		}
		else
		{
			if (section->Characteristics & IMAGE_SCN_MEM_WRITE)
			{
				protection |= PAGE_EXECUTE_WRITECOPY;
			}
			else
			{
				protection |= PAGE_EXECUTE;
			}
		}
	}
	else
	{
		if (section->Characteristics & IMAGE_SCN_MEM_READ)
		{
			if (section->Characteristics & IMAGE_SCN_MEM_WRITE)
			{
				protection |= PAGE_READWRITE;
			}
			else
			{
				protection |= PAGE_READONLY;
			}
		}
		else
		{
			if (section->Characteristics & IMAGE_SCN_MEM_WRITE)
			{
				protection |= PAGE_WRITECOPY;
			}
			else
			{
				protection |= PAGE_NOACCESS;
			}
		}
	}	

	if (protection)
	{
		m_targetProtections.push_back({ targetAddress, section->Misc.VirtualSize, protection });
	}
}

void ExecutableLoader::Protect()
{
	for (const auto& protection : m_targetProtections)
	{
		DWORD op;
		VirtualProtect(std::get<0>(protection), std::get<1>(protection), std::get<2>(protection), &op);
	}
}

void ExecutableLoader::LoadSections(IMAGE_NT_HEADERS* ntHeader)
{
	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

	for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		LoadSection(section);

		section++;
	}
}

#if defined(_M_AMD64)
typedef enum _FUNCTION_TABLE_TYPE
{
	RF_SORTED,
	RF_UNSORTED,
	RF_CALLBACK
} FUNCTION_TABLE_TYPE;

typedef struct _DYNAMIC_FUNCTION_TABLE
{
	LIST_ENTRY Links;
	PRUNTIME_FUNCTION FunctionTable;
	LARGE_INTEGER TimeStamp;

	ULONG_PTR MinimumAddress;
	ULONG_PTR MaximumAddress;
	ULONG_PTR BaseAddress;

	PGET_RUNTIME_FUNCTION_CALLBACK Callback;
	PVOID Context;
	PWSTR OutOfProcessCallbackDll;
	FUNCTION_TABLE_TYPE Type;
	ULONG EntryCount;
} DYNAMIC_FUNCTION_TABLE, *PDYNAMIC_FUNCTION_TABLE;

void ExecutableLoader::LoadExceptionTable(IMAGE_NT_HEADERS* ntHeader)
{
	IMAGE_DATA_DIRECTORY* exceptionDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];

	RUNTIME_FUNCTION* functionList = GetTargetRVA<RUNTIME_FUNCTION>(exceptionDirectory->VirtualAddress);
	DWORD entryCount = exceptionDirectory->Size / sizeof(RUNTIME_FUNCTION);

	// has no use - inverted function tables get used instead from Ldr; we have no influence on those
	if (!RtlAddFunctionTable(functionList, entryCount, (DWORD64)GetModuleHandle(nullptr)))
	{
		FatalError("Setting exception handlers failed.");
	}

	// replace the function table stored for debugger purposes (though we just added it above)
	{
		PLIST_ENTRY(NTAPI *rtlGetFunctionTableListHead)(VOID);
		rtlGetFunctionTableListHead = (decltype(rtlGetFunctionTableListHead))GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlGetFunctionTableListHead");

		if (rtlGetFunctionTableListHead)
		{
			auto tableListHead = rtlGetFunctionTableListHead();
			auto tableListEntry = tableListHead->Flink;

			while (tableListEntry != tableListHead)
			{
				auto functionTable = CONTAINING_RECORD(tableListEntry, DYNAMIC_FUNCTION_TABLE, Links);

				if (functionTable->BaseAddress == (ULONG_PTR)m_module)
				{
					trace("Replacing function table list entry %p with %p\n", (void*)functionTable->FunctionTable, (void*)functionList);

					if (functionTable->FunctionTable != functionList)
					{
						DWORD oldProtect;
						VirtualProtect(functionTable, sizeof(DYNAMIC_FUNCTION_TABLE), PAGE_READWRITE, &oldProtect);

						functionTable->EntryCount = entryCount;
						functionTable->FunctionTable = functionList;

						VirtualProtect(functionTable, sizeof(DYNAMIC_FUNCTION_TABLE), oldProtect, &oldProtect);
					}
				}

				tableListEntry = functionTable->Links.Flink;
			}
		}
	}

	// use CoreRT API instead
	if (HMODULE coreRT = GetModuleHandle(L"CoreRT.dll"))
	{
		auto sehMapper = (void(*)(void*, void*, PRUNTIME_FUNCTION, DWORD))GetProcAddress(coreRT, "CoreRT_SetupSEHHandler");

		sehMapper(m_module, ((char*)m_module) + ntHeader->OptionalHeader.SizeOfImage, functionList, entryCount);
	}
}
#endif

static void InitTlsFromExecutable();
extern HMODULE tlsDll;

void ExecutableLoader::LoadIntoModule(HMODULE module)
{
	m_module = module;

	IMAGE_DOS_HEADER* header = (IMAGE_DOS_HEADER*)m_origBinary;

	if (header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return;
	}

	IMAGE_DOS_HEADER* sourceHeader = (IMAGE_DOS_HEADER*)module;
	IMAGE_NT_HEADERS* sourceNtHeader = GetTargetRVA<IMAGE_NT_HEADERS>(sourceHeader->e_lfanew);

	auto origCheckSum = sourceNtHeader->OptionalHeader.CheckSum;
	auto origTimeStamp = sourceNtHeader->FileHeader.TimeDateStamp;
	auto origDebugDir = sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];

#if defined(PAYNE)
	IMAGE_TLS_DIRECTORY origTls = *GetTargetRVA<IMAGE_TLS_DIRECTORY>(sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
#endif

	IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(m_origBinary + header->e_lfanew);

	if (!LoadSnapshot(ntHeader))
	{
		LoadSections(ntHeader);
	}

	DWORD oldProtect;
	VirtualProtect(sourceNtHeader, 0x1000, PAGE_EXECUTE_READWRITE, &oldProtect);

	sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC] = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	ApplyRelocations();

	LoadImports(ntHeader);

#if defined(_M_AMD64)
	LoadExceptionTable(ntHeader);
#endif

	// copy over TLS index (source in this case indicates the TLS data to copy from, which is the launcher app itself)
	if (ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
	{
		const IMAGE_TLS_DIRECTORY* targetTls = GetTargetRVA<IMAGE_TLS_DIRECTORY>(sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
		const IMAGE_TLS_DIRECTORY* sourceTls = GetTargetRVA<IMAGE_TLS_DIRECTORY>(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

#if defined(_M_IX86)
		LPVOID* tlsBase = (LPVOID*)__readfsdword(0x2C);
#elif defined(_M_AMD64)
		LPVOID* tlsBase = (LPVOID*)__readgsqword(0x58);
#endif

#ifndef GTA_NY
		uint32_t tlsIndex = 0;
		void* tlsInit = nullptr;

		auto tlsExport = (void(*)(void**, uint32_t*))GetProcAddress(tlsDll, "GetThreadLocalStorage");
		tlsExport(&tlsInit, &tlsIndex);

		assert(tlsIndex < 64);

		if (sourceTls->StartAddressOfRawData)
		{
			DWORD oldProtect;
			VirtualProtect(tlsInit, sourceTls->EndAddressOfRawData - sourceTls->StartAddressOfRawData, PAGE_READWRITE, &oldProtect);

			memcpy(tlsBase[tlsIndex], reinterpret_cast<void*>(sourceTls->StartAddressOfRawData), sourceTls->EndAddressOfRawData - sourceTls->StartAddressOfRawData);
			memcpy(tlsInit, reinterpret_cast<void*>(sourceTls->StartAddressOfRawData), sourceTls->EndAddressOfRawData - sourceTls->StartAddressOfRawData);
		}

		if (sourceTls->AddressOfIndex)
		{
			hook::put(sourceTls->AddressOfIndex, tlsIndex);
		}
#else
		if (sourceTls->StartAddressOfRawData)
		{
			DWORD oldProtect;
			VirtualProtect((void*)targetTls->StartAddressOfRawData, sourceTls->EndAddressOfRawData - sourceTls->StartAddressOfRawData, PAGE_READWRITE, &oldProtect);

			memcpy(tlsBase[0], reinterpret_cast<void*>(sourceTls->StartAddressOfRawData), sourceTls->EndAddressOfRawData - sourceTls->StartAddressOfRawData);
			memcpy((void*)targetTls->StartAddressOfRawData, reinterpret_cast<void*>(sourceTls->StartAddressOfRawData), sourceTls->EndAddressOfRawData - sourceTls->StartAddressOfRawData);
		}
#endif
	}

	// store the entry point
	m_entryPoint = GetTargetRVA<void>(ntHeader->OptionalHeader.AddressOfEntryPoint);

	// copy over the offset to the new imports directory
	sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT] = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

#if defined(GTA_FIVE) || defined(IS_RDR3) || defined(GTA_NY)
	memcpy(sourceNtHeader, ntHeader, sizeof(IMAGE_NT_HEADERS) + (ntHeader->FileHeader.NumberOfSections * (sizeof(IMAGE_SECTION_HEADER))));
#endif

	sourceNtHeader->OptionalHeader.CheckSum = origCheckSum;
	sourceNtHeader->FileHeader.TimeDateStamp = origTimeStamp;

	sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG] = origDebugDir;
}

bool ExecutableLoader::ApplyRelocations()
{
	IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(m_module);

	IMAGE_NT_HEADERS* ntHeader = GetTargetRVA<IMAGE_NT_HEADERS>(dosHeader->e_lfanew);

	IMAGE_DATA_DIRECTORY* relocationDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	IMAGE_BASE_RELOCATION* relocation = GetTargetRVA<IMAGE_BASE_RELOCATION>(relocationDirectory->VirtualAddress);
	IMAGE_BASE_RELOCATION* endRelocation = reinterpret_cast<IMAGE_BASE_RELOCATION*>((char*)relocation + relocationDirectory->Size);

	constexpr uintptr_t base =
#ifdef _M_AMD64
	0x140000000
#else
	0x400000
#endif
	;

	intptr_t relocOffset = reinterpret_cast<intptr_t>(m_module) - static_cast<intptr_t>(base);

	if (relocOffset == 0)
	{
		return true;
	}

	// loop
	while (true)
	{
		// are we past the size?
		if (relocation >= endRelocation)
		{
			break;
		}

		// is this an empty block?
		if (relocation->SizeOfBlock == 0)
		{
			break;
		}

		// go through each and every relocation
		size_t numRelocations = (relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);
		uint16_t * relocStart = reinterpret_cast<uint16_t*>(relocation + 1);

		for (size_t i = 0; i < numRelocations; i++)
		{
			uint16_t type = relocStart[i] >> 12;
			uint32_t rva = (relocStart[i] & 0xFFF) + relocation->VirtualAddress;

			void* addr = GetTargetRVA<void>(rva);

			if (type == IMAGE_REL_BASED_HIGHLOW)
			{
				*reinterpret_cast<int32_t*>(addr) += relocOffset;
			}
			else if (type == IMAGE_REL_BASED_DIR64)
			{
				*reinterpret_cast<int64_t*>(addr) += relocOffset;
			}
			else if (type != IMAGE_REL_BASED_ABSOLUTE)
			{
				return false;
			}
		}

		// on to the next one!
		relocation = reinterpret_cast<IMAGE_BASE_RELOCATION*>((char*)relocation + relocation->SizeOfBlock);
	}

	return true;
}

HMODULE ExecutableLoader::ResolveLibrary(const char* name)
{
	return m_libraryLoader(name);
}

//
