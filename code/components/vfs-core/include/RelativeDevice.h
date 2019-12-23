#pragma once

#include <VFSDevice.h>

namespace vfs
{
class
#if defined(COMPILING_VFS_CORE)
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	RelativeDevice : public Device
{
  public:
	// finds the other device in the manager
	RelativeDevice(const std::string& otherPrefix);

	RelativeDevice(const fwRefContainer<Device>& otherDevice, const std::string& otherPrefix);

	virtual THandle Open(const std::string& fileName, bool readOnly) override;

	virtual THandle OpenBulk(const std::string& fileName, uint64_t* ptr) override;

	virtual THandle Create(const std::string& filename) override;

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

	virtual size_t GetLength(const std::string& fileName) override;

	virtual THandle FindFirst(const std::string& folder, FindData* findData) override;

	virtual bool FindNext(THandle handle, FindData* findData) override;

	virtual void FindClose(THandle handle) override;

	// Sets the path prefix for the device, which implementations should strip for generating a local path portion.
	virtual void SetPathPrefix(const std::string& pathPrefix) override;

  private:
	fwRefContainer<Device> m_otherDevice;

	std::string m_otherPrefix;
	std::string m_pathPrefix;

  private:
	inline std::string TranslatePath(const std::string& inPath)
	{
		return m_otherPrefix + inPath.substr(m_pathPrefix.length());
	}
};
}
