/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Launcher.h"
#include "Hooking.h"

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