/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
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