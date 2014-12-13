/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceVFS.h"
#include "ResourceVFSInternal.h"

namespace details
{
fwString ResolvePath(fwString inPath, bool write, const ResourceIdentity& sourceIdentity)
{
	// replace any backslashes with slashes
	std::replace(inPath.begin(), inPath.end(), '\\', '/');

	if (inPath.find("./") != std::string::npos)
	{
		trace("VFS path resolve: relative path indicators not allowed\n");
		return "";
	}

	// parse the path
	char* inStr = _strdup(inPath.c_str());
	char* p = inStr;
	bool local = false;
	fwString sourceResource = sourceIdentity.GetName();
	fwString outPath("");

	// is this a local path (server-specific)?
	if (*p == '@')
	{
		local = true;
		p++;
	}

	// is this a file from a different resource?
	if (*p == ':')
	{
		p++;

		// get the resource name after the :
		const char* resName = p;

		p = strchr(p, '/');

		// null-terminate it here
		if (p)
		{
			*p = '\0';
			p++;
		}

		sourceResource = resName;
	}

	// if we're still alive
	if (p)
	{
		// check if the source resource is running
		auto resource = TheResources.GetResource(sourceResource);

		if (resource.GetRef())
		{
			// if not wanting to write to the file
			if (!write)
			{
				// find the data file in the resource itself, first
				const char* dataFile = va("resources:/%s/%s", resource->GetName().c_str(), p);

				auto device = fiDevice::GetDevice(dataFile, true);

				if (device && device->getFileAttributes(dataFile) != INVALID_FILE_ATTRIBUTES)
				{
					outPath = dataFile;
				}
			}

			if (outPath.empty())
			{
				// try a local path
				fwString dataFile = va("rescache:/storage/%s/global/%s", resource->GetName().c_str(), p);

				fiDevice* device = nullptr;

				// if this is for writing, create all root paths first
				if (write)
				{
					device = fiDevice::GetDevice("rescache:/", true);

					device->mkdir("rescache:/storage");
					device->mkdir(va("rescache:/storage/%s/", resource->GetName().c_str()));
					device->mkdir(va("rescache:/storage/%s/global/", resource->GetName().c_str()));
				}
				else
				{
					device = fiDevice::GetDevice(dataFile.c_str(), true);

					if (!device)
					{
						trace("VFS path resolve: no such file for reading %s\n", inStr);
					}
				}

				if (device)
				{
					outPath = dataFile;
				}
			}
		}
		else
		{
			trace("VFS path resolve: no resource found by name %s\n", sourceResource.c_str());
		}
	}
	else
	{
		trace("VFS path resolve: path %s lacked slashes\n", inPath.c_str());
	}

	free(inStr);

	return outPath;
}

template<bool Write, bool Append>
fwRefContainer<VFSStream> OpenFile(const char* filePath, const ResourceIdentity sourceIdentity)
{
	// resolve the path
	fwString nativePath = details::ResolvePath(filePath, false, sourceIdentity);

	if (nativePath.empty())
	{
		// invalid path
		return nullptr;
	}

	// get a device for the path
	fiDevice* device = fiDevice::GetDevice(nativePath.c_str(), true);

	// if not valid, return
	if (!device)
	{
		return nullptr;
	}

	uint32_t handle = device->open(nativePath.c_str(), !Write);

	if (handle == -1)
	{
		trace("VFS: could not open file %s\n", nativePath.c_str());
		return nullptr;
	}

	if (Write && !Append)
	{
		device->truncate(handle);
	}

	return new RageVFSStream(device, handle, false);
}
}

fwRefContainer<VFSStream> ResourceVFS::OpenFileRead(const char* filePath, const ResourceIdentity& sourceIdentity)
{
	return details::OpenFile<false, false>(filePath, sourceIdentity);
}

fwRefContainer<VFSStream> ResourceVFS::OpenFileWrite(const char* filePath, const ResourceIdentity& sourceIdentity)
{
	return details::OpenFile<true, false>(filePath, sourceIdentity);
}

fwRefContainer<VFSStream> ResourceVFS::OpenFileAppend(const char* filePath, const ResourceIdentity& sourceIdentity)
{
	return details::OpenFile<true, true>(filePath, sourceIdentity);
}

ResourceIdentity::ResourceIdentity(fwString name)
	: m_name(name)
{

}