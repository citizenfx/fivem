#include "StdInc.h"
#include "sysAllocator.h"

#undef NDEBUG
#include <assert.h>

struct CStreamingInfo
{
	int pad[4];
	uint16_t nextIndex;
	uint16_t prevIndex;
	int pad2;

	void AddToList(CStreamingInfo* list);
};

CStreamingInfo*& g_streamingInfo = *(CStreamingInfo**)0x16CF28C;

int& g_streamingCount = *(int*)0xF21C64;

static void __fastcall RemoveStreamingEntry(CStreamingInfo* info)
{
	uint16_t thisIndex = info - g_streamingInfo;

	if (info->prevIndex != 0xFFFF)
	{
		g_streamingInfo[info->prevIndex].nextIndex = info->nextIndex;
	}

	if (info->nextIndex != 0xFFFF)
	{
		g_streamingInfo[info->nextIndex].prevIndex = info->prevIndex;
	}

	if (thisIndex >= 0x1C00 && thisIndex <= 0x1D00)
	{
		//trace("%d removed from %p\n", (int)thisIndex, _ReturnAddress());
	}

	info->nextIndex = -1;
	info->prevIndex = -1;
}

void CStreamingInfo::AddToList(CStreamingInfo* list)
{
	uint16_t thisIndex = this - g_streamingInfo;

	if (thisIndex >= 0x1C00 && thisIndex <= 0x1D00)
	{
		//trace("%d added to %d from %p\n", (int)thisIndex, list - g_streamingInfo, _ReturnAddress());
	}

	assert(prevIndex == 0xFFFF);
	assert(nextIndex == 0xFFFF);
	
	/*if (prevIndex != 0xFFFF)
	{
		trace("index %d prev index trail: ", (int)thisIndex);

		uint16_t idx = prevIndex;

		while (idx != 0xFFFF)
		{
			trace("%d ", (int)idx);

			idx = g_streamingInfo[idx].prevIndex;
		}

		trace("\n");

		assert(!"streaming info added to list twice");
	}*/

	nextIndex = list->nextIndex;
	prevIndex = list - g_streamingInfo;

	list->nextIndex = thisIndex;
	g_streamingInfo[nextIndex].prevIndex = thisIndex;
}

static void __fastcall RemoveStreamingEntryTail(CStreamingInfo* info)
{


	uint16_t thisIndex = info - g_streamingInfo;

	for (int i = 0; i < 0xC3A7; i++)
	{
		/*if (g_streamingInfo[i].prevIndex == thisIndex)
		{
			__asm int 3
		}*/

		if (g_streamingInfo[i].nextIndex == thisIndex)
		{
			//g_streamingInfo[i].nextIndex = oldNextIndex;
		}
	}
}

static void FreeStreamingIndices(void** info)
{
	if (*info)
	{
		rage::GetAllocator()->free(*info);
	}
}

static void __declspec(naked) FreeStreamingIndicesStub()
{
	__asm
	{
		push esi
		call FreeStreamingIndices
		add esp, 4h

		retn
	}
}

static HookFunction hookFunction([] ()
{
	uintptr_t dwFunc;

	__asm mov dwFunc, offset CStreamingInfo::AddToList

	hook::jump(0xBCBB00, dwFunc);

	hook::jump(0xBCBB70, RemoveStreamingEntry);

	hook::nop(0xBCBD51, 6);
	hook::call(0xBCBD51, FreeStreamingIndicesStub);
	//hook::jump(0xBCBBAB, RemoveStreamingEntryTail);
});