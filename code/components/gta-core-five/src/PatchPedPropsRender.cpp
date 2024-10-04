#include <StdInc.h>
#include <Hooking.h>

//
// The game caches a reference to each ped prop renderer when drawing. The cache
// is fixed sized (120) and OneSync allows scenarios where this limit can be
// exceeded.
//
// Legacy Hashes:
//	b2699: zebra-summer-mango
//	b2944: echo-harry-neptune
//
static constexpr uint32_t kMaxPedPropRequests = 100;
static constexpr uint32_t kMaxPedPropRenderers = 240;

static HookFunction hookFunction([]()
{
	// Increase the default size of pool 0x85FB462D
	{
		// 41 B8 32 00 00 00  mov r8d, 32h
		hook::put(hook::get_pattern("BA 2D 46 FB 85 41", 7), kMaxPedPropRequests);
	}

	// Increase the default size of pool 0x539C8EB8
	{
		// 41 B8 78 00 00 00  mov r8d, 78h
		hook::put(hook::get_pattern("BA B8 8E 9C 53 41", 7), kMaxPedPropRenderers);
	}

	// Increase capacity of the four caches (current/previous frame buffers)
	{
		// B8 78 00 00 00  mov eax, 78h
		hook::put(hook::get_pattern("B8 78 00 00 00 48 83 C3 10", 1), kMaxPedPropRenderers);
	}
});
