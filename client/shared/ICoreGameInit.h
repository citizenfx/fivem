/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class ICoreGameInit
{
public:
	virtual bool GetGameLoaded() = 0;

	virtual void KillNetwork(const wchar_t* errorString) = 0;

	virtual bool TryDisconnect() = 0;

	virtual void SetPreventSavePointer(bool* preventSaveValue) = 0;

	virtual void LoadGameFirstLaunch(bool(*callBeforeLoad)()) = 0;

	virtual void ReloadGame() = 0;

	virtual bool TriggerError(const char* errorString) = 0;

	bool ShAllowed = false;

public:
	// a1: error message
	fwEvent<const std::string&> OnTriggerError;

	fwEvent<> OnGameFinalizeLoad;

	fwEvent<> OnGameRequestLoad;
};

DECLARE_INSTANCE_TYPE(ICoreGameInit);