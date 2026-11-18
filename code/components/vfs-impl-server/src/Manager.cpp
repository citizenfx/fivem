#include <StdInc.h>
#include <Manager.h>

#include <LocalDevice.h>

#include <filesystem>
#include <utility>

namespace
{
struct DeviceMatch
{
	size_t matchedPrefixLength = 0;
	size_t canonicalPrefixLength = 0;
	fwRefContainer<vfs::Device> device {nullptr};
	std::string transformedPath;
	bool isPreferredPrefix = false;
};

enum class DevicePathKind
{
	Lexical,
	Canonical
};

std::string GetLexicalAbsoluteGenericPath(const std::filesystem::path& path)
{
	std::error_code ec;
	std::filesystem::path absolutePath = std::filesystem::absolute(path, ec);
	if (ec)
	{
		absolutePath = path;
	}

	return absolutePath.lexically_normal().generic_string();
}

std::string GetCanonicalAbsoluteGenericPath(const std::filesystem::path& path)
{
	std::error_code ec;
	std::filesystem::path normalizedPath = std::filesystem::weakly_canonical(path, ec);
	if (ec)
	{
		return GetLexicalAbsoluteGenericPath(path);
	}

	return normalizedPath.lexically_normal().generic_string();
}

void AddDirectorySuffix(std::string& path, const std::filesystem::path& sourcePath)
{
	if (path.empty() || path.back() == '/')
	{
		return;
	}

	std::string sourcePathString = sourcePath.generic_string();
	if (!sourcePathString.empty() && sourcePathString.back() == '/')
	{
		path += '/';
		return;
	}

	std::error_code ec;
	if (std::filesystem::is_directory(sourcePath, ec) && !ec)
	{
		path += '/';
	}
}

std::string GetMountedDevicePath(const std::string& devicePath, bool canonical)
{
	if (devicePath.empty())
	{
		return {};
	}

	std::filesystem::path path(devicePath);
	std::string absolutePath = canonical
		? GetCanonicalAbsoluteGenericPath(path)
		: GetLexicalAbsoluteGenericPath(path);
	AddDirectorySuffix(absolutePath, path);
	return absolutePath;
}

void UpdateDeviceMatch(DeviceMatch& match, size_t matchedPrefixLength, size_t canonicalPrefixLength,
	const fwRefContainer<vfs::Device>& device, std::string transformedPath, const bool isPreferredPrefix)
{
	if (match.device.GetRef())
	{
		if (match.matchedPrefixLength > matchedPrefixLength)
		{
			return;
		}

		if (match.matchedPrefixLength == matchedPrefixLength && match.isPreferredPrefix && !isPreferredPrefix)
		{
			return;
		}
	}

	match.matchedPrefixLength = matchedPrefixLength;
	match.canonicalPrefixLength = canonicalPrefixLength;
	match.device = device;
	match.transformedPath = std::move(transformedPath);
	match.isPreferredPrefix = isPreferredPrefix;
}

const DeviceMatch& SelectDeviceMatch(const DeviceMatch& lexicalMatch, const DeviceMatch& canonicalMatch, const bool pathResolvedThroughLink)
{
	if (!lexicalMatch.device.GetRef())
	{
		return canonicalMatch;
	}

	if (!canonicalMatch.device.GetRef())
	{
		return lexicalMatch;
	}

	// Keep VFS longest-prefix semantics after symlink resolution: a more specific
	// canonical mount wins even when the preferred mount is a parent.
	if (lexicalMatch.canonicalPrefixLength != canonicalMatch.canonicalPrefixLength)
	{
		return lexicalMatch.canonicalPrefixLength > canonicalMatch.canonicalPrefixLength
			? lexicalMatch
			: canonicalMatch;
	}

	if (pathResolvedThroughLink)
	{
		return lexicalMatch;
	}

	if (canonicalMatch.isPreferredPrefix && !lexicalMatch.isPreferredPrefix)
	{
		return canonicalMatch;
	}

	return lexicalMatch;
}

}

namespace vfs
{
ManagerServer::ManagerServer()
{
	m_fallbackDevice = new LocalDevice();
}

fwRefContainer<Device> MakeMemoryDevice();

fwRefContainer<Device> ManagerServer::FindDevice(const std::string& absolutePath, std::string& transformedPath)
{
	return FindDevice(absolutePath, transformedPath, {});
}

fwRefContainer<Device> ManagerServer::FindDevice(const std::string& absolutePath, std::string& transformedPath, const std::string& preferredMountPrefix)
{
	transformedPath.clear();

	std::filesystem::path absolute(absolutePath);
	std::string absoluteGenericString = GetLexicalAbsoluteGenericPath(absolute);
	AddDirectorySuffix(absoluteGenericString, absolute);

	auto findDevice = [&](const std::string& path, const DevicePathKind pathKind)
	{
		DeviceMatch foundMatch;

		for (const auto& mount : m_mounts)
		{
			for (const auto& mountedDevice : mount.devices)
			{
				const std::string& deviceAbsolutePath = pathKind == DevicePathKind::Canonical
					? mountedDevice.canonicalPath
					: mountedDevice.absolutePath;
				if (deviceAbsolutePath.empty())
				{
					continue;
				}

				size_t prefixLength = deviceAbsolutePath.size();

				// device matches if the path start is the same
				// e.g. usr/local/bin starts with /usr/local/
				if (path.rfind(deviceAbsolutePath, 0) == 0)
				{
					const bool isPreferredPrefix = !preferredMountPrefix.empty() && mount.prefix == preferredMountPrefix;
					std::string nextTransformedPath = mount.prefix + path.substr(prefixLength);
					size_t canonicalPrefixLength = prefixLength;
					if (pathKind == DevicePathKind::Lexical &&
						mountedDevice.resolvesThroughLink &&
						!mountedDevice.canonicalPath.empty())
					{
						canonicalPrefixLength = mountedDevice.canonicalPath.size();
					}

					// find the device with the largest prefix length
					UpdateDeviceMatch(foundMatch, prefixLength, canonicalPrefixLength, mountedDevice.device,
						std::move(nextTransformedPath), isPreferredPrefix);
				}
			}
		}

		return foundMatch;
	};

	auto returnMatch = [&](DeviceMatch& match)
	{
		if (match.device.GetRef())
		{
			transformedPath = std::move(match.transformedPath);
		}

		return match.device;
	};

	{
		std::lock_guard<std::recursive_mutex> lock(m_mountMutex);
		DeviceMatch lexicalMatch = findDevice(absoluteGenericString, DevicePathKind::Lexical);

		if (lexicalMatch.device.GetRef() && (preferredMountPrefix.empty() || lexicalMatch.isPreferredPrefix))
		{
			return returnMatch(lexicalMatch);
		}

		if (!m_hasCanonicalPathMounts)
		{
			return returnMatch(lexicalMatch);
		}
	}

	std::string canonicalGenericString = GetCanonicalAbsoluteGenericPath(absolute);
	AddDirectorySuffix(canonicalGenericString, absolute);
	const bool pathResolvedThroughLink = absoluteGenericString != canonicalGenericString;

	std::lock_guard<std::recursive_mutex> lock(m_mountMutex);
	DeviceMatch lexicalMatch = findDevice(absoluteGenericString, DevicePathKind::Lexical);
	if (lexicalMatch.device.GetRef() && (preferredMountPrefix.empty() || lexicalMatch.isPreferredPrefix))
	{
		return returnMatch(lexicalMatch);
	}

	if (!m_hasCanonicalPathMounts)
	{
		return returnMatch(lexicalMatch);
	}

	DeviceMatch canonicalMatch = findDevice(canonicalGenericString, DevicePathKind::Canonical);
	DeviceMatch selectedMatch = SelectDeviceMatch(lexicalMatch, canonicalMatch, pathResolvedThroughLink);

	return returnMatch(selectedMatch);
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
				return mount.devices[0].device;
			}
			else
			{
				// check each device assigned to the mount point
				Device::THandle handle;

				for (const auto& mountedDevice : mount.devices)
				{
					const auto& device = mountedDevice.device;
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

	const std::string devicePath = device->GetAbsolutePath();
	const std::string absolutePath = GetMountedDevicePath(devicePath, false);
	std::string canonicalPath = GetMountedDevicePath(absolutePath, true);
	const bool resolvesThroughLink = !canonicalPath.empty() && canonicalPath != absolutePath;
	MountedDevice mountedDevice {
		device,
		absolutePath,
		std::move(canonicalPath),
		resolvesThroughLink
	};

	// mount
	std::lock_guard<std::recursive_mutex> lock(m_mountMutex);

	// find an existing mount in the mount list to add to
	for (auto&& mount : m_mounts)
	{
		if (mount.prefix == path)
		{
			mount.devices.push_back(mountedDevice);
			m_hasCanonicalPathMounts = m_hasCanonicalPathMounts || resolvesThroughLink;
			return;
		}
	}

	// if we're here, we didn't find any existing device - add a new one instead
	{
		MountPoint mount;
		mount.prefix = path;
		mount.devices.push_back(mountedDevice);

		m_mounts.insert(mount);
		m_hasCanonicalPathMounts = m_hasCanonicalPathMounts || resolvesThroughLink;
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

	m_hasCanonicalPathMounts = false;
	for (const auto& mount : m_mounts)
	{
		for (const auto& mountedDevice : mount.devices)
		{
			m_hasCanonicalPathMounts = m_hasCanonicalPathMounts || mountedDevice.resolvesThroughLink;
			if (m_hasCanonicalPathMounts)
			{
				return;
			}
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
