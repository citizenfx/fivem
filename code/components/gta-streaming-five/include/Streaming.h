#pragma once

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
	uint8_t pad4[3];                 // +69
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
};

namespace streaming
{
	struct StreamingListEntry
	{
		StreamingListEntry* Prev;
		StreamingListEntry* Next;
		uint32_t Index;
	};

	// actually CStreaming
	class STREAMING_EXPORT Manager
	{
	private:
		inline Manager() {}

	public:
		void RequestObject(uint32_t objectId, int flags);

		void ReleaseObject(uint32_t objectId);

		static Manager* GetInstance();

	public:
		StreamingDataEntry* Entries;
		char pad[88];
		StreamingListEntry* RequestListHead;
		StreamingListEntry* RequestListTail;

		char pad2[368];

		int NumPendingRequests;
		int NumPendingRequests3;
		int NumPendingRequestsPrio;
	};

	void STREAMING_EXPORT LoadObjectsNow(bool a1);

	uint32_t STREAMING_EXPORT GetStreamingIndexForName(const std::string& name);

	STREAMING_EXPORT const std::string& GetStreamingNameForIndex(uint32_t index);

	STREAMING_EXPORT StreamingPackfileEntry* GetStreamingPackfileForEntry(StreamingDataEntry* entry);

	STREAMING_EXPORT uint32_t RegisterRawStreamingFile(uint32_t* fileId, const char* fileName, bool unkTrue, const char* registerAs, bool errorIfFailed);
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
