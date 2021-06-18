#include <StdInc.h>
#include <scrEngine.h>

static uintptr_t rangeStart = (uintptr_t)GetModuleHandle(NULL);
static uintptr_t rangeEnd = rangeStart + 0x6000000;

static bool ValidateArg(void* arg)
{
	if ((uintptr_t)arg >= rangeStart && (uintptr_t)arg < rangeEnd)
	{
		return false;
	}

	return true;
}

void PointerArgumentSafety()
{
// lua53 codegen.lua inp\natives_global.lua pointer_args > ..\..\code\components\rage-scripting-five\src\PASGen.h
#include "PASGen.h"
}
