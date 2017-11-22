/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

// Five does not have classical mission cleanup; this is merely a dummy until the new subsystem gets documented
class CMissionCleanup
{
public:
	static fwEvent<> OnCheckCollision;

	static fwEvent<CMissionCleanup*&> OnQueryMissionCleanup;

	inline void CheckIfCollisionHasLoadedForMissionObjects()
	{

	}

	inline void CleanUp(void*)
	{

	}
};

__declspec(selectany) fwEvent<> CMissionCleanup::OnCheckCollision;
__declspec(selectany) fwEvent<CMissionCleanup*&> CMissionCleanup::OnQueryMissionCleanup;