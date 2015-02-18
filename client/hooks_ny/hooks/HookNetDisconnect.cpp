// handles native network disconnection
#include "StdInc.h"
#include "CrossLibraryInterfaces.h"

#include <ICoreGameInit.h>

static bool g_disconnectSafeguard;

void SetDisconnectSafeguard(bool enable)
{
	g_disconnectSafeguard = enable;
}

static void FinalizeDisconnect()
{
	ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

	if (!g_disconnectSafeguard)
	{
		if (gameInit->TryDisconnect())
		{
			g_netLibrary->FinalizeDisconnect();
		}
	}
}

static HookFunction hookFunction([] ()
{
	// tail of disconnect func
	hook::jump(0x463C70, FinalizeDisconnect);
});