// allows confirming a 'SET_MSG_FOR_LOADING_SCREEN' even if the game isn't set to reload
#include "StdInc.h"
#include "HookCallbacks.h"
#include <GlobalEvents.h>

static bool& drawingLoadMsg = *(bool*)0x11DE801;

static bool WRAPPER ConfirmMenuResult(int a1, int a2, int a3, int a4, int a5, int a6, int a7) { EAXJMP(0x788F30); }

static HookFunction hookFunction([] ()
{
	static hook::inject_call<void, int> injectCall(0x4212F0);

	injectCall.inject([] (int)
	{
		if (drawingLoadMsg)
		{
			if (ConfirmMenuResult(8, 1, 2, 1, 0, 0, 0))
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

	static hook::inject_call<void, int> leaveGameCall(0xB5F600);

	leaveGameCall.inject([] (int)
	{
		OnMsgConfirm();

		leaveGameCall.call();
	});
});