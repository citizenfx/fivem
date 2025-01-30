#pragma once

#include <mutex>
#include <VFSManager.h>

namespace vfs
{
class ManagerServer : public vfs::Manager
{
private:
	struct MountPoint
	{
		std::string prefix;

		mutable std::vector<fwRefContainer<Device>> devices; // mutable? MUTABLE? is this Rust?!
	};

	// sorts mount points based on longest prefix
	struct MountPointComparator
	{
		inline bool operator()(const MountPoint& left, const MountPoint& right) const
		{
			size_t lengthLeft = left.prefix.length();
			size_t lengthRight = right.prefix.length();

			if (lengthLeft == lengthRight)
			{
				return (left.prefix < right.prefix);
			}

			return (lengthLeft > lengthRight);
		}
	};

private:
	std::set<MountPoint, MountPointComparator> m_mounts;

	std::recursive_mutex m_mountMutex;

	// fallback device - usually a local file system implementation
	fwRefContainer<vfs::Device> m_fallbackDevice;

public:
	ManagerServer();

public:
	virtual fwRefContainer<Device> GetDevice(const std::string& path) override;

	virtual fwRefContainer<Device> FindDevice(const std::string& absolutePath, std::string& transformedPath) override;

	virtual fwRefContainer<Device> GetNativeDevice(void* nativeDevice) override;

	virtual void Mount(fwRefContainer<Device> device, const std::string& path) override;

	virtual void Unmount(const std::string& path) override;
};

std::string MakeMemoryFilename(const void* buffer, size_t size);
}
