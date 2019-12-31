#include "StdInc.h"

#include <ResourceCacheDeviceV2.h>

#include <mmsystem.h>

#include <mutex>
#include <optional>
#include <variant>

#include <tbb/concurrent_unordered_map.h>

#include <HttpClient.h>

#include <ResourceCache.h>
#include <ResourceManager.h>

#include <ICoreGameInit.h>
#include <StreamingEvents.h>

#include <SHA1.h>

#include <pplawait.h>
#include <experimental/resumable>

#include <Error.h>

#include <IteratorView.h>

extern std::unordered_multimap<std::string, std::pair<std::string, std::string>> g_referenceHashList;

namespace resources
{
size_t RcdBaseStream::GetLength()
{
	return m_fetcher->GetLength(m_fileName);
}

bool RcdBaseStream::EnsureRead()
{
	if (!m_parentDevice.GetRef() || m_parentHandle == INVALID_DEVICE_HANDLE)
	{
		try
		{
			auto task = m_fetcher->FetchEntry(m_fileName);

			if (m_fetcher->IsBlocking())
			{
				task.wait();
			}
			else
			{
				if (!task.is_done())
				{
					return false;
				}
			}

			m_metaData = task.get().metaData;

			const auto& localPath = task.get().localPath;
			m_parentDevice = vfs::GetDevice(localPath);
			assert(m_parentDevice.GetRef());

			m_parentHandle = OpenFile(localPath);
			assert(m_parentHandle != INVALID_DEVICE_HANDLE);
		}
		catch (const std::exception& e)
		{
			// propagate throw for nonblocking
			if (!m_fetcher->IsBlocking())
			{
				throw e;
			}

			FatalError("Unable to ensure read in RCD: %s\n\nPlease report this issue, together with the information from 'Save information' down below on https://forum.fivem.net/.", e.what());

			return false;
		}
	}

	return true;
}

RcdStream::~RcdStream()
{
	if (m_parentDevice.GetRef() && m_parentHandle != INVALID_DEVICE_HANDLE)
	{
		CloseFile();
		m_parentHandle = INVALID_DEVICE_HANDLE;
	}
}

size_t RcdStream::Read(void* outBuffer, size_t size)
{
	try
	{
		if (!EnsureRead())
		{
			return 0;
		}

		return m_parentDevice->Read(m_parentHandle, outBuffer, size);
	}
	catch (const std::exception& e)
	{
		trace(__FUNCTION__ ": failing read for %s\n", e.what());

		return -1;
	}
}

size_t RcdStream::Seek(intptr_t off, int at)
{
	try
	{
		if (!EnsureRead())
		{
			return -1;
		}

		return m_parentDevice->Seek(m_parentHandle, off, at);
	}
	catch (const std::exception& e)
	{
		trace(__FUNCTION__ ": failing seek for %s\n", e.what());

		return -1;
	}
}

void RcdStream::CloseFile()
{
	m_parentDevice->Close(m_parentHandle);
}

vfs::Device::THandle RcdStream::OpenFile(const std::string& localPath)
{
	return m_parentDevice->Open(localPath, true);
}

RcdBulkStream::~RcdBulkStream()
{
	if (m_parentDevice.GetRef() && m_parentHandle != INVALID_DEVICE_HANDLE)
	{
		CloseFile();
		m_parentHandle = INVALID_DEVICE_HANDLE;
	}
}

size_t RcdBulkStream::ReadBulk(uint64_t ptr, void* outBuffer, size_t size)
{
	if (size == 0xFFFFFFFC)
	{
		return m_fetcher->ExistsOnDisk(m_fileName) ? 2048 : 0;
	}

	try
	{
		if (!EnsureRead())
		{
			return 0;
		}

		if (size == 0xFFFFFFFE || size == 0xFFFFFFFD || size == 0xFFFFFFFC)
		{
			return 2048;
		}

		return m_parentDevice->ReadBulk(m_parentHandle, m_parentPtr + ptr, outBuffer, size);
	}
	catch (const std::exception& e)
	{
		trace(__FUNCTION__ ": failing read for %s\n", e.what());

		return -1;
	}
}

vfs::Device::THandle RcdBulkStream::OpenFile(const std::string& localPath)
{
	return m_parentDevice->OpenBulk(localPath, &m_parentPtr);
}

void RcdBulkStream::CloseFile()
{
	m_parentDevice->CloseBulk(m_parentHandle);
}

std::shared_ptr<RcdStream> ResourceCacheDeviceV2::OpenStream(const std::string& fileName, bool readOnly)
{
	if (!readOnly)
	{
		return {};
	}

	if (!GetEntryForFileName(fileName))
	{
		return {};
	}

	return std::make_shared<RcdStream>(static_cast<RcdFetcher*>(this), fileName);
}

std::shared_ptr<RcdStream> ResourceCacheDeviceV2::CreateStream(const std::string& fileName)
{
	return {};
}

std::shared_ptr<RcdBulkStream> ResourceCacheDeviceV2::OpenBulkStream(const std::string& fileName, uint64_t* ptr)
{
	if (!GetEntryForFileName(fileName))
	{
		return {};
	}

	*ptr = 0;

	return std::make_shared<RcdBulkStream>(static_cast<RcdFetcher*>(this), fileName);
}

bool ResourceCacheDeviceV2::ExistsOnDisk(const std::string& fileName)
{
	auto entry = GetEntryForFileName(fileName);

	if (!entry)
	{
		return false;
	}

	auto cacheEntry = m_cache->GetEntryFor(*entry);

	if (!cacheEntry)
	{
		return false;
	}

	const std::string& localPath = cacheEntry->GetLocalPath();
	auto device = vfs::GetDevice(localPath);

	if (!device.GetRef())
	{
		return false;
	}

	if (device->GetAttributes(localPath) == -1)
	{
		return false;
	}

	return true;
}

concurrency::task<RcdFetchResult> ResourceCacheDeviceV2::FetchEntry(const std::string& fileName)
{
	auto entry = GetEntryForFileName(fileName);

	if (!entry)
	{
		return {};
	}

	std::unique_lock<std::mutex> lock(ms_lock);

	const auto& e = entry->get();
	const auto& referenceHash = e.referenceHash;
	auto it = ms_entries.find(referenceHash);

	if (it == ms_entries.end())
	{
		it = ms_entries.emplace(referenceHash, concurrency::create_task(std::bind(&ResourceCacheDeviceV2::DoFetch, this, *entry))).first;
	}

	return it->second;
}

concurrency::task<RcdFetchResult> ResourceCacheDeviceV2::DoFetch(const ResourceCacheEntryList::Entry& entryRef)
{
	auto entry = entryRef;

	std::optional<RcdFetchResult> result;

	auto fillResult = [&result](const ResourceCache::Entry& entry)
	{
		result = {
			entry.GetLocalPath(),
			entry.GetMetaData()
		};
	};

	do
	{
		auto cacheEntry = m_cache->GetEntryFor(entry);

		if (cacheEntry)
		{
			const std::string& localPath = cacheEntry->GetLocalPath();
			auto localStream = GetVerificationStream(entry, *cacheEntry);

			if (localStream.GetRef())
			{
				std::array<uint8_t, 8192> data;
				sha1nfo sha1;
				size_t numRead;

				// initialize context
				sha1_init(&sha1);

				// read from the stream
				while ((numRead = localStream->Read(data.data(), data.size())) > 0)
				{
					if (numRead == -1)
					{
						break;
					}

					sha1_write(&sha1, reinterpret_cast<char*>(&data[0]), numRead);
				}

				// get the hash result and convert it to a string
				uint8_t* hash = sha1_result(&sha1);

				auto hashString = fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
					hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]);

				if (hashString == entry.referenceHash)
				{
					fillResult(*cacheEntry);
				}
				else
				{
					trace(__FUNCTION__ ": %s hash %s does not match %s - redownloading\n",
						entry.basename,
						hashString,
						entry.referenceHash);
				}
			}
		}
		
		if (!result)
		{
			using FetchResultT = std::tuple<bool, std::variant<size_t, std::string>>;

			concurrency::task_completion_event<FetchResultT> tce;

			std::string outFileName = fmt::sprintf("%s/unconfirmed/%s_%08x", m_cache->GetCachePath(), "cache", HashString(entry.referenceHash.c_str()));

			// log the request starting
			uint32_t initTime = timeGetTime();

			trace(__FUNCTION__ " downloading %s (hash %s) from %s\n",
				entry.basename,
				entry.referenceHash.empty() ? "[direct]" : entry.referenceHash.c_str(),
				entry.remoteUrl);

			HttpRequestOptions options;
			options.progressCallback = [this, entry](const ProgressInfo& info)
			{
				if (info.downloadTotal != 0)
				{
					fx::OnCacheDownloadStatus(fmt::sprintf("%s%s/%s", m_pathPrefix, entry.resourceName, entry.basename), info.downloadNow, info.downloadTotal);
				}
			};

			//options.weight = GetWeightForFileName(handleData->entry.basename);

			std::string connectionToken;
			if (Instance<ICoreGameInit>::Get()->GetData("connectionToken", &connectionToken))
			{
				options.headers["X-CitizenFX-Token"] = connectionToken;
			}

			auto req = Instance<HttpClient>::Get()->DoFileGetRequest(entry.remoteUrl, vfs::GetDevice(outFileName), outFileName, options, [tce, outFileName](bool result, const char* errorData, size_t outSize)
			{
				if (result)
				{
					auto device = vfs::GetDevice(outFileName);
					outSize = device->GetLength(outFileName);

					tce.set({ true, outSize });
				}
				else
				{
					tce.set({ false, std::string(errorData, outSize) });
				}
			});

			auto fetchResult = co_await concurrency::task<FetchResultT>{tce};
			
			if (std::get<bool>(fetchResult))
			{
				auto size = std::get<size_t>(std::get<1>(fetchResult));

				// add the file to the resource cache
				std::map<std::string, std::string> metaData;
				metaData["filename"] = entry.basename;
				metaData["resource"] = entry.resourceName;
				metaData["from"] = entry.remoteUrl;
				metaData["reference"] = entry.referenceHash;

				AddEntryToCache(outFileName, metaData, entry);

				trace("ResourceCacheDevice: downloaded %s in %d msec (size %d)\n", entry.basename, (timeGetTime() - initTime), size);
			}
			else
			{
				auto error = std::get<std::string>(std::get<1>(fetchResult));

				trace("ResourceCacheDevice reporting failure: %s", error);
			}
		}
	} while (!result);

	return *result;
}

fwRefContainer<vfs::Stream> ResourceCacheDeviceV2::GetVerificationStream(const ResourceCacheEntryList::Entry& entry, const ResourceCache::Entry& cacheEntry)
{
	return vfs::OpenRead(cacheEntry.GetLocalPath());
}

void ResourceCacheDeviceV2::AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, const ResourceCacheEntryList::Entry& entry)
{
	m_cache->AddEntry(outFileName, metaData);
}

std::optional<std::reference_wrapper<const ResourceCacheEntryList::Entry>> ResourceCacheDeviceV2::GetEntryForFileName(std::string_view fileName)
{
	// strip the path prefix
	std::string_view relativeName = fileName.substr(m_pathPrefix.length());

	// relative paths are {resource}/{filepath}
	int slashOffset = relativeName.find_first_of('/');
	std::string_view resourceName = relativeName.substr(0, slashOffset);
	std::string_view itemName = relativeName.substr(slashOffset + 1);

	// get the relative resource
	fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
	fwRefContainer<fx::Resource> resource = resourceManager->GetResource(std::string(resourceName));

	// TODO: handle this some better way
	if (!resource.GetRef())
	{
		return {};
	}

	// get the entry list component
	fwRefContainer<ResourceCacheEntryList> entryList = resource->GetComponent<ResourceCacheEntryList>();

	// get the entry from the component
	auto entry = entryList->GetEntry(itemName);

	if (!entry)
	{
		return {};
	}

	return entry;
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

bool ResourceCacheDeviceV2::ExtensionCtl(int controlIdx, void* controlData, size_t controlSize)
{
	if (controlIdx == VFS_GET_RAGE_PAGE_FLAGS)
	{
		GetRagePageFlagsExtension* data = (GetRagePageFlagsExtension*)controlData;

		auto entry = GetEntryForFileName(data->fileName);

		if (entry)
		{
			auto extData = entry->get().extData;

			data->version = atoi(extData["rscVersion"].c_str());
			data->flags.flag1 = strtoul(extData["rscPagesVirtual"].c_str(), nullptr, 10);
			data->flags.flag2 = strtoul(extData["rscPagesPhysical"].c_str(), nullptr, 10);
			return true;
		}
	}
	else if (controlIdx == VFS_GET_RCD_DEBUG_INFO)
	{
		GetRcdDebugInfoExtension* data = (GetRcdDebugInfoExtension*)controlData;

		auto entry = GetEntryForFileName(data->fileName);

		if (entry)
		{
			auto extData = entry->get().extData;

			std::string diskHash = "<unknown>";

			auto cacheEntry = m_cache->GetEntryFor(*entry);

			if (cacheEntry)
			{
				auto localStream = GetVerificationStream(*entry, *cacheEntry);

				if (localStream.GetRef())
				{
					std::array<uint8_t, 8192> data;
					sha1nfo sha1;
					size_t numRead;

					// initialize context
					sha1_init(&sha1);

					// read from the stream
					while ((numRead = localStream->Read(data.data(), data.size())) > 0)
					{
						if (numRead == -1)
						{
							break;
						}

						sha1_write(&sha1, reinterpret_cast<char*>(&data[0]), numRead);
					}

					// get the hash result and convert it to a string
					uint8_t* hash = sha1_result(&sha1);

					diskHash = fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
						hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
						hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]);
				}
			}

			data->outData = fmt::sprintf("RSC version: %d\nRSC page flags: virt %08x/phys %08x\nResource name: %s\nReference hash: %s\nDisk hash: %s\nFile size: %d\n",
				atoi(extData["rscVersion"].c_str()),
				strtoul(extData["rscPagesVirtual"].c_str(), nullptr, 10),
				strtoul(extData["rscPagesPhysical"].c_str(), nullptr, 10),
				entry->get().resourceName,
				entry->get().referenceHash,
				diskHash,
				entry->get().size);

			data->outData += "Resources for hash:\n";

			for (auto& views : fx::GetIteratorView(g_referenceHashList.equal_range(entry->get().referenceHash)))
			{
				data->outData += fmt::sprintf("-> %s/%s\n", views.second.first, views.second.second);
			}

			return true;
		}
	}

	return false;
}

size_t ResourceCacheDeviceV2::GetLength(const std::string& fileName)
{
	auto entry = GetEntryForFileName(fileName);

	if (entry)
	{
		return entry->get().size;
	}

	return -1;
}

ResourceCacheDeviceV2::ResourceCacheDeviceV2(const std::shared_ptr<ResourceCache>& cache, bool blocking)
	: m_cache(cache), m_blocking(blocking)
{

}

std::mutex ResourceCacheDeviceV2::ms_lock;
tbb::concurrent_unordered_map<std::string, concurrency::task<RcdFetchResult>> ResourceCacheDeviceV2::ms_entries;
}

void MountResourceCacheDeviceV2(std::shared_ptr<ResourceCache> cache)
{
	vfs::Mount(new resources::ResourceCacheDeviceV2(cache, true), "cache:/");
	vfs::Mount(new resources::ResourceCacheDeviceV2(cache, false), "cache_nb:/");
}
