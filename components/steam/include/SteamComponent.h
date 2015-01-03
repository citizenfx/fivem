/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "SteamLoader.h"
#include "IClientEngine.h"

class SteamComponent
{
private:
	SteamLoader m_steamLoader;

	IClientEngine* m_clientEngine;

	HSteamPipe m_steamPipe;
	HSteamUser m_steamUser;

private:
	bool RunPresenceDummy();

	void InitializePresence();

public:
	void Initialize();	
};