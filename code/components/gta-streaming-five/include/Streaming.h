#pragma once

#include <atArray.h>

#ifdef COMPILING_GTA_STREAMING_FIVE
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

#include <fiCollectionWrapper.h>

struct StreamingPackfileEntry
{
	FILETIME modificationTime;       // +0
	uint8_t pad0[8];                 // +8
	uint32_t nameHash;               // +16
	uint8_t pad[20];                 // +20
	uint64_t packfileParentHandle;   // +40
	uint64_t pad1;                   // +48
	rage::fiPackfile* packfile;      // +56
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
	uint16_t isHdd;                  // +96
	uint16_t pad7;                   // +98
	uint32_t pad8;                   // +100
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
	{
	public:
		virtual ~strStreamingModule() = 0;

		//
		// Creates a new asset for `name`, or returns the existing index in this module for it.
		//
		virtual uint32_t* FindSlotFromHashKey(uint32_t* id, const char* name) = 0;

		//
		// Returns the index in this streaming module for the asset specified by `name`.
		//
		virtual uint32_t* FindSlot(uint32_t* id, const char* name) = 0;

		//
		// Unloads the specified asset from the streaming module.
		// This won't update the asset state in CStreaming, use these functions instead.
		//
		virtual void Remove(uint32_t id) = 0;

		//
		// Removes the specified asset from the streaming module.
		//
		virtual void RemoveSlot(uint32_t object) = 0;

		//
		// Loads an asset from an in-memory RSC file.
		//
		virtual bool Load(uint32_t object, const void* buffer, uint32_t length) = 0;

		//
		// Loads an asset from a block map.
		//
		virtual void PlaceResource(uint32_t object, void* blockMap, const char* name) = 0;

		//
		// Sets the asset pointer directly.
		//
		virtual void SetResource(uint32_t object, strAssetReference& reference) = 0;

		//
		// Gets the asset pointer for a loaded asset.
		// Returns NULL if not loaded.
		//
		virtual void* GetPtr(uint32_t object) = 0;

		virtual void* GetDataPtr(uint32_t object) = 0;

		virtual void* Defragment(uint32_t object, void* blockMap, const char* name) = 0;

		// nullsub
		virtual void m_58() = 0;

		// nullsub
		virtual void m_60() = 0;

		// only overridden in specific modules
		virtual void* GetResource(uint32_t object) = 0;

		// nullsub
		virtual void m_70() = 0;

		// unknown function, involving releasing
		virtual void m_78(uint32_t object, int) = 0;

		virtual void AddRef(uint32_t id) = 0;

		virtual void RemoveRef(uint32_t id) = 0;

		virtual void ResetAllRefs() = 0; // resetrefcount

		virtual int GetNumRefs(uint32_t id) = 0;

		//
		// Formats the reference count as a string.
		//
		virtual const char* GetRefCountString(uint32_t id, char* buffer, size_t length) = 0;

		virtual int GetDependencies(uint32_t object, uint32_t* outDependencies, size_t count) = 0;

		// nullsub?
		virtual void m_B0() = 0;
		virtual void m_B8() = 0;
		virtual void m_C0() = 0;

		// ...

		uint32_t baseIdx;
	};

	class STREAMING_EXPORT strStreamingModuleMgr
	{
	public:
		virtual ~strStreamingModuleMgr() = default;

		strStreamingModule* GetStreamingModule(int index);

		strStreamingModule* GetStreamingModule(const char* extension);
	};

	// actually CStreaming
	class STREAMING_EXPORT Manager
	{
	private:
		inline Manager() {}

	public:
		void RequestObject(uint32_t objectId, int flags);

		bool ReleaseObject(uint32_t objectId);

		bool ReleaseObject(uint32_t objectId, int flags);

		bool IsObjectReadyToDelete(uint32_t streamingIndex, int flags);

		void FindAllDependents(atArray<uint32_t>& outIndices, uint32_t objectId);

		static Manager* GetInstance();

	public:
		StreamingDataEntry* Entries;
		char pad3[16];
		int numEntries;
		int f;
		char pad[88 - 16 - 8];
		StreamingListEntry* RequestListHead;
		StreamingListEntry* RequestListTail;

		char pad2[368 - 40];

		strStreamingModuleMgr moduleMgr;

		char pad4[32];

		int NumPendingRequests;
		int NumPendingRequests3;
		int NumPendingRequestsPrio;
	};

	void STREAMING_EXPORT LoadObjectsNow(bool a1);

	uint32_t STREAMING_EXPORT GetStreamingIndexForName(const std::string& name);

	STREAMING_EXPORT const std::string& GetStreamingNameForIndex(uint32_t index);

	STREAMING_EXPORT StreamingPackfileEntry* GetStreamingPackfileByIndex(int index);

	STREAMING_EXPORT uint32_t RegisterRawStreamingFile(uint32_t* fileId, const char* fileName, bool unkTrue, const char* registerAs, bool errorIfFailed);

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
