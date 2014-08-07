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
	AdjustLimit("static_bounds", 3500);
	AdjustLimit("transform_matrix", 13000 * 6);
	AdjustLimit("building", 32000 * 8);
	AdjustLimit("drawbldict", 3500);
	AdjustLimit("ptrnode_single", 110000);

	*(DWORD*)(0x5A9509) += 90 * 1024 * 1024;
	*(DWORD*)(0x5A951B) += 90 * 1024 * 1024;
	*(DWORD*)(0x5A9554) += 90 * 1024 * 1024;
});