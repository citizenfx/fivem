#pragma once

struct CStreamingModule
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

struct CStreamingModuleManager
{
	CStreamingModule modules[25];
	int moduleCount;

	inline int GetModuleFromExt(const char* ext)
	{
		if (moduleCount > 0)
		{
			for (int i = 0; i < moduleCount; i++)
			{
				if (_strnicmp(modules[i].ext, ext, 3) == 0)
				{
					return i;
				}
			}
		}

		return 0xFF;
	}
};

extern GAMESPEC_EXPORT CStreamingModuleManager* streamingModuleMgr;
