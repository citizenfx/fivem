#include "StdInc.h"

struct CStreamingType
{
	uint32_t pad;
	void*(*getAt)(int index);
	void*(*defrag)(void* unknown, void* blockMap);
	uint32_t pad2;
	void(*releaseData)(int index);
	int(*getIndexByName)(const char* name);
	void(*unkFunc)();
	void(*addRef)(int index);
	void(*release)(int index);
	int(*getUsageCount)(int index);
	int(*getParents)(int index, int* parents);
	void(*onLoad)(int index, void* blockMap, int a3);
	void(*setData)(int index, void* physicalData);
	char typeName[32];
	char ext[4];
	int startIndex;
	uint32_t pad3;
	uint32_t fileVersion;
};

struct CStreamingTypeManager
{
	CStreamingType types[25];
};

CStreamingTypeManager& streamingTypes = *(CStreamingTypeManager*)0x1227F40;

static std::unordered_map<int, std::string> g_indexToName;

std::string GetStreamName(int idx, int typeIdx)
{
	return g_indexToName[(typeIdx << 24) | idx];
}

int GetTypeStart(int type)
{
	return streamingTypes.types[type].startIndex;
}

int GetTypeEnd(int type)
{
	return streamingTypes.types[type + 1].startIndex - 1;
}

extern char curImgLabel[256];
 
static int RegisterFileName(const char* name, int type)
{
	name = curImgLabel;

	int typeIdx = type / 100;
	int modelIdx = streamingTypes.types[typeIdx].getIndexByName(name);

	trace("Registering model %s as %s #%d\n", name, streamingTypes.types[typeIdx].typeName, modelIdx);

	g_indexToName[(typeIdx << 24) | modelIdx] = name;

	return modelIdx;
}

static void __declspec(naked) StoreImgEntryNameStub()
{
	__asm
	{
		push esi
		push eax

		call RegisterFileName

		add esp, 8h

		retn
	}
}

static HookFunction hookFunction([] ()
{
	// img index entry add function, call to file handler registration
	hook::nop(0xBCC370, 6);
	hook::call(0xBCC370, StoreImgEntryNameStub);
});