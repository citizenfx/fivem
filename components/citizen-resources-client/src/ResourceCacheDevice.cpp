/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceCacheDevice.h"

#include <ResourceManager.h>

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include <Error.h>

ResourceCacheDevice::ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking)
	: ResourceCacheDevice(cache, blocking, cache->GetCachePath())
{

}

ResourceCacheDevice::ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking, const std::string& cachePath)
	: m_cache(cache), m_blocking(blocking), m_cachePath(cachePath)
{
	m_httpClient = Instance<HttpClient>::Get();
}

boost::optional<ResourceCacheEntryList::Entry> ResourceCacheDevice::GetEntryForFileName(const std::string& fileName)
{
	// strip the path prefix
	std::string relativeName = fileName.substr(m_pathPrefix.length());

	// relative paths are {resource}/{filepath}
	int slashOffset = relativeName.find_first_of('/');
	std::string resourceName = relativeName.substr(0, slashOffset);
	std::string itemName = relativeName.substr(slashOffset + 1);

	// get the relative resource
	fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
	fwRefContainer<fx::Resource> resource = resourceManager->GetResource(resourceName);

	// TODO: handle this some better way
	if (!resource.GetRef())
	{
		return boost::optional<ResourceCacheEntryList::Entry>();
	}

	// get the entry list component
	fwRefContainer<ResourceCacheEntryList> entryList = resource->GetComponent<ResourceCacheEntryList>();

	// get the entry from the component
	auto entry = entryList->GetEntry(itemName);

	return entry;
}

ResourceCacheDevice::THandle ResourceCacheDevice::OpenInternal(const std::string& fileName, uint64_t* bulkPtr)
{
	// find the entry for this file
	auto entry = GetEntryForFileName(fileName);

	if (!entry.is_initialized())
	{
		return InvalidHandle;
	}

	// allocate a file handle
	THandle handle;
	auto handleData = AllocateHandle(&handle);

	// is this a bulk handle?
	handleData->bulkHandle = (bulkPtr != nullptr);
	handleData->entry = entry.get();
	handleData->status = HandleData::StatusNotFetched;

	// open the file beforehand if it's in the cache
	auto cacheEntry = m_cache->GetEntryFor(entry->referenceHash);
	
	if (cacheEntry.is_initialized())
	{
		const std::string& localPath = cacheEntry->GetLocalPath();

		handleData->parentDevice = vfs::GetDevice(localPath);
		
		if (handleData->parentDevice.GetRef())
		{
			handleData->parentHandle = (bulkPtr) ? handleData->parentDevice->OpenBulk(localPath, &handleData->bulkPtr) : handleData->parentDevice->Open(localPath, true);

			if (handleData->parentHandle != InvalidHandle)
			{
				handleData->status = HandleData::StatusFetched;
				handleData->metaData = cacheEntry->GetMetaData();

				MarkFetched(handleData);
			}
		}
	}

	return handle;
}

ResourceCacheDevice::THandle ResourceCacheDevice::Open(const std::string& fileName, bool readOnly)
{
	// this is a read-only device
	if (!readOnly)
	{
		return InvalidHandle;
	}

	// open the file
	return OpenInternal(fileName, nullptr);
}

ResourceCacheDevice::THandle ResourceCacheDevice::OpenBulk(const std::string& fileName, uint64_t* ptr)
{
	*ptr = 0;

	return OpenInternal(fileName, ptr);
}

auto ResourceCacheDevice::AllocateHandle(THandle* idx) -> HandleData*
{
	std::lock_guard<std::mutex> lock(m_handleLock);

	for (int i = 0; i < _countof(m_handles); i++)
	{
		if (m_handles[i].status == HandleData::StatusEmpty)
		{
			*idx = i;
			m_handles[i].status = HandleData::StatusError;

			return &m_handles[i];
		}
	}

	FatalError(__FUNCTION__ " - failed to allocate file handle");

	return nullptr;
}

#include <ICoreGameInit.h>

bool ResourceCacheDevice::EnsureFetched(HandleData* handleData)
{
	// is it fetched already?
	if (handleData->status == HandleData::StatusFetched)
	{
		return true;
	}

	if (handleData->status == HandleData::StatusFetching)
	{
		if (m_blocking)
		{
			std::unique_lock<std::mutex> lock(handleData->lockMutex);
			handleData->lockVar.wait(lock);
		}

		return false;
	}

	handleData->status = HandleData::StatusFetching;

	// log the request starting
	uint32_t initTime = timeGetTime();

	trace(__FUNCTION__ " downloading %s (hash %s) from %s\n", handleData->entry.basename.c_str(), handleData->entry.referenceHash.c_str(), handleData->entry.remoteUrl.c_str());

	// file extension for cache stuff
	std::string extension = handleData->entry.basename.substr(handleData->entry.basename.find_last_of('.') + 1);
	std::string outFileName = m_cachePath + extension + "_" + handleData->entry.referenceHash;

	// http request
	m_httpClient->DoFileGetRequest(handleData->entry.remoteUrl, m_cachePath.c_str(), outFileName, [=] (bool result, const char* errorData, size_t outSize)
	{
		if (result)
		{
			auto device = vfs::GetDevice(outFileName);
			outSize = device->GetLength(outFileName);
		}

		if (!result || outSize == 0)
		{
			handleData->status = HandleData::StatusError;

			ICoreGameInit* init = Instance<ICoreGameInit>::Get();
			std::string reason;

			std::string caller;
			std::string initTime;

			if (init->GetData("gta-core-five:loadCaller", &caller))
			{
				if (!caller.empty())
				{
					init->GetData("gta-core-five:loadTime", &initTime);

					uint64_t time = GetTickCount64() - _atoi64(initTime.c_str());

					reason = fmt::sprintf("\nThis happened during a LoadObjectsNow call from %s, which by now took %d msec. Please report this.", caller, time);
				}
			}

			if (outSize == 0)
			{
				reason += "\nThe file was empty.";
			}

			trace("ResourceCacheDevice reporting failure: %s%s", errorData, reason);
			init->SetData("rcd:error", fmt::sprintf("Failed in ResourceCacheDevice: error result %s%s", errorData, reason));
		}
		else
		{
			// log success
			trace("ResourceCacheDevice: downloaded %s in %d msec (size %d)\n", handleData->entry.basename.c_str(), (timeGetTime() - initTime), outSize);

			// add the file to the resource cache
			std::map<std::string, std::string> metaData;
			metaData["filename"] = handleData->entry.basename;
			metaData["resource"] = handleData->entry.resourceName;
			metaData["from"] = handleData->entry.remoteUrl;

			AddEntryToCache(outFileName, metaData, handleData);

			// open the file as desired
			handleData->parentDevice = vfs::GetDevice(outFileName);

			if (handleData->parentDevice.GetRef())
			{
				handleData->parentHandle = (handleData->bulkHandle) ?
					handleData->parentDevice->OpenBulk(outFileName, &handleData->bulkPtr) : 
					handleData->parentDevice->Open(outFileName, true);
			}

			handleData->metaData = metaData;
			MarkFetched(handleData);

			handleData->status = HandleData::StatusFetched;
		}

		// unblock the mutex
		handleData->lockVar.notify_all();
	});

	if (m_blocking)
	{
		std::unique_lock<std::mutex> lock(handleData->lockMutex);
		handleData->lockVar.wait(lock);
	}

	return (handleData->status == HandleData::StatusFetched);
}

void ResourceCacheDevice::AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, HandleData* handleData)
{
	m_cache->AddEntry(outFileName, metaData);
}

void ResourceCacheDevice::MarkFetched(HandleData* handleData)
{

}

size_t ResourceCacheDevice::Read(THandle handle, void* outBuffer, size_t size)
{
	// get the handle
	auto handleData = &m_handles[handle];

	// if the file isn't fetched, fetch it first
	bool fetched = EnsureFetched(handleData);

	// not fetched and non-blocking - return 0
	if (handleData->status == HandleData::StatusNotFetched || handleData->status == HandleData::StatusFetching)
	{
		return 0;
	}
	else if (handleData->status == HandleData::StatusError)
	{
		return -1;
	}

	return handleData->parentDevice->Read(handleData->parentHandle, outBuffer, size);
}

size_t ResourceCacheDevice::ReadBulk(THandle handle, uint64_t ptr, void* outBuffer, size_t size)
{
	// get the handle
	auto handleData = &m_handles[handle];

	// if the file isn't fetched, fetch it first
	bool fetched = EnsureFetched(handleData);

	// not fetched and non-blocking - return 0
	if (handleData->status == HandleData::StatusNotFetched || handleData->status == HandleData::StatusFetching)
	{
		return 0;
	}
	else if (handleData->status == HandleData::StatusError)
	{
		return -1;
	}

	return handleData->parentDevice->ReadBulk(handleData->parentHandle, ptr + handleData->bulkPtr, outBuffer, size);
}

size_t ResourceCacheDevice::Seek(THandle handle, intptr_t offset, int seekType)
{
	// get the handle
	auto handleData = &m_handles[handle];

	// make sure the file is fetched
	if (handleData->status != HandleData::StatusFetched)
	{
		return -1;
	}

	return handleData->parentDevice->Seek(handleData->parentHandle, offset, seekType);
}

bool ResourceCacheDevice::Close(THandle handle)
{
	// get the handle
	auto handleData = &m_handles[handle];

	bool retval = true;

	// close any parent device handle
	if (handleData->status == HandleData::StatusFetched)
	{
		retval = handleData->parentDevice->Close(handleData->parentHandle);
	}

	// clear the handle and return
	handleData->status = HandleData::StatusEmpty;

	return retval;
}

bool ResourceCacheDevice::CloseBulk(THandle handle)
{
	// get the handle
	auto handleData = &m_handles[handle];

	bool retval = true;

	// close any parent device handle
	if (handleData->status == HandleData::StatusFetched)
	{
		retval = handleData->parentDevice->CloseBulk(handleData->parentHandle);
	}

	// clear the handle and return
	handleData->status = HandleData::StatusEmpty;

	return retval;
}

ResourceCacheDevice::THandle ResourceCacheDevice::FindFirst(const std::string& folder, vfs::FindData* findData)
{
	return -1;
}

bool ResourceCacheDevice::FindNext(THandle handle, vfs::FindData* findData)
{
	return false;
}

void ResourceCacheDevice::FindClose(THandle handle)
{
	
}

size_t ResourceCacheDevice::GetLength(THandle handle)
{
	auto handleData = &m_handles[handle];

	// close any parent device handle
	if (handleData->status == HandleData::StatusFetched)
	{
		return handleData->parentDevice->GetLength(handleData->parentHandle);
	}

	return handleData->entry.size;
}

size_t ResourceCacheDevice::GetLength(const std::string& fileName)
{
	auto entry = GetEntryForFileName(fileName);

	if (entry)
	{
		return entry->size;
	}

	return -1;
}

void ResourceCacheDevice::SetPathPrefix(const std::string& pathPrefix)
{
	m_pathPrefix = pathPrefix;
}

void ResourceCacheEntryList::AttachToObject(fx::Resource* resource)
{
	m_parentResource = resource;
}

void MountResourceCacheDevice(std::shared_ptr<ResourceCache> cache)
{
	vfs::Mount(new ResourceCacheDevice(cache, true), "cache:/");
	vfs::Mount(new ResourceCacheDevice(cache, false), "cache_nb:/");
}