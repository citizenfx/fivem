#pragma once

#include <VFSDevice.h>
#include <zlib.h>

#ifdef COMPILING_VFS_CORE
#define VFS_CORE_EXPORT DLL_EXPORT
#else
#define VFS_CORE_EXPORT DLL_IMPORT
#endif

namespace vfs
{
	class VFS_CORE_EXPORT RagePackfile7 : public Device
	{
	private:
		struct Entry
		{
			uint64_t nameOffset : 16;
			uint64_t size : 24;
			uint64_t offset : 24;

			uint32_t virtFlags;
			uint32_t physFlags;
		};

		struct Header7
		{
			uint32_t magic;
			uint32_t entryCount;
			uint32_t nameLength;
			uint32_t encryption;
		};

		struct HandleData
		{
			bool valid;
			bool compressed;
			Entry entry;
			size_t curOffset;
			size_t curDecOffset;

			uint8_t zbuf[0x2000];
			z_stream strm;

			inline HandleData()
				: valid(false)
			{

			}
		};

		void InternalRead(HandleData* handleData);

	private:
		fwRefContainer<Device> m_parentDevice;

		THandle m_parentHandle;

		uint64_t m_parentPtr;

		std::string m_pathPrefix;

		Header7 m_header;

		HandleData m_handles[32];

		std::vector<Entry> m_entries;

		std::vector<char> m_nameTable;

	private:
		HandleData* AllocateHandle(THandle* outHandle);

		HandleData* GetHandle(THandle inHandle);

		const Entry* FindEntry(const std::string& path);

		void FillFindData(FindData* data, const Entry* entry);

	public:
		RagePackfile7();

		virtual ~RagePackfile7() override;

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

		virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override;

	public:
		bool OpenArchive(const std::string& archivePath, bool needsEncryption = false);
	};
}
