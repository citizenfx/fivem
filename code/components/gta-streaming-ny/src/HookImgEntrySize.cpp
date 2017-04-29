/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

char curImgLabel[256];

void strncpyThing(char*, const char* a, int)
{
	strncpy(curImgLabel, a, sizeof(curImgLabel));
	curImgLabel[255] = '\0';
}

char* strrchrThing(char*, int a)
{
	return strrchr(curImgLabel, a);
}

DWORD pushThingRet = 0xBCC373;

static std::unordered_map<uint32_t, std::string> g_imgNameMap;

void SetRecordedImgName(uint32_t imgIndex, uint32_t fileTypeIndex)
{
	DWORD* imgBases = (DWORD*)(0x1227F98);

	g_imgNameMap[imgIndex + imgBases[fileTypeIndex / 4]] = curImgLabel;
}

void __declspec(naked) pushThing()
{
	__asm
	{
		mov eax, offset curImgLabel
		push eax
		//jmp pushThingRet
		call ecx

		push esi
		push eax
		call SetRecordedImgName
		pop eax
		add esp, 4h

		jmp pushThingRet
	}
}


static HookFunction imgEntrySize([] ()
{
	hook::call(0xBCC2FD, strncpyThing);

	hook::call(0xBCC30E, strrchrThing);

	//hook::jump(0xBCC36C, pushThing);
});