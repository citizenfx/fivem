#include "StdInc.h"

void AdjustLimit(std::string limit, int value)
{
	if (limit == "static_bounds")
	{
		hook::put(0xC0A6B2, value);
	}
	else if (limit == "transform_matrix")
	{
		hook::put(0xA4C011, value);
	}
	else if (limit == "building")
	{
		hook::put(0xB1B486, value);
	}
	else if (limit == "drawbldict")
	{
		hook::put(0x907FE6, value);
		hook::put(0x90803E, value);
	}
	else if (limit == "ptrnode_single")
	{
		hook::put(0xB534B6, value);
	}
}

static HookFunction hookFunction([] ()
{
	AdjustLimit("static_bounds", 5800);
	AdjustLimit("transform_matrix", 13000 * 6);
	AdjustLimit("building", 32000 * 8);
	AdjustLimit("drawbldict", 7500);
	AdjustLimit("ptrnode_single", 230000);

	// txdstore count
	hook::put(0x820DB7, 7500);
	hook::put(0x820E2F, 6500);

	// placeable matrix?
	hook::put(0x9E79FC, 52000);

	// global ipl pointer count used during loading
	hook::put(0x8D7153, 296608); // was 32768

	*(DWORD*)(0x5A9509) += 90 * 1024 * 1024;
	*(DWORD*)(0x5A951B) += 90 * 1024 * 1024;
	*(DWORD*)(0x5A9554) += 90 * 1024 * 1024;

	// increase allowed amount of filesystem mounts (fixes CreatePlayer exception @ 0x69ECB2 due to player:/ not being mounted)
	hook::put(0x4022E6, 256); // was 64

	// don't kill the networked player when out of range
	hook::return_function(0x878F70);
});