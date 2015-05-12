/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UserLibrary.h"

UserLibrary::UserLibrary(const wchar_t* fileName)
{
	FILE* f = _wfopen(fileName, L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);
		m_libraryBuffer.resize(ftell(f));

		fseek(f, 0, SEEK_SET);
		fread(&m_libraryBuffer[0], 1, m_libraryBuffer.size(), f);

		fclose(f);
	}
}

const uint8_t* UserLibrary::GetExportCode(const char* getName) const
{
	// get the DOS header
	IMAGE_DOS_HEADER* header = (IMAGE_DOS_HEADER*)&m_libraryBuffer[0];

	if (header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return nullptr;
	}

	// get the NT header
	const IMAGE_NT_HEADERS* ntHeader = (const IMAGE_NT_HEADERS*)&m_libraryBuffer[header->e_lfanew];

	// find the export directory
	auto exportDirectoryData = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

	// get the export directory
	const IMAGE_EXPORT_DIRECTORY* exportDirectory = (const IMAGE_EXPORT_DIRECTORY*)GetOffsetPointer(exportDirectoryData.VirtualAddress);

	const uint32_t* names = (const uint32_t*)GetOffsetPointer(exportDirectory->AddressOfNames);
	const uint16_t* ordinals = (const uint16_t*)GetOffsetPointer(exportDirectory->AddressOfNameOrdinals);
	const uint32_t* functions = (const uint32_t*)GetOffsetPointer(exportDirectory->AddressOfFunctions);

	for (int i = 0; i < exportDirectory->NumberOfNames; i++)
	{
		const char* name = (const char*)GetOffsetPointer(names[i]);

		if (_stricmp(name, getName) == 0)
		{
			return GetOffsetPointer(functions[ordinals[i]]);
		}
	}

	return nullptr;
}

const uint8_t* UserLibrary::GetOffsetPointer(uint32_t offset) const
{
	// get the DOS header
	const IMAGE_DOS_HEADER* header = (const IMAGE_DOS_HEADER*)&m_libraryBuffer[0];

	// get the NT header
	const IMAGE_NT_HEADERS* ntHeader = (const IMAGE_NT_HEADERS*)&m_libraryBuffer[header->e_lfanew];

	// loop through each sections to find where our offset is
	const IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(ntHeader);

	for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		uint32_t curRaw = sections[i].PointerToRawData;
		uint32_t curVirt = sections[i].VirtualAddress;

		if (offset >= curVirt && offset < curVirt + sections[i].SizeOfRawData)
		{
			offset -= curVirt;
			offset += curRaw;

			return &m_libraryBuffer[offset];
		}
	}

	return nullptr;
}