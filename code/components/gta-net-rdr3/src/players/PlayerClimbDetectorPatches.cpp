#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Patterns.h>

static void ApplyClimbPoolResize()
{
	static const char* kPoolPatterns[] = {
		"BA 30 00 00 00 48 89 43 08 41 B8 50 14 00 00",
		"BA 30 00 00 00 48 89 43 08 41 B8 50 03 00 00"
	};

	for (const char* patternText : kPoolPatterns)
	{
		auto base = hook::get_pattern<uint8_t>(patternText);

		hook::put<uint32_t>(base + 1, 144);
		hook::put<uint8_t>(base + 0x24, 0x80);
	}
}

static HookFunction hookFunction([]()
{
	ApplyClimbPoolResize();
});
