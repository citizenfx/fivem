#include "StdInc.h"
#include "BoundStreaming.h"

static RuntimeHookFunction rhFunction("bounds_arent_cdimage", [] ()
{
	hook::jump(0xA722F0, &BoundStreaming::LoadCollision);
	hook::jump(0x8D50A0, &BoundStreaming::ReleaseCollision);

	hook::jump(0xC578F1, &BoundStreaming::LoadAllObjectsTail);

	// unknown stuff relating to img checks
	*(BYTE*)(0xC09C19) = 0xEB;
	*(WORD*)(0xC09EDC) = 0x9090;
	*(WORD*)(0x8D5A38) = 0x9090; // this one is most likely to be wrong
});