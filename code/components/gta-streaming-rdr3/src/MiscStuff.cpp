#include <StdInc.h>
#include <Hooking.h>


static HookFunction hookFunction([]()
{
    // reset? entity level cap to PRI_OPTIONAL_LOW
    hook::put<uint8_t>(hook::get_pattern("8D 41 ? 89 0D ? ? ? ? 89 05", 2), 3);
    // init entity level cap to PRI_OPTIONAL_LOW
    hook::put<uint32_t>(hook::get_pattern("B8 ? ? ? ? 83 25 ? ? ? ? ? 48 8B CB", 1), 3);

    // allow low-priority objects even when LOD distance is <20% (not needed if part below enabled)
    // hook::nop(hook::get_pattern("0F 97 05 ? ? ? ? 0F 96 05", 7), 7);

    // disable ability to recalculate a level cap
    hook::nop(hook::get_pattern("89 05 ? ? ? ? 89 05 ? ? ? ? 48 83 C4 ? C3 48 83 EC ? 80 79"), 12);
});