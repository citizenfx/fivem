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
		virtual void CloseCollection() = 0;

		virtual int64_t OpenCollectionEntry(uint16_t index, uint64_t* ptr) = 0;

		virtual const FileEntry* GetEntry(uint16_t index) = 0;

		virtual int64_t Unk1(uint16_t index, bool flag) = 0;

		virtual const char* GetEntryName(uint16_t index) = 0;

		virtual void GetEntryNameToBuffer(uint16_t index, char* buffer, int maxLen) = 0;

		virtual uint16_t GetEntryByName(const char* name) = 0;

		virtual int32_t GetUnk1() = 0;

		virtual bool UnkA(uint16_t) = 0;

		virtual bool CloseBasePackfile() = 0;

		virtual uint8_t UnkC() = 0;

		virtual bool UnkD(uint8_t) = 0;

		virtual void* UnkE(const char*) = 0;

		virtual uint64_t UnkF(void*, bool) = 0;
	};
}