// patches for launcher compatibility

#include "StdInc.h"

DEFINE_INJECT_HOOK(isNonWritableHook, 0xD19516)
{
	Eax(1);
	return DoNowt();
}

static HookFunction hookFunction([] ()
{
	// /GS check
	//hook::return_function(0xD0CDB8);

	// is non-writable check
	isNonWritableHook.inject();

	// icon resource from launcher exe
	hook::put<uint8_t>(0x61CE68, 1);
});