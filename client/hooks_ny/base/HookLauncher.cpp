#include "StdInc.h"
#include "Launcher.h"
#include "Hooking.h"

int hook::baseAddressDifference;

static class HookLauncher : public sigslot::has_slots<>
{
public:
	void OnPostLoad(HMODULE, bool*);
} hookLauncher;

void HookLauncher::OnPostLoad(HMODULE gameModule, bool* continueLoad)
{
	hook::set_base((uintptr_t)gameModule);

	HookFunction::RunAll();
}

static InitFunction init([] ()
{
	g_signalPostLoad.connect(&hookLauncher, &HookLauncher::OnPostLoad);
});