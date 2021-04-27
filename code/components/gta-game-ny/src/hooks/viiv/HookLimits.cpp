#include "StdInc.h"

void AdjustLimit(std::string limit, int value)
{
	if (limit == "static_bounds")
	{
		hook::put(hook::pattern("6A 03 68 ? ? ? ? 68 F4 01 00 00 E8").count(2).get(1).get<void*>(8), value);
	}
	else if (limit == "transform_matrix")
	{
		hook::put(hook::get_pattern("68 C8 32 00 00", 1), value);
	}
	else if (limit == "building")
	{
		hook::put(hook::get_pattern("68 00 7D 00 00", 1), value);
	}
	else if (limit == "drawbldict")
	{
		auto loc = hook::pattern("68 00 09 00 00").count(3);
		hook::put(loc.get(1).get<void*>(1), value);
		hook::put(loc.get(2).get<void*>(1), value);
	}
	else if (limit == "ptrnode_single")
	{
		hook::put(hook::get_pattern("68 80 38 01 00", 1), value);
	}
}

static HookFunction hookFunction([] ()
{
	AdjustLimit("static_bounds", 5800);
	AdjustLimit("transform_matrix", 13000 * 6);
	AdjustLimit("building", 32000 * 8);
	AdjustLimit("drawbldict", 7500);
	AdjustLimit("ptrnode_single", 230000);

	{
		// txdstore count
		auto loc = hook::pattern("68 70 17 00 00").count(11);
		hook::put(loc.get(1).get<void*>(1), 7500);
		hook::put(loc.get(2).get<void*>(1), 6500);
	}

	// placeable matrix?
	hook::put(hook::get_pattern("68 58 1B 00 00 B9", 1), 52000);

	// global ipl pointer count used during loading
	hook::put(hook::get_pattern("68 00 80 00 00 E8", 1), 296608); // was 32768

	{
		auto loc = hook::pattern("68 00 00 40 06").count(3);
		*(DWORD*)(loc.get(0).get<void*>(1)) += 90 * 1024 * 1024;
		*(DWORD*)(loc.get(1).get<void*>(1)) += 90 * 1024 * 1024;
		*(DWORD*)(loc.get(2).get<void*>(1)) += 90 * 1024 * 1024;
	}

	// don't kill the networked player when out of range
	hook::return_function(hook::get_pattern("F6 81 6C 02 00 00 04 0F 28 CA", -0x42));
});
