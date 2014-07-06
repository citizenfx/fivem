#include "StdInc.h"
#include "GameInit.h"
#include "HookCallbacks.h"
#include <DrawCommands.h>

static void LoadingScreenFrameWrapper()
{
	HookCallbacks::RunCallback(StringHash("loadsFrame"), nullptr);

	// call loading screen frame
	((void(*)())0x4228D0)();
}

static void AfterFrameChangeHook()
{
	*(float*)0x18A824C = 0;

	if (GameInit::ShouldSwitchToCustomLoad())
	{
		*(DWORD*)0x18A8254 = 1; // switch to load screen '1'
	}
}

static HookFunction hookFunction([] ()
{
	// wrapper for frame event
	hook::jump(0x422360, LoadingScreenFrameWrapper);

	// 'next frame' jump
	/*static hook::inject_call<void, uint8_t, uint8_t> nextLoadFrame(0x4229F4);

	nextLoadFrame.inject([] (uint8_t a, uint8_t b)
	{
		if (GameInit::ShouldSwitchToCustomLoad())
		{
			*(DWORD*)0x18A8254 = 1; // switch to load screen '1'
			return;
		}

		nextLoadFrame.call(a, b);
	});*/

	hook::nop(0x422B5F, 8);
	hook::call(0x422B5F, AfterFrameChangeHook);

	// 'before' draw
	static hook::inject_call<void, int> drawBefore(0x422C76);

	drawBefore.inject([] (int before)
	{
		if (*(DWORD*)0x18A8254 == 1)
		{
			SetTextureGtaIm(GameInit::GetLastCustomLoadTexture());

			int resX = *(int*)0xFDCEAC;
			int resY = *(int*)0xFDCEB0;

			// we need to subtract 0.5f from each vertex coordinate (half a pixel after scaling) due to the usual half-pixel/texel issue
			uint32_t color = 0xFFFFFFFF;
			DrawImSprite(-0.5f, -0.5f, resX - 0.5f, resY - 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
			return;
		}

		drawBefore.call(before);
	});
});