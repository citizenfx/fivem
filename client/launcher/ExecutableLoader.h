#pragma once

#include <winnt.h>

class ExecutableLoader
{
private:
	const uint8_t* m_origBinary;
	HMODULE m_module;
	uintptr_t m_loadLimit;

	HMODULE(*m_libraryLoader)(const char*);

private:
	void LoadSection(IMAGE_SECTION_HEADER* section);
	void LoadSections(IMAGE_NT_HEADERS* ntHeader);

	void LoadImports(IMAGE_NT_HEADERS* ntHeader);

	HMODULE ResolveLibrary(const char* name);

	template<typename T>
	inline const T* GetRVA(uint32_t rva)
	{
		return (T*)(m_origBinary + rva);
	}

	template<typename T>
	inline T* GetTargetRVA(uint32_t rva)
	{
		return (T*)((uint8_t*)m_module + rva);
	}

public:
	ExecutableLoader(const uint8_t* origBinary);

	inline void SetLoadLimit(uintptr_t loadLimit)
	{
		m_loadLimit = loadLimit;
	}

	inline void SetLibraryLoader(HMODULE(*loader)(const char*))
	{
		m_libraryLoader = loader;
	}

	void LoadIntoModule(HMODULE module);
};