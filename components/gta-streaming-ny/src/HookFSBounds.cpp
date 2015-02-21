/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "BoundStreaming.h"
#include "Hooking.h"
#include <BaseResourceScripting.h>

bool SetStreamingWbnMode(bool fs);

//static RuntimeHookFunction rhFunction("bounds_arent_cdimage", [] ()
static InitFunction initFunction([]()
{
	OnSetWorldAssetConfig.Connect([] (fwString type, bool value)
	{
		if (value && type == "bounds_arent_cdimage")
		{
			hook::jump(0xA722F0, &BoundStreaming::LoadCollision);
			hook::jump(0x8D50A0, &BoundStreaming::ReleaseCollision);

			hook::jump(0xC578F1, &BoundStreaming::LoadAllObjectsTail);

			// unknown stuff relating to img checks
			//*(BYTE*)(0xC09C19) = 0xEB;
			//*(WORD*)(0xC09EDC) = 0x9090;
			*(WORD*)(0x8D5A38) = 0x9090; // this one is most likely to be wrong

			// enable WBN mode
			SetStreamingWbnMode(true);
		}
	});
});