#pragma once

#include <atArray.h>

#ifdef COMPILING_GTA_CORE_FIVE
#define GTA_CORE_EXPORT DLL_EXPORT
#else
#define GTA_CORE_EXPORT DLL_IMPORT
#endif

namespace rage
{
	enum InitFunctionType : int
	{
		INIT_UNKNOWN = 0,
		INIT_CORE = 1,
		INIT_BEFORE_MAP_LOADED = 2,
		INIT_AFTER_MAP_LOADED = 4,
		INIT_SESSION = 8
	};

	GTA_CORE_EXPORT const char* InitFunctionTypeToString(InitFunctionType type);

	struct InitFunctionData
	{
		void(*initFunction)(int);
		void(*shutdownFunction)(int);

		int initOrder;
		int shutdownOrder;
		InitFunctionType initMask;
		InitFunctionType shutdownMask;

		uint32_t funcHash;

		GTA_CORE_EXPORT const char* GetName() const;

		GTA_CORE_EXPORT bool TryInvoke(InitFunctionType type);
	};

	// arguments: type
	extern GTA_CORE_EXPORT fwEvent<InitFunctionType> OnInitFunctionStart;

	// arguments: type, order index, count
	extern GTA_CORE_EXPORT fwEvent<InitFunctionType, int, int> OnInitFunctionStartOrder;

	// arguments: type, func index, data (non-const)
	// can be canceled to prevent invocation
	extern GTA_CORE_EXPORT fwEvent<InitFunctionType, int, InitFunctionData&> OnInitFunctionInvoking;

	// arguments: type, data
	extern GTA_CORE_EXPORT fwEvent<InitFunctionType, const InitFunctionData&> OnInitFunctionInvoked;

	// arguments: type, order index
	extern GTA_CORE_EXPORT fwEvent<InitFunctionType, int> OnInitFunctionEndOrder;

	// arguments: type
	extern GTA_CORE_EXPORT fwEvent<InitFunctionType> OnInitFunctionEnd;

	struct InitFunctionEntry
	{
		int order;
		atArray<int> functions;
		InitFunctionEntry* next;
	};

	struct InitFunctionList
	{
		InitFunctionType type;
		InitFunctionEntry* entries;
		InitFunctionList* next;
	};

	class gameSkeleton
	{
	public:
		void GTA_CORE_EXPORT RunInitFunctions(InitFunctionType type);

	protected:
		int m_functionOrder; // 8
		InitFunctionType m_functionType; // 12

		intptr_t pad; // 16

		atArray<InitFunctionData> m_initFunctions; // 24
		void* pad2; // 40

		char pad3[256]; // 48

		InitFunctionList* m_initFunctionList;

	private:
		virtual ~gameSkeleton() = 0;
	};
}


