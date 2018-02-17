#include <StdInc.h>
#include <Hooking.h>

#include <netBlender.h>

static hook::cdecl_stub<void(rage::netBlender*, uint32_t timeStamp)> netBlender_SetTimestamp([]()
{
	return hook::get_pattern("48 8B D9 39 79 18 74 76 48", -0x13);
});

namespace rage
{
void netBlender::SetTimestamp(uint32_t timestamp)
{
	return netBlender_SetTimestamp(this, timestamp);
}
}
