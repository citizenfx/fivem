#include <StdInc.h>
#include <Hooking.h>
#include <CrossBuildRuntime.h>

// By default, the game only allows a YMAP to have 6 YTYP dependencies in the manifest, the ones that come after just get ignored.
// This can be problematic as for custom YMAPs you're very likely to use assets from more than 6 different YTYPs.

static HookFunction hookFunction([]()
{
    auto pattern = hook::pattern("0F B7 41 26 8B EA 48 8B F9 66 83 F8 06");
    if (!pattern.empty())
    {
        auto location = pattern.get_first<unsigned char>(12);
        hook::put<uint8_t>(location, 0x10);  // 6 -> 16
    }
    else
    {
        auto pattern2 = hook::pattern("66 83 F8 06 73");
        if (!pattern2.empty())
        {
            auto location = pattern2.get_first<unsigned char>(3);
            hook::put<uint8_t>(location, 0x10);  // 6 -> 16
        }
    }
    auto allocPattern = hook::pattern("8B 59 18 B9 18 00 00 00 E8");
    if (!allocPattern.empty())
    {
        auto location = allocPattern.get_first<unsigned char>(4);
        hook::put<uint8_t>(location, 0x40);
    }
});