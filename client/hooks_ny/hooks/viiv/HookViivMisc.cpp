#include "StdInc.h"

static RuntimeHookFunction deadlock1("modelinfo_deadlock_hack", [] ()
{
	hook::nop(0xA4C0B9, 5);
	hook::nop(0xA4C02C, 5);
});

static DWORD buildingConstruct = 0xB1B3E0;

char* __fastcall BuildingConstructHook(char* building)
{
	building = ((char*(__fastcall*)(char*))buildingConstruct)(building);

	*(DWORD*)(building + 36) |= 0x40;

	return building;
}

#include "Hooking.FuncCalls.h"

static RuntimeHookFunction boundSanity("static_bound_sanity", [] ()
{
	hook::call(0xC09852, BuildingConstructHook);

	/*hook::thiscall<char*, char>::inject(0xC09852, [] (char* self)
	{
		return self;
	});*/
});

static RuntimeHookFunction noDistantlights("no_distantlights", [] ()
{
	hook::return_function(0x902430);
});

static RuntimeHookFunction oddWaitDeadlock("odd_wait_deadlock", [] ()
{
	// some weird internal wait loop
	hook::put<WORD>(0x7BC56C, 0x9090);
	*(BYTE*)(0x7BC57A) = 0xEB;
	*(WORD*)(0x7BC555) = 0x9090;

	// skip another wait loop more internally
	*(BYTE*)(0x7BC42E) = 0xEB;

	// and a wait loop wrt streaming
	hook::put<uint16_t>(0x5B161F, 0x9090);

	// 'crashing' stuff
	//hook::put<uint8_t>(0x62F520, 0xCC);
	//hook::put<uint8_t>(0x622430, 0xCC);

	// don't destroy some windowed stuff which somehow only triggers when not debugged
	//hook::put<uint8_t>(0x6224AE, 0xCC);
	hook::nop(0x6224AE, 49);
	hook::nop(0x6224E7, 6);

	hook::put<uint8_t>(0x62F520, 0xC3);

	hook::return_function(0x621D10);
});

/*
static HookFunction hookFunction([] ()
{
	hook::thiscall<void, float, float*>::inject(0x8D6736, [] (float* a1, float* a2)
	{
		__asm int 3
	});
});
*/