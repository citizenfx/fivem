#pragma once

#include <VFSDevice.h>

namespace vfs
{
class LocalDevice : public Device
{
public:
	virtual THandle Open(const std::string& fileName, bool readOnly, bool append = false) override;

	virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr) override;

	virtual THandle Create(const std::string& filename, bool createIfExists = true, bool append = false) override;

	virtual size_t Read(THandle handle, void* outBuffer, size_t size) override;

	virtual size_t ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size) override;

	virtual size_t Write(THandle handle, const void* buffer, size_t size) override;

	virtual size_t WriteBulk(THandle handle, uint64_t ptr, const void* buffer, size_t size) override;

	virtual size_t Seek(THandle handle, intptr_t offset, int seekType) override;

	virtual bool Close(THandle handle) override;

	virtual bool CloseBulk(THandle handle) override;

	virtual bool RemoveFile(const std::string& filename) override;

	virtual bool RenameFile(const std::string& from, const std::string& to) override;

	virtual bool CreateDirectory(const std::string& name) override;

	virtual bool RemoveDirectory(const std::string& name) override;

	virtual std::time_t GetModifiedTime(const std::string& fileName) override;

	virtual size_t GetLength(THandle handle) override;

	virtual THandle FindFirst(const std::string& folder, FindData* findData) override;

	virtual bool FindNext(THandle handle, FindData* findData) override;

	virtual void FindClose(THandle handle) override;

	virtual uint32_t GetAttributes(const std::string& filename) override;

	virtual bool ExtensionCtl(int controlIdx, void* controlData, size_t controlSize) override;

	virtual std::string GetAbsolutePath() const override
	{
		return {};
	}

	bool Flush(THandle handle) override;
};
}
