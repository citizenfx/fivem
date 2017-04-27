#pragma once

#ifdef COMPILING_GTA_STREAMING_FIVE
#define STREAMING_EXPORT DLL_EXPORT
#else
#define STREAMING_EXPORT DLL_IMPORT
#endif

#include <fiCollectionWrapper.h>

struct StreamingPackfileEntry
{
	FILETIME modificationTime;
	uint8_t pad[32];
	uint64_t packfileParentHandle;
	uint64_t pad1;
	rage::fiPackfile* packfile;
	uint8_t pad2[2];
	uint8_t loadedFlag;
	uint8_t pad3;
	uint8_t enabled;
	uint8_t pad4[19];
	uint32_t parentIdentifier;
	uint32_t pad5;
	uint16_t isHdd;
	uint16_t pad6;
	uint32_t pad7;
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