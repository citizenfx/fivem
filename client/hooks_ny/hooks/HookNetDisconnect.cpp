// handles native network disconnection
#include "StdInc.h"
#include "CrossLibraryInterfaces.h"

static void FinalizeDisconnect()
{
	g_netLibrary->FinalizeDisconnect();
}

static HookFunction hookFunction([] ()
{
	// tail of disconnect func
	hook::jump(0x463C70, FinalizeDisconnect);
});