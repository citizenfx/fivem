/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceCacheDevice.h"

#include <StreamingEvents.h>

#include <ResourceManager.h>

#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>

#include <ICoreGameInit.h>

#include <optick.h>

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include <Error.h>

#include <SHA1.h>

static concurrency::concurrent_unordered_map<uint32_t, std::shared_ptr<std::atomic<int32_t>>> g_downloadingSet;
static concurrency::concurrent_unordered_set<uint32_t> g_downloadedSet;

static concurrency::concurrent_unordered_map<std::string, std::shared_ptr<ResourceCacheDevice::FileData>> g_fileDataSet;

inline std::shared_ptr<ResourceCacheDevice::FileData> GetFileDataForEntry(const ResourceCacheEntryList::Entry& entry)
{
	auto refHash = (entry.referenceHash.empty()) ? entry.remoteUrl : entry.referenceHash;

	auto it = g_fileDataSet.find(refHash);

	if (it == g_fileDataSet.end())
	{
		auto ptr = std::make_shared<ResourceCacheDevice::FileData>();

		g_fileDataSet[refHash] = { ptr };

		return std::move(ptr);
	}

	return it->second;
}

ResourceCacheDevice::ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking)
	: ResourceCacheDevice(cache, blocking, cache->GetCachePath(), cache->GetPhysCachePath())
{

}

ResourceCacheDevice::ResourceCacheDevice(std::shared_ptr<ResourceCache> cache, bool blocking, const std::string& cachePath, const std::string& physCachePath)
	: m_cache(cache), m_blocking(blocking), m_cachePath(cachePath), m_physCachePath(physCachePath)
{
	m_httpClient = Instance<HttpClient>::Get();

	if (GetFileAttributes(MakeRelativeCitPath(L"bin\\aria2c.exe").c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		m_extDownloader = CreateExtDownloader();
	}
}

std::optional<ResourceCacheEntryList::Entry> ResourceCacheDevice::GetEntryForFileName(const std::string& fileName)
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
		return std::optional<ResourceCacheEntryList::Entry>();
	}

	// get the entry list component
	fwRefContainer<ResourceCacheEntryList> entryList = resource->GetComponent<ResourceCacheEntryList>();

	// get the entry from the component
	auto entry = entryList->GetEntry(itemName);

	if (!entry)
	{
		return {};
	}

	return entry->get();
}

ResourceCacheDevice::THandle ResourceCacheDevice::OpenInternal(const std::string& fileName, uint64_t* bulkPtr)
{
	// find the entry for this file
	auto entry = GetEntryForFileName(fileName);

	if (!entry)
	{
		return InvalidHandle;
	}

	// allocate a file handle
	THandle handle;
	auto handleData = AllocateHandle(&handle);

	// is this a bulk handle?
	handleData->bulkHandle = (bulkPtr != nullptr);
	handleData->entry = *entry;
	handleData->fileData = GetFileDataForEntry(*entry);
	handleData->fileData->status = FileData::StatusNotFetched;
	handleData->fileName = fileName;

	// open the file beforehand if it's in the cache
	auto cacheEntry = m_cache->GetEntryFor(*entry);
	
	if (cacheEntry)
	{
		const std::string& localPath = cacheEntry->GetLocalPath();
		handleData->localPath = localPath;

		auto parentDevice = vfs::GetDevice(handleData->localPath);

		if (parentDevice.GetRef())
		{
			if (parentDevice->GetAttributes(localPath) != INVALID_FILE_ATTRIBUTES)
			{
				std::unique_lock<std::mutex> lock(handleData->fileData->fetchLock);

				handleData->fileData->status = FileData::StatusFetched;
				handleData->fileData->metaData = cacheEntry->GetMetaData();

				MarkFetched(handleData);
			}
		}
	}

	return handle;
}

void ResourceCacheDevice::EnsureDeferredOpen(THandle handle, HandleData* handleData)
{
	if (handleData->parentDevice.GetRef() && handleData->parentHandle != InvalidHandle)
	{
		return;
	}

	if (handleData->localPath.empty())
	{
		return;
	}

	handleData->parentDevice = vfs::GetDevice(handleData->localPath);

	if (handleData->parentDevice.GetRef())
	{
		handleData->parentHandle = (handleData->bulkHandle) ? handleData->parentDevice->OpenBulk(handleData->localPath, &handleData->bulkPtr) : handleData->parentDevice->Open(handleData->localPath, true);

		if (handleData->parentHandle != InvalidHandle)
		{
			auto cacheEntry = m_cache->GetEntryFor(handleData->entry);

			{
				std::unique_lock<std::mutex> lock(handleData->fileData->fetchLock);

				handleData->fileData->status = FileData::StatusFetched;
				handleData->fileData->metaData = cacheEntry->GetMetaData();

				MarkFetched(handleData);
			}

			// validate the file
			if (handleData->bulkHandle)
			{
				bool valid = true;

				if (handleData->entry.extData.find("rscVersion") != handleData->entry.extData.end() && handleData->entry.extData["rscVersion"] != "0")
				{
					char rscHeader[4];
					this->ReadBulk(handle, 0, &rscHeader, 4);

					memcpy(handleData->fileData->rscHeader, rscHeader, 4);

					if (rscHeader[0] != 'R' || rscHeader[1] != 'S' || rscHeader[2] != 'C')
					{
						trace(__FUNCTION__ ": %s didn't parse as an RSC. Refetching?\n", handleData->fileName);

						AddCrashometry("rcd_invalid_resource", "true");

						valid = false;
					}
				}

				if (valid)
				{
					auto length = handleData->parentDevice->GetLength(handleData->localPath);

					// calculate a hash of the file
					std::vector<uint8_t> data(8192);
					sha1nfo sha1;
					size_t numRead;

					// initialize context
					sha1_init(&sha1);

					// read from the stream
					size_t off = 0;
					size_t toRead = std::min(data.size(), length - off);

					while ((numRead = this->ReadBulk(handle, off, data.data(), toRead)) > 0)
					{
						sha1_write(&sha1, reinterpret_cast<char*>(&data[0]), numRead);
						off += numRead;
						toRead = std::min(data.size(), length - off);
					}

					if (off < length)
					{
						AddCrashometry("rcd_corrupted_read", "true");

						valid = false;
					}

					if (valid)
					{
						// get the hash result and convert it to a string
						uint8_t* hash = sha1_result(&sha1);

						std::string h = fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
							hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
							hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]);

						if (h != cacheEntry->GetHashString())
						{
							AddCrashometry("rcd_corrupted_file", "true");

							valid = false;
						}
					}
				}

				if (!valid)
				{
					handleData->fileData->status = FileData::StatusNotFetched;

					// close the file so we can refetch it
					handleData->parentDevice->CloseBulk(handleData->parentHandle);

					// and remove it
					handleData->parentDevice->RemoveFile(handleData->localPath);

					// unset stuff so nobody gets the bright idea to think we have a handle
					handleData->parentDevice = {};
					handleData->parentHandle = InvalidHandle;
					handleData->localPath = {};
				}
			}
		}
	}
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
		if (!m_handles[i].allocated)
		{
			*idx = i;
			m_handles[i].allocated = true;
			m_handles[i].parentDevice = nullptr;
			m_handles[i].parentHandle = INVALID_DEVICE_HANDLE;

			return &m_handles[i];
		}
	}

	FatalError(__FUNCTION__ " - failed to allocate file handle");

	return nullptr;
}

static int GetWeightForFileName(const std::string& fileName)
{
	auto ext = fileName.substr(fileName.find_last_of('.'));

	if (ext == ".ybn" || ext == ".ymap" || ext == ".ytyp")
	{
		return 255;
	}
	else if (ext == ".ydd" || ext == ".ydr")
	{
		return 128;
	}
	else if (ext == ".ytd" || ext == ".rpf" || ext == ".gfx")
	{
		return 64;
	}
	else if (fileName.find("+hi") != std::string::npos || fileName.find("_hi") != std::string::npos)
	{
		return 16;
	}

	return 32;
}

bool ResourceCacheDevice::EnsureFetched(HandleData* handleData)
{
	OPTICK_EVENT();

	// is it fetched already?
	if (handleData->fileData->status == FileData::StatusFetched)
	{
		return true;
	}

	// file extension for cache stuff
	auto remoteHash = HashRageString((handleData->entry.referenceHash.empty()) ? handleData->entry.remoteUrl.c_str() : handleData->entry.referenceHash.c_str());

	std::string extension = handleData->entry.basename.substr(handleData->entry.basename.find_last_of('.') + 1);
	std::string outFileName = fmt::sprintf("%s/unconfirmed/%s_%08x", m_cachePath, extension, remoteHash);
	std::string outPhysFileName = fmt::sprintf("%s/unconfirmed/phys_%s_%08x", m_physCachePath, extension, remoteHash);

	auto openFile = [this, handleData](const ResourceCache::Entry& entry)
	{
		// open the file as desired
		handleData->parentDevice = vfs::GetDevice(entry.GetLocalPath());

		if (handleData->parentDevice.GetRef())
		{
			handleData->parentHandle = (handleData->bulkHandle) ?
				handleData->parentDevice->OpenBulk(entry.GetLocalPath(), &handleData->bulkPtr) :
				handleData->parentDevice->Open(entry.GetLocalPath(), true);
		}

		if (handleData->parentDevice.GetRef() && handleData->parentHandle != InvalidHandle)
		{
			MarkFetched(handleData);

			return true;
		}

		return false;
	};

	if (handleData->fileData->status == FileData::StatusFetching)
	{
		if (m_blocking)
		{
			OPTICK_EVENT("block on Fetching");

			WaitForSingleObject(handleData->fileData->eventHandle, INFINITE);

			if (handleData->fileData->status == FileData::StatusFetched && (handleData->parentDevice.GetRef() == nullptr || handleData->parentHandle == INVALID_DEVICE_HANDLE))
			{
				auto entry = m_cache->GetEntryFor(handleData->entry);

				if (entry)
				{
					return openFile(*entry);
				}
			}
		}

		return false;
	}

	OPTICK_EVENT("set StatusFetching");

	ResetEvent(handleData->fileData->eventHandle);
	handleData->fileData->status = FileData::StatusFetching;

	if (auto it = g_downloadingSet.find(remoteHash); (it == g_downloadingSet.end() || *it->second == 0))
	{
		// mark this hash as downloading (to prevent multiple concurrent downloads)
		if (it == g_downloadingSet.end())
		{
			it = g_downloadingSet.insert({ remoteHash, std::make_shared<std::atomic<int32_t>>() }).first;
		}

		(*g_downloadingSet[remoteHash])++;

		// log the request starting
		uint32_t initTime = timeGetTime();

		trace(__FUNCTION__ " downloading %s (hash %s) from %s\n",
			handleData->entry.basename.c_str(),
			handleData->entry.referenceHash.empty() ? "[direct]" : handleData->entry.referenceHash.c_str(),
			handleData->entry.remoteUrl.c_str());

		HttpRequestOptions options;
		options.progressCallback = [this, handleData](const ProgressInfo& info)
		{
			handleData->downloadProgress = info.downloadNow;
			handleData->downloadSize = info.downloadTotal;

			if (info.downloadTotal != 0)
			{
				fx::OnCacheDownloadStatus(fmt::sprintf("%s%s/%s", m_pathPrefix, handleData->entry.resourceName, handleData->entry.basename), info.downloadNow, info.downloadTotal);
			}
		};

		options.weight = GetWeightForFileName(handleData->entry.basename);

		std::string connectionToken;
		if (Instance<ICoreGameInit>::Get()->GetData("connectionToken", &connectionToken))
		{
			options.headers["X-CitizenFX-Token"] = connectionToken;
		}

		auto entryRef = handleData->entry;
		auto fileDataRef = handleData->fileData;

		auto onDownloaded = [=](bool result, const char* errorData, size_t outSize)
		{
			if (!result || outSize == 0)
			{
				fileDataRef->status = FileData::StatusError;

				if (handleData->fileData == fileDataRef)
				{
					handleData->getRequest = {};
				}

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
				trace("ResourceCacheDevice: downloaded %s in %d msec (size %d)\n", entryRef.basename.c_str(), (timeGetTime() - initTime), outSize);

				g_downloadedSet.insert(remoteHash);

				std::unique_lock<std::mutex> lock(fileDataRef->fetchLock);

				// add the file to the resource cache
				std::map<std::string, std::string> metaData;
				metaData["filename"] = entryRef.basename;
				metaData["resource"] = entryRef.resourceName;
				metaData["from"] = entryRef.remoteUrl;
				metaData["reference"] = entryRef.referenceHash;

				AddEntryToCache(outFileName, metaData, entryRef);

				fileDataRef->metaData = metaData;

				bool success = true;

				if (!m_blocking && handleData->fileData == fileDataRef)
				{
					auto entry = m_cache->GetEntryFor(entryRef);

					if (entry)
					{
						success = openFile(*entry);
					}
					else
					{
						trace("Failed to write valid entry for ResourceCacheDevice file %s - retrying?\n", entryRef.remoteUrl);

						success = false;
					}
				}

				if (success)
				{
					fileDataRef->status = FileData::StatusFetched;
				}

				if (handleData->fileData == fileDataRef)
				{
					handleData->getRequest = {};
				}
			}

			// unblock the mutex
			SetEvent(fileDataRef->eventHandle);

			// allow downloading this file again
			(*g_downloadingSet[remoteHash])--;
		};

		// http request
		if (m_extDownloader)
		{
			handleData->extHandle = m_extDownloader->StartDownload(handleData->entry.remoteUrl, outPhysFileName, options, [=]()
			{
				auto device = vfs::GetDevice(outPhysFileName);
				size_t outSize = device->GetLength(outPhysFileName);

				// copy the file to the target, slowly.
				std::vector<uint8_t> fileBuf(8192);

				{
					auto inStream = vfs::OpenRead(outPhysFileName);

					if (!inStream.GetRef())
					{
						onDownloaded(false, "huh", 3);
						return;
					}

					auto outDevice = vfs::GetDevice(m_cachePath);
					auto handle = outDevice->Create(outFileName);
					auto outStream = vfs::Stream{ outDevice, handle };

					size_t read = 0;

					do
					{
						read = inStream->Read(fileBuf);

						if (read > 0)
						{
							outStream.Write(fileBuf.data(), read);
						}
					} while (read != 0);

					outDevice->Close(handle);
				}

				device->RemoveFile(outPhysFileName);

				// done!
				onDownloaded(true, "", outSize);
			});
		}
		else
		{
			handleData->getRequest = m_httpClient->DoFileGetRequest(handleData->entry.remoteUrl, vfs::GetDevice(m_cachePath), outFileName, options, [=](bool result, const char* errorData, size_t outSize)
			{
				if (result)
				{
					auto device = vfs::GetDevice(outFileName);
					outSize = device->GetLength(outFileName);
				}

				onDownloaded(result, errorData, outSize);
			});
		}
	}

	if (m_blocking)
	{
		OPTICK_EVENT("block on NotFetched");

		while (handleData->fileData->status == FileData::StatusFetching)
		{
			WaitForSingleObject(handleData->fileData->eventHandle, 2500);
		}
	}

	if (handleData->fileData->status == FileData::StatusFetched && (handleData->parentDevice.GetRef() == nullptr || handleData->parentHandle == INVALID_DEVICE_HANDLE))
	{
		auto cacheEntry = m_cache->GetEntryFor(handleData->entry);

		if (!cacheEntry || !openFile(*cacheEntry))
		{
			ICoreGameInit* init = Instance<ICoreGameInit>::Get();

			init->SetData("rcd:error", "Failed in ResourceCacheDevice: corruption uncovered.");

			handleData->fileData->status = FileData::StatusError;
		}
	}

	return (handleData->fileData->status == FileData::StatusFetched);
}

void ResourceCacheDevice::AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, const ResourceCacheEntryList::Entry& entryRef)
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

	// open it if we can
	EnsureDeferredOpen(handle, handleData);

	// if the file isn't fetched, fetch it first
	bool fetched = EnsureFetched(handleData);

	if (m_blocking && !fetched)
	{
		while (!fetched)
		{
			Sleep(1000);

			fetched = EnsureFetched(handleData);
		}
	}

	// not fetched and non-blocking - return 0
	if (handleData->fileData->status == FileData::StatusNotFetched || handleData->fileData->status == FileData::StatusFetching)
	{
		return 0;
	}
	else if (handleData->fileData->status == FileData::StatusError)
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
	bool fetched = false;
	
	if (size != 0xFFFFFFFC)
	{
		fetched = EnsureFetched(handleData);
	}

	// special sentinel for PatchStreamingPreparation to determine if file is fetched
	if (size == 0xFFFFFFFE || size == 0xFFFFFFFD || size == 0xFFFFFFFC)
	{
		auto getRequest = handleData->getRequest;

		if (size != 0xFFFFFFFC)
		{
			int newWeight = (size == 0xFFFFFFFE) ? -1 : 1;

			if (getRequest)
			{
				// if FFFFFFFE, this is an active request; if FFFFFFFD, this isn't
				// no ExtensionCtl support exists for RageVFSDeviceAdapter yet, so we do it this way
				getRequest->SetRequestWeight(newWeight);
			}
			else
			{
				if (!handleData->extHandle.empty())
				{
					m_extDownloader->SetRequestWeight(handleData->extHandle, newWeight);
				}
			}
		}

		return (handleData->fileData->status == FileData::StatusFetched) ? 2048 : 0;
	}

	EnsureDeferredOpen(handle, handleData);

	if (fetched)
	{
		// fetched status might've changed
		fetched = EnsureFetched(handleData);
	}

	if (m_blocking && !fetched)
	{
		while (!fetched)
		{
			Sleep(1000);

			fetched = EnsureFetched(handleData);
		}
	}

	// not fetched and non-blocking - return 0
	if (handleData->fileData->status == FileData::StatusNotFetched || handleData->fileData->status == FileData::StatusFetching)
	{
		return 0;
	}
	else if (handleData->fileData->status == FileData::StatusError)
	{
		return -1;
	}

	if (m_blocking)
	{
		while (!handleData->parentDevice.GetRef() || handleData->parentHandle == InvalidHandle)
		{
			EnsureDeferredOpen(handle, handleData);

			fetched = EnsureFetched(handleData);

			if (!fetched)
			{
				Sleep(1000);
			}
		}
	}

	return handleData->parentDevice->ReadBulk(handleData->parentHandle, ptr + handleData->bulkPtr, outBuffer, size);
}

size_t ResourceCacheDevice::Seek(THandle handle, intptr_t offset, int seekType)
{
	// get the handle
	auto handleData = &m_handles[handle];

	// make sure the file is fetched
	if (handleData->parentDevice.GetRef() && handleData->parentHandle != InvalidHandle && handleData->fileData->status != FileData::StatusFetched)
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
	if (handleData->parentDevice.GetRef() && handleData->parentHandle != INVALID_DEVICE_HANDLE)
	{
		retval = handleData->parentDevice->Close(handleData->parentHandle);

		handleData->parentDevice = {};
		handleData->parentHandle = InvalidHandle;
	}

	// clear the handle and return
	handleData->fileData = nullptr;
	handleData->allocated = false;
	handleData->localPath = {};
	handleData->fileName = {};

	return retval;
}

bool ResourceCacheDevice::CloseBulk(THandle handle)
{
	// get the handle
	auto handleData = &m_handles[handle];

	bool retval = true;

	// close any parent device handle
	if (handleData->parentDevice.GetRef() && handleData->parentHandle != INVALID_DEVICE_HANDLE)
	{
		retval = handleData->parentDevice->CloseBulk(handleData->parentHandle);

		handleData->parentDevice = {};
		handleData->parentHandle = InvalidHandle;
	}

	// clear the handle and return
	handleData->fileData = nullptr;
	handleData->allocated = false;
	handleData->localPath = {};
	handleData->fileName = {};

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
	if (handleData->parentDevice.GetRef() && handleData->parentHandle != InvalidHandle && handleData->fileData->status == FileData::StatusFetched)
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

uint32_t ResourceCacheDevice::GetAttributes(const std::string& fileName)
{
	auto entry = GetEntryForFileName(fileName);

	if (entry)
	{
		return 0;
	}

	return -1;
}

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

struct ResourceFlags
{
	uint32_t flag1;
	uint32_t flag2;
};

struct GetRagePageFlagsExtension
{
	const char* fileName; // in
	int version;
	ResourceFlags flags; // out
};

#define VFS_GET_RCD_DEBUG_INFO 0x30001

struct GetRcdDebugInfoExtension
{
	const char* fileName; // in
	std::string outData; // out
};

static const char* StatusToString(ResourceCacheDevice::FileData::Status status)
{
	switch (status)
	{
	case ResourceCacheDevice::FileData::StatusEmpty:
		return "StatusEmpty";
	case ResourceCacheDevice::FileData::StatusFetched:
		return "StatusFetched";
	case ResourceCacheDevice::FileData::StatusFetching:
		return "StatusFetching";
	case ResourceCacheDevice::FileData::StatusNotFetched:
		return "StatusNotFetched";
	case ResourceCacheDevice::FileData::StatusError:
		return "StatusError";
	}
}

#include <IteratorView.h>

extern std::unordered_multimap<std::string, std::pair<std::string, std::string>> g_referenceHashList;

bool ResourceCacheDevice::ExtensionCtl(int controlIdx, void* controlData, size_t controlSize)
{
	if (controlIdx == VFS_GET_RAGE_PAGE_FLAGS)
	{
		GetRagePageFlagsExtension* data = (GetRagePageFlagsExtension*)controlData;

		auto entry = GetEntryForFileName(data->fileName);

		if (entry)
		{
			data->version = atoi(entry->extData["rscVersion"].c_str());
			data->flags.flag1 = strtoul(entry->extData["rscPagesVirtual"].c_str(), nullptr, 10);
			data->flags.flag2 = strtoul(entry->extData["rscPagesPhysical"].c_str(), nullptr, 10);
			return true;
		}
	}
	else if (controlIdx == VFS_GET_RCD_DEBUG_INFO)
	{
		GetRcdDebugInfoExtension* data = (GetRcdDebugInfoExtension*)controlData;

		auto entry = GetEntryForFileName(data->fileName);

		if (entry)
		{
			auto fileData = GetFileDataForEntry(*entry);

			data->outData = fmt::sprintf("RSC version: %d\nRSC page flags: virt %08x/phys %08x\nResource name: %s\nReference hash: %s\n", 
				atoi(entry->extData["rscVersion"].c_str()),
				strtoul(entry->extData["rscPagesVirtual"].c_str(), nullptr, 10),
				strtoul(entry->extData["rscPagesPhysical"].c_str(), nullptr, 10),
				entry->resourceName,
				entry->referenceHash);

			if (fileData)
			{
				auto remoteHash = HashRageString((entry->referenceHash.empty()) ? entry->remoteUrl.c_str() : entry->referenceHash.c_str());

				data->outData += fmt::sprintf("Status: %s\nDownloaded now: %s\nRSC header: %02x %02x %02x %02x\n\n",
					StatusToString(fileData->status),
					(g_downloadedSet.find(remoteHash) != g_downloadedSet.end()) ? "Yes" : "No", 
					fileData->rscHeader[0], fileData->rscHeader[1], fileData->rscHeader[2], fileData->rscHeader[3]);
			}

			data->outData += "Resources for hash:\n";

			for (auto& views : fx::GetIteratorView(g_referenceHashList.equal_range(entry->referenceHash)))
			{
				data->outData += fmt::sprintf("-> %s/%s\n", views.second.first, views.second.second);
			}

			return true;
		}
	}

	return false;
}

void ResourceCacheDevice::SetPathPrefix(const std::string& pathPrefix)
{
	m_pathPrefix = pathPrefix;
}

void ResourceCacheEntryList::AttachToObject(fx::Resource* resource)
{
	m_parentResource = resource;
}

void MountKvpDevice();

void MountResourceCacheDeviceV2(std::shared_ptr<ResourceCache> cache);

static bool g_useNewRcd = true;

void MountResourceCacheDevice(std::shared_ptr<ResourceCache> cache)
{
	if (g_useNewRcd)
	{
		MountResourceCacheDeviceV2(cache);
	}
	else
	{
		vfs::Mount(new ResourceCacheDevice(cache, true), "cache:/");
		vfs::Mount(new ResourceCacheDevice(cache, false), "cache_nb:/");
	}

	MountKvpDevice();
}
