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

#include <InterfaceMapperBase.h>

class STEAM_EXPORT InterfaceMapper : public InterfaceMapperBase
{
public:
	InterfaceMapper(void* interfacePtr);

private:
	std::unordered_map<std::string, void*> m_methodCache;

private:
	const char* GetMethodName(void* methodPtr);

	void* LookupMethod(const char* methodName);

	void* GetMethodByName(const char* methodName);

public:
	template<typename TReturn, typename... TArgs>
	TReturn Invoke(const char* methodName, TArgs... args)
	{
		void* method = GetMethodByName(methodName);

		return ((TReturn(__thiscall*)(void*, TArgs...))method)(m_interface, args...);
	}
};

extern bool g_isOldSpawnProcess;