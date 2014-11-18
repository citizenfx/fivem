#include "StdInc.h"
#include "Hooking.h"
#include "GlobalEvents.h"

static void DrawFrontendWrap()
{
	((void(*)())0x44CCD0)();

	OnPostFrontendRender();
}

static HookFunction hookFunction([] ()
{
	hook::put(0xE9F1AC, DrawFrontendWrap);
});