#pragma once

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

extern GAMESPEC_EXPORT CStreamingTypeManager& streamingTypes;