/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <VFSDevice.h>

#ifdef COMPILING_VFS_CORE
#define VFS_CORE_EXPORT DLL_EXPORT
#else
#define VFS_CORE_EXPORT DLL_IMPORT
#endif

namespace vfs
{
	class VFS_CORE_EXPORT RagePackfile : public Device
	{
	private:
		struct Entry
		{
			uint32_t nameOffset;
			uint32_t length;

			uint32_t dataOffset : 31;
			uint32_t isDirectory : 1;

			uint32_t flags;
		};

		struct Header2
		{
			uint32_t magic;
			uint32_t tocSize;
			uint32_t numEntries;
			uint32_t unkFlag;
			uint32_t cryptoFlag;
		};

		struct HandleData
		{
			bool valid;
			Entry entry;
			size_t curOffset;

			inline HandleData()
				: valid(false)
			{

			}
		};

	private:
		fwRefContainer<Device> m_parentDevice;

		THandle m_parentHandle;

		uint64_t m_parentPtr;

		std::string m_pathPrefix;

		Header2 m_header;

		HandleData m_handles[32];

		std::vector<Entry> m_entries;

		std::vector<char> m_nameTable;

	private:
		HandleData* AllocateHandle(THandle* outHandle);

		HandleData* GetHandle(THandle inHandle);

		const Entry* FindEntry(const std::string& path);

		void FillFindData(FindData* data, const Entry* entry);

	public:
		RagePackfile();

		virtual ~RagePackfile() override;

		virtual THandle Open(const std::string& fileName, bool readOnly) override;

		virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr) override;

		virtual size_t Read(THandle handle, void* outBuffer, size_t size) override;

		virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size) override;

		virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override;

		virtual bool Close(THandle handle) override;

		virtual bool CloseBulk(THandle handle) override;

		virtual size_t GetLength(THandle handle) override;

		virtual size_t GetLength(const std::string& fileName) override;

		virtual THandle FindFirst(const std::string& folder, FindData* findData) override;

		virtual bool FindNext(THandle handle, FindData* findData) override;

		virtual void FindClose(THandle handle) override;

		virtual void SetPathPrefix(const std::string& pathPrefix) override;

	public:
		bool OpenArchive(const std::string& archivePath);
	};
}
