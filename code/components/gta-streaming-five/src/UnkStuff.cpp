#include <StdInc.h>
#include <Hooking.h>

static int ReturnTrue()
{
	return 1;
}

static HookFunction hookFunction([] ()
{
#if 0
	hook::jump(hook::pattern("48 8B 48 08 48 85 C9 74  0C 8B 81").count(1).get(0).get<char>(-0x10), ReturnTrue);
	hook::put<uint8_t>(hook::pattern("80 3D ? ? ? ? ? 48 8B F1 74 07 E8 ? ? ? ? EB 05").count(1).get(0).get<void>(0xA), 0xEB);
	hook::put<uint8_t>(hook::pattern("0F 8E ? ? 00 00 80 3D ? ? ? ? 00 74 07 E8").count(1).get(0).get<void>(0xD), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 12 48 FF C3 48 83 C0 04 48 81 FB").count(1).get(0).get<void>(-0xB), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 63 45 8D 47 02 E8").count(1).get(0).get<void>(0), 0xEB);

	/*{
		auto m = hook::pattern("44 89 7C 24 60 81 FA DB  3E 14 7D 74 16").count(1).get(0);

		hook::put<uint8_t>(m.get<void>(5), 0x90);
		hook::put<uint8_t>(m.get<void>(6), 0xBA);
		hook::put<uint32_t>(m.get<void>(7), HashString("meow"));
		hook::nop(m.get<void>(11), 2);
	}

	{
		auto m = hook::pattern("44 89 44 24 60 81 FA DB  3E 14 7D 74 16").count(1).get(0);

		hook::put<uint8_t>(m.get<void>(5), 0x90);
		hook::put<uint8_t>(m.get<void>(6), 0xBA);
		hook::put<uint32_t>(m.get<void>(7), HashString("meow"));
		hook::nop(m.get<void>(11), 2);
	}*/

	hook::nop(hook::pattern("0F B6 05 ? ? ? ? 40 8A BB").count(1).get(0).get<char>(0), 0x7);
	hook::put<uint8_t>(hook::pattern("48 83 C6 04 49 2B EC 75 CB").count(1).get(0).get<void>(0x15), 0xEB);

	hook::put<uint8_t>(hook::pattern("F6 05 ? ? ? ? ? 74 08 84 C0 0F 84").count(1).get(0).get<void>(0x18), 0xEB);
#endif

	// increase the heap size for allocator 0
	hook::put<uint32_t>(hook::get_pattern("83 C8 01 48 8D 0D ? ? ? ? 41 B1 01 45 33 C0", 17), 600 * 1024 * 1024); // 600 MiB, default in 323 was 412 MiB

	// don't pass flag 4 for streaming requests of netobjs
	hook::put<int>(hook::get_pattern("BA 06 00 00 00 41 23 CE 44 33 C1 44 23 C6 41 33", 1), 2);
});