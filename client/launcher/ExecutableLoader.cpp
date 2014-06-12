#include "StdInc.h"
#include "ExecutableLoader.h"

ExecutableLoader::ExecutableLoader(const uint8_t* origBinary)
{
	m_origBinary = origBinary;
	m_loadLimit = UINT_MAX;
	
	SetLibraryLoader([] (const char* name)
	{
		return LoadLibraryA(name);
	});
}

void ExecutableLoader::LoadImports(IMAGE_NT_HEADERS* ntHeader)
{
	IMAGE_DATA_DIRECTORY* importDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	auto descriptor = GetRVA<IMAGE_IMPORT_DESCRIPTOR>(importDirectory->VirtualAddress);

	while (descriptor->Name)
	{
		const char* name = GetRVA<char>(descriptor->Name);

		HMODULE module = ResolveLibrary(name);

		if (!module)
		{
			FatalError("Could not load dependent module %s. Error code was %i.\n", name, GetLastError());
		}

		// "don't load"
		if (reinterpret_cast<uint32_t>(module) == 0xFFFFFFFF)
		{
			descriptor++;
			continue;
		}

		auto nameTableEntry = GetRVA<uint32_t>(descriptor->OriginalFirstThunk);
		auto addressTableEntry = GetTargetRVA<uint32_t>(descriptor->FirstThunk);

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
				auto import = GetRVA<IMAGE_IMPORT_BY_NAME>(*nameTableEntry);

				function = GetProcAddress(module, import->Name);
				functionName = import->Name;
			}			

			if (!function)
			{
				FatalError("Could not load function %s in dependent module %s.\n", functionName, name);
			}

			*addressTableEntry = (uint32_t)function;

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

	if ((uintptr_t)targetAddress >= m_loadLimit)
	{
		return;
	}

	if (section->SizeOfRawData > 0)
	{
		memcpy(targetAddress, sourceAddress, section->SizeOfRawData);

		DWORD oldProtect;
		VirtualProtect(targetAddress, section->SizeOfRawData, PAGE_EXECUTE_READWRITE, &oldProtect);
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

void ExecutableLoader::LoadIntoModule(HMODULE module)
{
	m_module = module;

	IMAGE_DOS_HEADER* header = (IMAGE_DOS_HEADER*)m_origBinary;

	if (header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return;
	}

	IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(m_origBinary + header->e_lfanew);

	LoadSections(ntHeader);
	LoadImports(ntHeader);

	// copy over TLS index (source in this case indicates the TLS data to copy from, which is the launcher app itself)
	IMAGE_DOS_HEADER* sourceHeader = (IMAGE_DOS_HEADER*)module;
	IMAGE_NT_HEADERS* sourceNtHeader = GetTargetRVA<IMAGE_NT_HEADERS>(sourceHeader->e_lfanew);

	const IMAGE_TLS_DIRECTORY* targetTls = GetRVA<IMAGE_TLS_DIRECTORY>(ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
	const IMAGE_TLS_DIRECTORY* sourceTls = GetTargetRVA<IMAGE_TLS_DIRECTORY>(sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

	*(DWORD*)(targetTls->AddressOfIndex) = *(DWORD*)(sourceTls->AddressOfIndex);

	// copy over the offset to the new imports directory
	DWORD oldProtect;
	VirtualProtect(sourceNtHeader, 0x1000, PAGE_EXECUTE_READWRITE, &oldProtect);

	sourceNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT] = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
}

HMODULE ExecutableLoader::ResolveLibrary(const char* name)
{
	return m_libraryLoader(name);
}