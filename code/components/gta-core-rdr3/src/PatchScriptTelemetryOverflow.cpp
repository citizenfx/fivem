#include <StdInc.h>
#include <Hooking.h>

//
// One of the telemetry metrics collects the script hash of every network script handler, appending
// one dword per handler into a statically allocated metric buffer. The loop is bounded by the pool
// size instead of the buffer capacity, and the buffer only has room for 55 entries.
//
// Every resource gets a network script handler, so any server with more than 55 resources writes
// past the end of the metric and into whatever the linker placed after it. It stays unnoticed for a
// while because the first few hundred bytes land inside the next metric, but around 270 resources
// the damage reaches the script handler singletons and the game crashes dereferencing one of them.
//

static HookFunction hookFunction([]()
{
	auto location = hook::get_pattern<uint8_t>("83 E1 1F 48 C1 C2 20 FF C1 48 D3 C2 84 C0 74 ? 48 F7 D2 3B 72 18 0F 8D", 0x13);

	hook::put<uint8_t>(location, 0x83);
	hook::put<uint8_t>(location + 1, 0xFE);
	hook::put<uint8_t>(location + 2, 0x37);
});
