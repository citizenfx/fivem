/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class InterfaceMapper
{
public:
	InterfaceMapper(void* interfacePtr);

private:
	std::unordered_map<std::string, void*> m_methodCache;

	void* m_interface;

	uintptr_t m_moduleValidStart;
	uintptr_t m_moduleValidEnd;

	uintptr_t m_moduleValidDataStart;
	uintptr_t m_moduleValidDataEnd;

private:
	void UpdateCachedModule();

	const char* GetMethodName(void* methodPtr);

	void* LookupMethod(const char* methodName);

	void* GetMethodByName(const char* methodName);

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

public:
	template<typename TReturn, typename... TArgs>
	TReturn Invoke(const char* methodName, TArgs... args)
	{
		void* method = GetMethodByName(methodName);

		return ((TReturn(__thiscall*)(void*, TArgs...))method)(m_interface, args...);
	}

	inline bool IsValid()
	{
		return m_interface != nullptr;
	}
};