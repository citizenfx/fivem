// allows confirming a 'SET_MSG_FOR_LOADING_SCREEN' even if the game isn't set to reload
#include "StdInc.h"
#include <Hooking.h>
#include <GlobalEvents.h>

static bool* drawingLoadMsg;

static hook::cdecl_stub<bool(int, int, int, int, int, int, int)> _confirmMenuResult([]()
{
	return hook::get_pattern("83 EC 0C 80 3D ? ? ? ? 00 56 8B");
});

static HookFunction hookFunction([] ()
{
	return;

	drawingLoadMsg = *hook::get_pattern<bool*>("CC 0F B6 ? ? ? ? ? FF 74 24 04 33 C9 38", 0x41);

	static hook::inject_call<void, int> injectCall((ptrdiff_t)hook::get_pattern("8b ec 83 e4 f0 83 ec 18 b9 ? ? ? ? 53 56", 65));

	injectCall.inject([] (int)
	{
		if (drawingLoadMsg)
		{
			if (_confirmMenuResult(8, 1, 2, 1, 0, 0, 0))
			{
				//HookCallbacks::RunCallback(StringHash("msgConfirm"), nullptr);
				OnMsgConfirm();

				drawingLoadMsg = false;
			}
		}
		else
		{
			injectCall.call();
		}
	});

	static hook::inject_call<void, int> leaveGameCall(*(uintptr_t*)(hook::get_call(*hook::get_pattern<uintptr_t>("68 ? ? ? ? 68 24 6E D6 55", 1))));

	leaveGameCall.inject([] (int)
	{
		OnMsgConfirm();

		leaveGameCall.call();
	});
});
