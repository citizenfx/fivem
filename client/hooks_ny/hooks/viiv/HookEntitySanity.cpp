#include "StdInc.h"

static void** g_modelInfoPtrs = (void**)0x15F73B0;

int IsEntityNotNull(char* entity)
{
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

		cmp dword ptr [esi], 0D83BACh
		je dont

		push esi
		call IsEntityNotNull
		add esp, 4h

		test eax, eax
		jz dont

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

		cmp dword ptr [edi], 0D83BACh
		je dont

		push edi
		call IsEntityNotNull
		add esp, 4h

		test eax, eax
		jz dont

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

	// mhm
	hook::return_function(0x818110);
});