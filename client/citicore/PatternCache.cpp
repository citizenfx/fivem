/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

void DLL_EXPORT Citizen_PatternSaveHint(uint64_t hash, uintptr_t hint)
{
	fwPlatformString hintsFile = MakeRelativeCitPath(L"citizen\\hints.dat");
	FILE* hints = _pfopen(hintsFile.c_str(), _P("ab"));

	if (hints)
	{
		fwrite(&hash, 1, sizeof(hash), hints);
		fwrite(&hint, 1, sizeof(hint), hints);

		fclose(hints);
	}
}