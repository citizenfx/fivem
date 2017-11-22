/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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