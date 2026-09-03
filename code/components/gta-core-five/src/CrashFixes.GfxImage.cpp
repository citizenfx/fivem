#include "StdInc.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

static char imageInfoOffset;

static void (*g_origGfxShapeSetToImage)(void* self, hook::FlexStruct* image, bool bilinear);
static void GfxShapeSetToImage(void* self, hook::FlexStruct* image, bool bilinear)
{
	if (!image || !image->Get<void*>(imageInfoOffset))
	{
		return;
	}

	g_origGfxShapeSetToImage(self, image, bilinear);
}

static HookFunction hookFunction([]()
{
	imageInfoOffset = *(char*)hook::get_pattern("48 8B 4A ? 0F 29 78 ? 48 8B 01", 0x3);
	g_origGfxShapeSetToImage = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 39 7D ? 74 ? 48 85 FF")), GfxShapeSetToImage);
});