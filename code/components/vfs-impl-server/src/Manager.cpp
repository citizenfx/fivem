#include <StdInc.h>
#include <Manager.h>

#include <LocalDevice.h>

#include <filesystem>

namespace vfs
{
ManagerServer::ManagerServer()
{
	m_fallbackDevice = new LocalDevice();
}

fwRefContainer<Device> MakeMemoryDevice();

fwRefContainer<Device> ManagerServer::FindDevice(const std::string& absolutePath, std::string& transformedPath)
{
	std::filesystem::path absolute(absolutePath);
	std::string absoluteGenericString = std::filesystem::absolute(absolute).generic_string();
	std::lock_guard<std::recursive_mutex> lock(m_mountMutex);

	size_t longestPrefixLength = 0;
	fwRefContainer<Device> foundDevice {nullptr};
	for (const auto& mount : m_mounts)
	{
		for (const auto& device : mount.devices)
		{
			// check if the device has an absolute path
			if (device->GetAbsolutePath().empty())
			{
				continue;
			}

			std::string deviceAbsolutePath = std::filesystem::absolute(std::filesystem::path(device->GetAbsolutePath())).generic_string();
			size_t prefixLength = deviceAbsolutePath.size();

			// find the device with the largest prefix length
			if (longestPrefixLength > prefixLength)
			{
				continue;
			}

			// device matches if the path start is the same
			// e.g. usr/local/bin starts with /usr/local/
			if (absoluteGenericString.rfind(deviceAbsolutePath, 0) == 0)
			{
				longestPrefixLength = deviceAbsolutePath.size();
				foundDevice = device;
				// e.g. /usr/local/bin -> bin, @local/bin
				transformedPath = mount.prefix + absoluteGenericString.substr(deviceAbsolutePath.size());
			}
		}
	}

	return foundDevice;
}

fwRefContainer<Device> ManagerServer::GetDevice(const std::string& path)
{
	// return hardcoded devices
	{
		if (_strnicmp(path.c_str(), "memory:", 7) == 0)
		{
			static fwRefContainer<Device> memoryDevice = MakeMemoryDevice();

			return memoryDevice;
		}
	}

	std::lock_guard<std::recursive_mutex> lock(m_mountMutex);

	// if only one device exists for a chosen prefix, we want to always return that device
	// if multiple exists, we only want to return a device if there's a file/directory entry by the path we specify

	for (const auto& mount : m_mounts)
	{
		// if the prefix patches
		if (strncmp(path.c_str(), mount.prefix.c_str(), mount.prefix.length()) == 0)
		{
			// single device case
			if (mount.devices.size() == 1)
			{
				return mount.devices[0];
			}
			else
			{
				// check each device assigned to the mount point
				Device::THandle handle;

				for (const auto& device : mount.devices)
				{
					if ((handle = device->Open(path, true)) != Device::InvalidHandle)
					{
						device->Close(handle);

						return device;
					}
				}

				// as this is a valid mount, but no valid file, bail out with nothing
				return nullptr;
			}
		}
	}

	return m_fallbackDevice;
}

fwRefContainer<Device> ManagerServer::GetNativeDevice(void* nativeDevice)
{
	assert(false);

	return nullptr;
}

void ManagerServer::Mount(fwRefContainer<Device> device, const std::string& path)
{
	// set the path prefix on the device
	device->SetPathPrefix(path);

	// mount
	std::lock_guard<std::recursive_mutex> lock(m_mountMutex);

	// find an existing mount in the mount list to add to
	for (auto&& mount : m_mounts)
	{
		if (mount.prefix == path)
		{
			mount.devices.push_back(device);
			return;
		}
	}

	// if we're here, we didn't find any existing device - add a new one instead
	{
		MountPoint mount;
		mount.prefix = path;
		mount.devices.push_back(device);

		m_mounts.insert(mount);
	}
}

void ManagerServer::Unmount(const std::string& path)
{
	std::lock_guard<std::recursive_mutex> lock(m_mountMutex);

	// we need to manually iterate to be able to use erase(iterator)
	for (auto it = m_mounts.begin(); it != m_mounts.end(); )
	{
		const auto& mount = *it;

		if (mount.prefix == path)
		{
			it = m_mounts.erase(it);
		}
		else
		{
			++it;
		}
	}
}
}

#ifdef IS_FXSERVER
static InitFunction initfunction([]()
{
	Instance<vfs::Manager>::Set(new vfs::ManagerServer());
});
#endif
