#include "StdInc.h"

#ifndef IS_FXSERVER
#include "Hooking.h"
#include "Hooking.Stubs.h"

#include "../../../vendor/minhook/include/MinHook.h"

namespace hook
{
void trampoline_raw(void* address, const void* target, void** origTrampoline)
{
	static auto mhInitializer = ([] {
		return MH_Initialize();
	})();

	auto location = reinterpret_cast<void*>(hook::get_adjusted(address));
	MH_CreateHook(location, const_cast<void*>(target), origTrampoline);
	assert(MH_EnableHook(location) == MH_OK);
}
}
#endif
