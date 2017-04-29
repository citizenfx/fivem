/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_STEAM
#define STEAM_EXPORT __declspec(dllexport)
#else
#define STEAM_EXPORT
#endif

class STEAM_EXPORT InterfaceMapperBase
{
public:
	InterfaceMapperBase(void* interfacePtr);

protected:
	void* m_interface;

private:
	uintptr_t m_moduleStart;

	uintptr_t m_moduleValidStart;
	uintptr_t m_moduleValidEnd;

	uintptr_t m_moduleValidDataStart;
	uintptr_t m_moduleValidDataEnd;

protected:
	inline uintptr_t GetModuleStart()
	{
		return m_moduleStart;
	}

	inline bool IsValidCodePointer(void* ptr)
	{
		register uintptr_t pointer = reinterpret_cast<uintptr_t>(ptr);

		return (pointer >= m_moduleValidStart && pointer <= m_moduleValidEnd);
	}

	inline bool IsValidDataPointer(void* ptr)
	{
		register uintptr_t pointer = reinterpret_cast<uintptr_t>(ptr);

		return (pointer >= m_moduleValidDataStart && pointer <= m_moduleValidDataEnd);
	}

	void UpdateCachedModule();

public:
	inline bool IsValid()
	{
		return m_interface != nullptr;
	}
};