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

#include <openssl/sha.h>

#include <VFSError.h>

#include <pplawait.h>
#include <agents.h>
#include <experimental/resumable>

#include <concurrent_queue.h>

#include <CoreConsole.h>
#include <Error.h>

#include <IteratorView.h>

extern tbb::concurrent_unordered_map<std::string, bool> g_stuffWritten;
extern std::unordered_multimap<std::string, std::pair<std::string, std::string>> g_referenceHashList;

int GetWeightForFileName(const std::string& fileName);

namespace resources
{
size_t RcdBaseStream::GetLength()
{
	return m_fetcher->GetLength(m_fileName);
}

bool RcdBaseStream::EnsureRead(const std::function<void(bool, const std::string&)>& cb)
{
	if (!m_parentDevice.GetRef() || m_parentHandle == INVALID_DEVICE_HANDLE)
	{
		try
		{
			auto task = m_fetcher->FetchEntry(m_fileName);

			if (task == concurrency::task<RcdFetchResult>{ })
			{
				throw RcdFetchFailedException(fmt::sprintf("Failed to get entry for %s\n", m_fileName));
			}

			if (cb)
			{
				task.then([this, cb](concurrency::task<RcdFetchResult> task)
				{
					try
					{
						task.get();

						cb(true, {});
					}
					catch (const std::exception& e)
					{
						m_fetcher->PropagateError(e.what());

						cb(false, e.what());
					}
				});
			}

			if (!m_fetcher->IsBlocking())
			{
				if (!task.is_done())
				{
					return false;
				}
			}

			const auto& result = task.get();
			m_metaData = result.metaData;

			const auto& localPath = result.localPath;
			m_parentDevice = vfs::GetDevice(localPath);
			assert(m_parentDevice.GetRef());

			m_parentHandle = OpenFile(localPath);
			assert(m_parentHandle != INVALID_DEVICE_HANDLE);
		}
		catch (const RcdFetchFailedException& e)
		{
			m_fetcher->UnfetchEntry(m_fileName);

			throw;
		}
		catch (const std::exception& e)
		{
			// propagate throw for nonblocking
			if (!m_fetcher->IsBlocking())
			{
				m_fetcher->UnfetchEntry(m_fileName);

				throw;
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
		m_fetcher->PropagateError(e.what());

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
		m_fetcher->PropagateError(e.what());

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
	// if this is a read request for 'real' size data, satisfy it with such
	if (ptr == 0 && size == 16 && m_realSize > 0)
	{
		uint8_t* data = (uint8_t*)outBuffer;
		memset(data, 0, size);
		data[7] = ((m_realSize >> 0) & 0xFF);
		data[14] = ((m_realSize >> 8) & 0xFF);
		data[5] = ((m_realSize >> 16) & 0xFF);
		data[2] = ((m_realSize >> 24) & 0xFF);
		return 16;
	}

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
		m_fetcher->PropagateError(e.what());

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
	auto entry = GetEntryForFileName(fileName);

	if (!entry)
	{
		return {};
	}

	*ptr = 0;

	return std::make_shared<RcdBulkStream>(static_cast<RcdFetcher*>(this), fileName, GetRealSize(entry->get()));
}

size_t ResourceCacheDeviceV2::GetRealSize(const ResourceCacheEntryList::Entry& entry)
{
	size_t realSize = 0;

	if (entry.size == 0xFFFFFF)
	{
		const auto& extData = entry.extData;

		if (auto it = extData.find("rawSize"); it != extData.end())
		{
			realSize = std::stoi(it->second);
		}
	}

	return realSize;
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

	if (g_stuffWritten.find(localPath) == g_stuffWritten.end())
	{
		auto device = vfs::GetDevice(localPath);

		if (!device.GetRef())
		{
			return false;
		}

		if (device->GetAttributes(localPath) == -1)
		{
			return false;
		}
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

	if (it == ms_entries.end() || !it->second)
	{
		auto retTask = concurrency::create_task(std::bind(&ResourceCacheDeviceV2::DoFetch, this, *entry));

		if (it != ms_entries.end())
		{
			it->second = std::move(retTask);
		}
		else
		{
			it = ms_entries.emplace(referenceHash, std::move(retTask)).first;
		}
	}

	return *it->second;
}

void ResourceCacheDeviceV2::UnfetchEntry(const std::string& fileName)
{
	auto entry = GetEntryForFileName(fileName);

	if (entry)
	{
		std::unique_lock<std::mutex> lock(ms_lock);

		const auto& e = entry->get();
		const auto& referenceHash = e.referenceHash;
		auto it = ms_entries.find(referenceHash);

		if (it != ms_entries.end())
		{
			it->second = {};
		}
	}
}

static ConVar<int>* g_downloadBackoff;

// from https://learn.microsoft.com/en-us/cpp/parallel/concrt/how-to-create-a-task-that-completes-after-a-delay?view=msvc-170
concurrency::task<void> complete_after(unsigned int timeout)
{
	using namespace concurrency;

	// A task completion event that is set when a timer fires.
	task_completion_event<void> tce;

	// Create a non-repeating timer.
	auto fire_once = std::make_shared<timer<int>>(timeout, 0, nullptr, false);
	// Create a call object that sets the completion event after the timer fires.
	auto callback = std::make_shared<call<int>>([tce](int)
	{
		tce.set();
	});

	// Connect the timer to the callback and start the timer.
	fire_once->link_target(callback.get());
	fire_once->start();

	// Create a task that completes after the completion event is set.
	task<void> event_set(tce);

	// Create a continuation task that cleans up resources and
	// and return that continuation task.
	return event_set.then([callback = std::move(callback), fire_once = std::move(fire_once)]()
	{
	});
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

	int tries = 0;
	std::string lastError = "Unknown error.";
	bool downloaded = false;

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
				SHA_CTX sha1;
				size_t numRead;

				size_t readNow = 0;
				size_t readTotal = localStream->GetLength();

				// initialize context
				SHA1_Init(&sha1);

				// read from the stream
				while ((numRead = localStream->Read(data.data(), data.size())) > 0)
				{
					if (numRead == -1)
					{
						break;
					}

					readNow += numRead;
					fx::OnCacheVerifyStatus(fmt::sprintf("%s%s/%s", m_pathPrefix, entry.resourceName, entry.basename), readNow, readTotal);

					SHA1_Update(&sha1, reinterpret_cast<char*>(&data[0]), numRead);
				}

				// get the hash result and convert it to a string
				uint8_t hash[20];
				SHA1_Final(hash, &sha1);

				auto hashString = fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
					hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]);

				if (hashString == entry.referenceHash || (downloaded && entry.referenceHash.empty()))
				{
					fillResult(*cacheEntry);
				}
				else
				{
					trace(__FUNCTION__ ": %s hash %s does not match %s - redownloading\n",
						entry.basename,
						hashString,
						entry.referenceHash);

					lastError = fmt::sprintf("%s hash %s does not match %s",
						entry.basename,
						hashString,
						entry.referenceHash);
				}
			}
		}
		else if (downloaded)
		{
			lastError = "Failed to add entry to local storage (download corrupted?)";
		}
		
		if (!result)
		{
			using FetchResultT = std::tuple<bool, std::variant<size_t, std::string>>;

			concurrency::task_completion_event<FetchResultT> tce;

			std::string outFileName = fmt::sprintf("%s/unconfirmed/%s_%08x", m_cache->GetCachePath(), "cache", HashString(entry.referenceHash.c_str()));

			// log the request starting
			uint32_t initTime = timeGetTime();

			console::DPrintf("citizen:resources:client", __FUNCTION__ " downloading %s (hash %s) from %s\n",
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

			options.weight = ::GetWeightForFileName(entry.basename);

			std::string connectionToken;
			if (Instance<ICoreGameInit>::Get()->GetData("connectionToken", &connectionToken))
			{
				options.headers["X-CitizenFX-Token"] = connectionToken;
			}

			options.addErrorBody = true;

			std::string referenceHash = entry.referenceHash;

			auto req = Instance<HttpClient>::Get()->DoFileGetRequest(entry.remoteUrl, vfs::GetDevice(outFileName), outFileName, options, [this, referenceHash, tce, outFileName](bool result, const char* errorData, size_t outSize)
			{
				RemoveHttpRequest(referenceHash);

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

			StoreHttpRequest(referenceHash, req);

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

				downloaded = true;

				console::DPrintf("citizen:resources:client", "ResourceCacheDevice: downloaded %s in %d msec (size %d)\n", entry.basename, (timeGetTime() - initTime), size);
			}
			else
			{
				auto error = std::get<std::string>(std::get<1>(fetchResult));

				lastError = fmt::sprintf("Failure downloading %s: %s", entry.basename, error);
				trace("^3ResourceCacheDevice reporting failure downloading %s: %s\n", entry.basename, error);

				co_await complete_after(g_downloadBackoff->GetValue() * pow(2, tries + 1));
			}
		}

		if (tries >= 4)
		{
			throw RcdFetchFailedException(lastError);
		}

		tries++;
	} while (!result);

	co_return *result;
}

fwRefContainer<vfs::Stream> ResourceCacheDeviceV2::GetVerificationStream(const ResourceCacheEntryList::Entry& entry, const ResourceCache::Entry& cacheEntry)
{
	return vfs::OpenRead(cacheEntry.GetLocalPath());
}

void ResourceCacheDeviceV2::AddEntryToCache(const std::string& outFileName, std::map<std::string, std::string>& metaData, const ResourceCacheEntryList::Entry& entry)
{
	m_cache->AddEntry(outFileName, metaData);
}

void ResourceCacheDeviceV2::StoreHttpRequest(const std::string& hash, const HttpRequestPtr& request)
{
	int changeWeight = -1;

	{
		std::shared_lock _(m_pendingRequestWeightsMutex);
		if (auto it = m_pendingRequestWeights.find(hash); it != m_pendingRequestWeights.end())
		{
			changeWeight = it->second;
		}
	}

	if (changeWeight != -1)
	{
		request->SetRequestWeight(changeWeight);

		std::unique_lock _(m_pendingRequestWeightsMutex);
		m_pendingRequestWeights.erase(hash);
	}

	{
		std::unique_lock _(m_requestMapMutex);
		m_requestMap[hash] = request;
	}
}

void ResourceCacheDeviceV2::RemoveHttpRequest(const std::string& hash)
{
	std::unique_lock _(m_requestMapMutex);
	m_requestMap.erase(hash);
}

void ResourceCacheDeviceV2::SetRequestWeight(const std::string& hash, int newWeight)
{
	HttpRequestPtr requestPtr;

	{
		std::shared_lock _(m_requestMapMutex);
		if (auto it = m_requestMap.find(hash); it != m_requestMap.end())
		{
			requestPtr = it->second;
		}
	}

	if (requestPtr)
	{
		requestPtr->SetRequestWeight(newWeight);
	}
	else if (newWeight != -1)
	{
		std::unique_lock _(m_pendingRequestWeightsMutex);
		m_pendingRequestWeights[hash] = newWeight;
	}
	else // if !requestPtr && newWeight == -1
	{
		std::unique_lock _(m_pendingRequestWeightsMutex);
		m_pendingRequestWeights.erase(hash);
	}
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

#ifndef IS_RDR3
struct ResourceFlags
{
	uint32_t flag1;
	uint32_t flag2;
};
#else
struct ResourceFlags
{
	uint32_t magic; // 'RSC8'
	uint32_t version;
	uint32_t virtPages;
	uint32_t physPages;
	uint64_t fileSize;
	uint64_t fileTime;
};
#endif

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

#define VFS_RCD_REQUEST_HANDLE 0x30003

struct RequestHandleExtension
{
	vfs::Device::THandle handle;
	std::function<void(bool success, const std::string& error)> onRead;
};

#define VFS_RCD_SET_WEIGHT 0x30004

struct SetWeightExtension
{
	const char* fileName;
	int newWeight;
};

struct tp_work
{
	explicit tp_work(std::function<void()>&& cb)
	{
		auto data = new work_data(std::move(cb));
		work = CreateThreadpoolWork(handle_work, data, NULL);
	}

	tp_work(const tp_work&) = delete;

	void post()
	{
		SubmitThreadpoolWork(work);
	}

private:
	static VOID CALLBACK handle_work(
	_Inout_ PTP_CALLBACK_INSTANCE Instance,
	_Inout_opt_ PVOID Context,
	_Inout_ PTP_WORK Work)
	{
		work_data* wd = (work_data*)Context;
		wd->cb();

		delete wd;
		CloseThreadpoolWork(Work);
	}

	struct work_data
	{
		explicit work_data(std::function<void()>&& cb)
			: cb(std::move(cb))
		{
			
		}

		std::function<void()> cb;
	};

	PTP_WORK work;
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

#ifdef IS_RDR3
			SYSTEMTIME st;
			GetSystemTime(&st);

			FILETIME ft;
			SystemTimeToFileTime(&st, &ft);

			data->version = atoi(extData["rscVersion"].c_str());
			data->flags.magic = 0x38435352;
			data->flags.fileSize = entry->get().size;
			data->flags.fileTime = ((uint64_t(ft.dwHighDateTime) << 32) | ft.dwLowDateTime);
			data->flags.version = atoi(extData["rscVersion"].c_str());
			data->flags.virtPages = strtoul(extData["rscPagesVirtual"].c_str(), nullptr, 10);
			data->flags.physPages = strtoul(extData["rscPagesPhysical"].c_str(), nullptr, 10);
#elif defined(GTA_FIVE)
			data->version = atoi(extData["rscVersion"].c_str());
			data->flags.flag1 = strtoul(extData["rscPagesVirtual"].c_str(), nullptr, 10);
			data->flags.flag2 = strtoul(extData["rscPagesPhysical"].c_str(), nullptr, 10);
#elif defined(GTA_NY)
			data->version = atoi(extData["rscVersion"].c_str());
			data->flags.flag1 = strtoul(extData["rscPagesVirtual"].c_str(), nullptr, 10);
			// flag2 would be out of bounds
#endif
			return true;
		}
	}
	else if (controlIdx == VFS_RCD_REQUEST_HANDLE)
	{
		struct HandleContainer
		{
			explicit HandleContainer(ResourceCacheDeviceV2* self, vfs::Device::THandle hdl)
				: self(self), hdl(hdl)
			{
				
			}

			~HandleContainer()
			{
				self->m_handleDeleteQueue.push(hdl);
			}

			ResourceCacheDeviceV2* self;
			vfs::Device::THandle hdl;
		};

		RequestHandleExtension* data = (RequestHandleExtension*)controlData;

		auto handle = std::make_shared<HandleContainer>(this, data->handle);
		auto hd = GetHandle(handle->hdl);
		auto cb = data->onRead;

		tp_work work{ [this, handle, hd, cb]()
			{
				{
					THandle hdl;

					while (m_handleDeleteQueue.try_pop(hdl))
					{
						CloseBulk(hdl);
					}
				}

				try
				{
					hd->bulkStream->EnsureRead([this, handle, cb](bool success, const std::string& error)
					{
						cb(success, error);
					});
				}
				catch (const resources::RcdFetchFailedException& e)
				{
					cb(false, e.what());
				}
			} };

		work.post();
	}
	else if (controlIdx == VFS_GET_DEVICE_LAST_ERROR)
	{
		vfs::GetLastErrorExtension* data = (vfs::GetLastErrorExtension*)controlData;

		data->outError = m_lastError;

		return true;
	}
	else if (controlIdx == VFS_RCD_SET_WEIGHT)
	{
		auto data = (SetWeightExtension*)controlData;
		auto entry = GetEntryForFileName(data->fileName);

		if (entry)
		{
			SetRequestWeight(entry->get().referenceHash, data->newWeight);
		}

		return true;
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
					SHA_CTX sha1;
					size_t numRead;

					// initialize context
					SHA1_Init(&sha1);

					// read from the stream
					while ((numRead = localStream->Read(data.data(), data.size())) > 0)
					{
						if (numRead == -1)
						{
							break;
						}

						SHA1_Update(&sha1, reinterpret_cast<char*>(&data[0]), numRead);
					}

					// get the hash result and convert it to a string
					uint8_t hash[20];
					SHA1_Final(hash, &sha1);

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
tbb::concurrent_unordered_map<std::string, std::optional<concurrency::task<RcdFetchResult>>> ResourceCacheDeviceV2::ms_entries;
}

void MountResourceCacheDeviceV2(std::shared_ptr<ResourceCache> cache)
{
	vfs::Mount(new resources::ResourceCacheDeviceV2(cache, true), "cache:/");
	vfs::Mount(new resources::ResourceCacheDeviceV2(cache, false), "cache_nb:/");
}

static InitFunction initFunction([]
{
	resources::g_downloadBackoff = new ConVar<int>("cl_rcdFailureBackoff", ConVar_None, 500);
});
