/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

typedef void* (*CreateInterface_t)(const char* pName, int* pReturnCode);

class SteamLoader
{
private:
	HMODULE m_hSteamClient;

	uint32_t GetSteamProcessId();

	std::wstring GetSteamDllPath();

	void LoadGameOverlayRenderer(const std::wstring& baseDllPath);

	void* GetProcAddressInternal(const char* name);

public:
	void Initialize();

	bool IsSteamRunning(bool ignoreCreateFunc);

	template<typename TProcedure>
	TProcedure GetProcAddress(const char* name)
	{
		return reinterpret_cast<TProcedure>(GetProcAddressInternal(name));
	}

	CreateInterface_t GetCreateInterfaceFunc();
};