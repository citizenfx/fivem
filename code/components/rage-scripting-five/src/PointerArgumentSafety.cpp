#include <StdInc.h>
#include <scrEngine.h>

#include <ICoreGameInit.h>

static uintptr_t rangeStart = (uintptr_t)GetModuleHandle(NULL);
static uintptr_t rangeEnd = rangeStart + 0x6000000;
static ICoreGameInit* icgi;
bool storyMode;

static bool ValidateArg(void* arg)
{
	if ((uintptr_t)arg >= rangeStart && (uintptr_t)arg < rangeEnd)
	{
		if (!storyMode)
		{
			return false;
		}
	}

	return true;
}

void PointerArgumentSafety()
{
	icgi = Instance<ICoreGameInit>::Get();
	icgi->OnSetVariable.Connect([](const std::string& name, bool value)
	{
		if (name == "storyMode")
		{
			storyMode = value;
		}
	});

// lua53 codegen.lua inp\natives_global.lua pointer_args > ..\..\code\components\rage-scripting-five\src\PASGen.h
#include "PASGen.h"
}
