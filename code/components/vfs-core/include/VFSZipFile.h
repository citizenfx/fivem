#pragma once

#include <VFSDevice.h>

#ifdef COMPILING_VFS_CORE
#define VFS_CORE_EXPORT DLL_EXPORT
#else
#define VFS_CORE_EXPORT DLL_IMPORT
#endif

namespace vfs
{
	class VFS_CORE_EXPORT ZipFile : public Device
	{
	private:
		struct Entry
		{
			int64_t entryOffset;
			size_t fileSize;
		};

		struct IgnoreCaseLess
		{
			inline bool operator()(const std::string& left, const std::string& right) const
			{
				return _stricmp(left.c_str(), right.c_str()) < 0;
			}
		};

		struct HandleData
		{
			bool valid;
			void* mzZip;
			void* mzStream;
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

		HandleData m_handles[32];

		std::unordered_map<std::string, Entry> m_entries;

	private:
		HandleData* AllocateHandle(THandle* outHandle);

		HandleData* GetHandle(THandle inHandle);

		const Entry* FindEntry(const std::string& path);

		void FillFindData(FindData* data, const Entry* entry);

	public:
		ZipFile();

		virtual ~ZipFile() override;

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
