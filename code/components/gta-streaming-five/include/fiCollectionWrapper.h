/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <fiDevice.h>

namespace rage
{
	template<typename T, uint32_t chunkSize, uint32_t chunksCountUnused>
	struct chunkyArray
	{
		chunkyArray(): count(0)
		{
		}

		T& operator[](uint32_t index)
		{
			return memory[index / chunkSize][index % chunkSize];
		}

		uint32_t GetCount()
		{
			return count;
		}

		T* memory[chunksCountUnused];
		uint32_t count;
	};

	class fiCollection : public fiDevice
	{
	public:
		// basically, packfile entry; no idea what uses this like that though
		struct FileEntry
		{
			uint64_t nameOffset : 16;
			uint64_t size : 24;
			uint64_t offset : 24;

			uint32_t virtFlags;
			uint32_t physFlags;
		};

		struct RawEntry
		{
			FileEntry fe;
#if defined(IS_RDR3)
			char m_pad[8];
#endif
			uint64_t timestamp;

			const char* fileName;
		};

		char m_pad[1448];
		chunkyArray<RawEntry, 1024, 64> m_entries;

	public:
		// UnInit
		virtual void CloseCollection() = 0;

		// OpenBulkFromHandle
		virtual int64_t OpenCollectionEntry(uint16_t index, uint64_t* ptr) = 0;

		virtual RawEntry* GetEntry(uint16_t index) = 0;

		// GetEntryPhysicalSortKey
		virtual int64_t Unk1(uint16_t index, bool flag) = 0;

		virtual const char* GetEntryName(uint16_t index) = 0;

		// GetEntryFullName?
		virtual void GetEntryNameToBuffer(uint16_t index, char* buffer, int maxLen) = 0;

		// GetEntryIndex
		virtual uint16_t GetEntryByName(const char* name) = 0;

		// GetBasePhysicalSortKey
		virtual int32_t GetUnk1() = 0;

		// Prefetch
		virtual bool UnkA(uint16_t) = 0;

		// ?? ought to be IsPackfile
		virtual bool CloseBasePackfile() = 0;

		// IsStreaming
		virtual uint8_t UnkC() = 0;

		// SetStreaming
		virtual bool UnkD(uint8_t) = 0;

		virtual void* UnkE(const char*) = 0;

		virtual uint64_t UnkF(void*, bool) = 0;
	};
}
