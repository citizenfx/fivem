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

	public:
		virtual void m_0() = 0;

		virtual void m_8() = 0;

		virtual void m_10() = 0;

		virtual void m_18() = 0;

		virtual void m_20() = 0;

		virtual void m_28() = 0;

		virtual void m_30() = 0;

		virtual void m_38() = 0;

		virtual void m_40() = 0;

		virtual void m_48() = 0;

		virtual void m_50() = 0;

		virtual void m_58() = 0;

		virtual void m_60() = 0;

		virtual void m_68() = 0;

		virtual void m_70() = 0;

		virtual void HasEntries() = 0;

		virtual uint32_t GetEntryCount() = 0;

		virtual void GetEntryPhysicalSortKey() = 0;

		virtual uint32_t GetEntryNameHash(uint32_t id) = 0;

		virtual int GetEntryFileExtId(uint32_t id) = 0;

		virtual void GetEntryName() = 0;

		virtual void GetEntryFullName() = 0;

		virtual uint32_t GetEntryByName(uint32_t nameHash, bool unk) = 0; // GetEntryIndex

		virtual uint32_t GetEntryByName(const char* name) = 0;
	};
}
