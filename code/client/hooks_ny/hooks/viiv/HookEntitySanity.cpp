#include "StdInc.h"

static void** g_modelInfoPtrs = (void**)0x15F73B0;

int IsEntityNotNull(char* entity)
{
	auto mip = g_modelInfoPtrs[*(uint16_t*)(entity + 46)];

	if (*(uint16_t*)(entity + 46) != 0xFFFF)
	{
		if (g_modelInfoPtrs[*(uint16_t*)(entity + 46)] == nullptr)
		{
			return false;
		}
	}

	return true;
}

static DWORD entityCheck2HookDont = 0xB2B407;
static DWORD entityCheck2HookDo = 0xB2B269;

void __declspec(naked) EntityCheck2Hook()
{
	__asm
	{
		cmp [esi + 3Ch], eax
		jz dont

		push esi
		call IsEntityNotNull
		add esp, 4h

		test eax, eax
		jz dont

		cmp dword ptr[esi], 0D83BACh
		je dont

		cmp word ptr [esi + 2Eh], 0FFFFh
		je dont

		jmp entityCheck2HookDo

dont:
		jmp entityCheck2HookDont
	}
}

DWORD entityCheck3Dont = 0x9E7073;

void __declspec(naked) EntityCheck3Hook()
{
	__asm
	{
		cmp ax, 0FFFFh
		je dont

		push edi
		call IsEntityNotNull
		add esp, 4h

		test eax, eax
		jz dont

		cmp dword ptr [edi], 0D83BACh
		je dont

		cmp word ptr [edi + 52h], 0FFFFh
		retn

dont:
		mov eax, entityCheck3Dont
		mov dword ptr [esp], eax
		retn
	}
}

bool __fastcall SomeEntityCheckHook(char* entity)
{
	if (*(uint32_t*)entity == 0xD83BAC) // destructed CEntity
	{
		return true;
	}

	if (*(uint16_t*)(entity + 46) == 0xFFFF)
	{
		return true;
	}

	if (*(uint16_t*)(entity + 46) != 30999)
	{
		if (g_modelInfoPtrs[*(uint16_t*)(entity + 46)] == nullptr)
		{
			return true;
		}
	}

	return ((bool(__fastcall*)(char*))(0x9E7AB0))(entity);
}

static std::set<void*> g_entitySet;

void LogEntityAdd(void* entity)
{
	g_entitySet.insert(entity);
}

void LogEntityRemove(void* entity)
{
	g_entitySet.erase(entity);
}

void LogEntityDestruct(void* entity)
{
	if (g_entitySet.find(entity) != g_entitySet.end())
	{
		// this entity is destroyed without removing!
		__asm int 3
	}
}

static void __declspec(naked) EntityRemoveStub()
{
	__asm
	{
		push ecx
		push ecx

		call LogEntityRemove

		add esp, 4h
		pop ecx

		sub esp, 2Ch
		push ebx
		push ebp

		push 9EA685h
		retn
	}
}

static void __declspec(naked) EntityAddStub()
{
	__asm
	{
		push ecx
		push ecx

		call LogEntityAdd

		add esp, 4h
		pop ecx

		push    ebp
		mov     ebp, esp
		and     esp, 0FFFFFFF0h

		push 9E9E16h
		retn
	}
}

static void __declspec(naked) EntityDestructHook()
{
	__asm
	{
		push esi
		call LogEntityDestruct
		add esp, 4h

		mov ecx, esi

		push 9E86C0h
		retn
	}
}

static void __declspec(naked) SectorCBCheckEntity()
{
	__asm
	{
		push ebx 
		push eax

		push eax
		call IsEntityNotNull
		add esp, 4h

		test eax, eax
		jz dont

		call [ebp + 10h]

	dont:
		add esp, 8h

		retn
	}
}

/*static void EntityAddToSimulationTest(char* entity)
{
	auto mi = g_modelInfoPtrs[*(uint16_t*)(entity + 46)];

	if (*(uint32_t*)((char*)mi + 60) != 913119442)
	{
		//__asm int 3
	}
}

static void __declspec(naked) EntityAddToSimulationProxy()
{
	__asm
	{
		push ecx
		push ecx
		call EntityAddToSimulationTest
		add esp, 4h
		pop ecx

//		push 9E8710h
		retn
	}
}*/

static void __declspec(naked) CreatePhysicsInstanceForMloInst()
{
	__asm
	{
//		int 3

		push 9EAB70h
		retn
	}
}

static void* CheckAndRetnModelInfoForCleanup(int mi)
{
	if (g_modelInfoPtrs[mi])
	{
		return g_modelInfoPtrs[mi];
	}

	static int fake[72 / 4] = { 0 };
	fake[68 / 4] = 1;

	return fake;
}

static void __declspec(naked) CheckEntityStreamCleanup()
{
	__asm
	{
		push eax
		call CheckAndRetnModelInfoForCleanup
		add esp, 4

		cmp dword ptr [eax + 44h], 0
		retn
	}
}

static RuntimeHookFunction esrhf("entity_sanity", [] ()
{
	//hook::jump(0x9E9E10, EntityAddStub);
	//hook::jump(0x9EA680, EntityRemoveStub);

	//hook::call(0x9EA4F2, EntityDestructHook);
	// end debug code

	hook::call(0x7D7849, SomeEntityCheckHook);

	// another crash fix for render list entities
	hook::jump(0xB2B260, EntityCheck2Hook);

	// and yet another one
	hook::call(0x9E7055, EntityCheck3Hook);

	// skip bad entities in sector scans
	hook::nop(0x81825D, 8);
	hook::call(0x81825D, SectorCBCheckEntity);

	// meh
	// overwrites another hook, so bad idea
	//hook::nop(0xAC1CFE, 11);
	//hook::call(0xAC1CFE, CheckEntityStreamCleanup);

	// mhm
	hook::return_function(0x818110);

	//hook::put(0xD88E5C, CreatePhysicsInstanceForMloInst);

	// physics changes to try to get #bd instances to accept phBoundComposite like #bn do
	/*hook::put(0x98F0BF, 2); // phArchetype type?
	hook::put<uint8_t>(0x9EAC19, 5); // phInstGta type

	// jump over some centity::something-update-physics-matrix func that gets invoked on phLevel
	hook::put<uint8_t>(0x9EACDD, 0xEB);

	// nop out some function that gets called on the physics archetype
	hook::nop(0x98F0DA, 5);
	hook::put(0x98F0DA, 0x9004C483);

	// and another one nearby
	hook::put<uint8_t>(0x98EEF9, 0xEB);*/

	//hook::put(0xD88E60, EntityAddToSimulationProxy);
});