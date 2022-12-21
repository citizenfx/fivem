#pragma once

#include <atArray.h>

#ifdef COMPILING_GTA_STREAMING_FIVE
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

#include <fiCollectionWrapper.h>
#include "XBRVirtual.h"

struct StreamingPackfileEntry
{
	FILETIME modificationTime;       // +0
	uint8_t pad0[8];                 // +8
	uint32_t nameHash;               // +16
	uint8_t pad[20];                 // +20
	uint64_t packfileParentHandle;   // +40
	uint64_t pad1;                   // +48
#ifdef GTA_FIVE
	rage::fiPackfile* packfile;      // +56
#elif IS_RDR3
	void* packfile;					 // +56
#endif
	uint8_t pad2[2];                 // +64
	uint8_t loadedFlag;              // +66
	uint8_t pad3;                    // +67
	uint8_t enabled;                 // +68
	uint8_t pad4;                    // +69
	uint8_t isDLC;                   // +70
	uint8_t isUnk;                   // +71
	uint8_t cacheFlags;              // +72
	uint8_t pad5[15];                // +73
	uint32_t parentIdentifier;       // +88
	uint32_t pad6;                   // +92
	uint8_t isHdd;                   // +96
	uint8_t isHdd_2;				 // +97
	uint16_t pad7;                   // +98
	uint32_t pad8;                   // +100
#ifdef IS_RDR3
	char pad9[40];
#endif
};

struct StreamingDataEntry
{
	uint32_t handle;
	uint32_t flags;

	size_t STREAMING_EXPORT ComputePhysicalSize(uint32_t strIndex);

	size_t STREAMING_EXPORT ComputeVirtualSize(uint32_t strIndex, void* a3, bool a4);
};

namespace streaming
{
	struct StreamingListEntry
	{
		StreamingListEntry* Prev;
		StreamingListEntry* Next;
		uint32_t Index;
	};

	// in reality a blockmap, but at least fwAssetRscStore only uses the first block reference pointer
	struct strAssetReference
	{
		void* unknown;
		void* asset;
	};

	class strStreamingModule
#ifdef GTA_FIVE
		: XBR_VIRTUAL_BASE_2802(0)
#endif
	{
public:
		XBR_VIRTUAL_DTOR(strStreamingModule)

#ifdef IS_RDR3
		XBR_VIRTUAL_METHOD(void, m_8, ())

		XBR_VIRTUAL_METHOD(void, m_10, ())

		//
		// Creates a new asset for `name`, or returns the existing index in this module for it.
		//
		XBR_VIRTUAL_METHOD(uint32_t*, FindSlotFromHashKey, (uint32_t* id, uint32_t name))
#elif GTA_FIVE
		//
		// Creates a new asset for `name`, or returns the existing index in this module for it.
		//
		XBR_VIRTUAL_METHOD(uint32_t*, FindSlotFromHashKey, (uint32_t* id, const char* name))
#endif

		//
		// Returns the index in this streaming module for the asset specified by `name`.
		//
		XBR_VIRTUAL_METHOD(uint32_t*, FindSlot, (uint32_t* id, const char* name))

		//
		// Unloads the specified asset from the streaming module.
		// This won't update the asset state in CStreaming, use these functions instead.
		//
		XBR_VIRTUAL_METHOD(void, Remove, (uint32_t id))

		//
		// Removes the specified asset from the streaming module.
		//
		XBR_VIRTUAL_METHOD(void, RemoveSlot, (uint32_t object))

		//
		// Loads an asset from an in-memory RSC file.
		//
		XBR_VIRTUAL_METHOD(bool, Load, (uint32_t object, const void* buffer, uint32_t length))

		//
		// Loads an asset from a block map.
		//
		XBR_VIRTUAL_METHOD(void, PlaceResource, (uint32_t object, void* blockMap, const char* name))

		//
		// Sets the asset pointer directly.
		//
		XBR_VIRTUAL_METHOD(void, SetResource, (uint32_t object, strAssetReference& reference))

		//
		// Gets the asset pointer for a loaded asset.
		// Returns NULL if not loaded.
		//
		XBR_VIRTUAL_METHOD(void*, GetPtr, (uint32_t object))

		XBR_VIRTUAL_METHOD(void*, GetDataPtr, (uint32_t object))

		XBR_VIRTUAL_METHOD(void*, Defragment, (uint32_t object, void* blockMap, const char* name))

		// nullsub
		XBR_VIRTUAL_METHOD(void, m_58, ())

		// nullsub
		XBR_VIRTUAL_METHOD(void, m_60, ())

		// only overridden in specific modules
		XBR_VIRTUAL_METHOD(void*, GetResource, (uint32_t object))

		// nullsub
		XBR_VIRTUAL_METHOD(void, GetModelMapTypeIndex, (uint32_t localIndex, uint32_t& outIndex))

		// unknown function, involving releasing
		XBR_VIRTUAL_METHOD(void, m_78, (uint32_t object, int))

		XBR_VIRTUAL_METHOD(void, AddRef, (uint32_t id))

		XBR_VIRTUAL_METHOD(void, RemoveRef, (uint32_t id))

		XBR_VIRTUAL_METHOD(void, ResetAllRefs, ()) // resetrefcount

		XBR_VIRTUAL_METHOD(int, GetNumRefs, (uint32_t id))

		//
		// Formats the reference count as a string.
		//
		XBR_VIRTUAL_METHOD(const char*, GetRefCountString, (uint32_t id, char* buffer, size_t length))

		XBR_VIRTUAL_METHOD(int, GetDependencies, (uint32_t object, uint32_t* outDependencies, size_t count))

		// nullsub?
		XBR_VIRTUAL_METHOD(void, m_B0, ())
		XBR_VIRTUAL_METHOD(void, m_B8, ())
		XBR_VIRTUAL_METHOD(void, m_C0, ())

		// ...

		uint32_t baseIdx;
	};

	class STREAMING_EXPORT strStreamingModuleMgr
	{
	public:
		strStreamingModule* GetStreamingModule(int index);

		strStreamingModule* GetStreamingModule(const char* extension);

	private:
		void* vtbl;
		char pad[16];

	public:
		atArray<strStreamingModule*> modules;
	};

	// actually CStreaming
	class STREAMING_EXPORT Manager
	{
	private:
		inline Manager()
		{
#ifdef GTA_FIVE
			static_assert(offsetof(Manager, NumPendingRequests) == 0x1E0);
#endif
		}

	public:
		void RequestObject(uint32_t objectId, int flags);

		bool ReleaseObject(uint32_t objectId);

		bool ReleaseObject(uint32_t objectId, int flags);

		bool IsObjectReadyToDelete(uint32_t streamingIndex, int flags);

		void FindAllDependents(atArray<uint32_t>& outIndices, uint32_t objectId);

		template<typename TPred>
		void FindAllDependentsCustomPred(atArray<uint32_t>& outIndices, uint32_t objectId, TPred&& pred)
		{
			for (size_t i = 0; i < numEntries; i++)
			{
				if (pred(Entries[i]))
				{
					FindDependentsInner(i, outIndices, objectId);
				}
			}
		}

		static Manager* GetInstance();

	private:
		void FindDependentsInner(uint32_t selfId, atArray<uint32_t>& outIndices, uint32_t objectId);

	public:
		StreamingDataEntry* Entries;
		char pad3[16];
		int numEntries;
		int f;
		char pad[88 - 16 - 8];
		StreamingListEntry* RequestListHead;
		StreamingListEntry* RequestListTail;

#ifndef IS_RDR3
		char pad2[368 - 40];
#endif

		strStreamingModuleMgr moduleMgr;

		int NumPendingRequests;
		int NumPendingRequests3;
		int NumPendingRequestsPrio;
	};

	void STREAMING_EXPORT LoadObjectsNow(bool a1);

	uint32_t STREAMING_EXPORT GetStreamingIndexForName(const std::string& name);

	STREAMING_EXPORT std::string GetStreamingNameForIndex(uint32_t index);

	STREAMING_EXPORT std::string GetStreamingBaseNameForHash(uint32_t hash);

	uint32_t STREAMING_EXPORT GetStreamingIndexForLocalHashKey(streaming::strStreamingModule* module, uint32_t hash);

	STREAMING_EXPORT StreamingPackfileEntry* GetStreamingPackfileByIndex(int index);

	STREAMING_EXPORT uint32_t* RegisterRawStreamingFile(uint32_t* fileId, const char* fileName, bool unkTrue, const char* registerAs, bool errorIfFailed);

	STREAMING_EXPORT StreamingPackfileEntry* GetStreamingPackfileForEntry(StreamingDataEntry* entry);

	// are we trying to shut down?
	STREAMING_EXPORT bool IsStreamerShuttingDown();

	atArray<StreamingPackfileEntry>& GetStreamingPackfileArray();
}

namespace rage
{
	class strStreamingAllocator : public sysMemAllocator
	{
	public:
		static STREAMING_EXPORT strStreamingAllocator* GetInstance();
	};
}

#if 0
namespace rage
{
	class strStreamingModule
	{

	};

	class STREAMING_EXPORT strStreamingModuleMgr
	{
	private:
		inline strStreamingModuleMgr() {}

	public:
		strStreamingModule* GetModuleFromExtension(const char* extension);

		static strStreamingModuleMgr* GetInstance();
	};
}
#endif
