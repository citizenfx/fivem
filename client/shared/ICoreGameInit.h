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
};

DECLARE_INSTANCE_TYPE(ICoreGameInit);