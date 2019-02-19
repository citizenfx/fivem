#include "StdInc.h"
#include <ResourceStreamComponent.h>

#include <VFSManager.h>

#include <ResourceFileDatabase.h>

#include <botan/sha160.h>

#include <json.hpp>

template<typename TFunc>
static void IterateRecursively(const std::string& root, const TFunc& func)
{
	fwRefContainer<vfs::Device> device = vfs::GetDevice(root);

	std::vector<std::string> filenames;

	std::function<void(std::string)> recurse;
	recurse = [&](const std::string& parent)
	{
		std::vector<std::string> directories;

		vfs::FindData findData;
		auto handlef = device->FindFirst(parent, &findData);

		if (handlef != -1)
		{
			do
			{
				std::string fn = fmt::sprintf("%s/%s", parent, findData.name);

				if (findData.attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (findData.name[0] != '.')
					{
						directories.emplace_back(fn);
					}
				}
				else
				{
					filenames.emplace_back(fn);
				}
			} while (device->FindNext(handlef, &findData));

			device->FindClose(handlef);
		}

		for (auto& directory : directories)
		{
			recurse(directory);
		}
	};

	recurse(root);

	for (const auto& fn : filenames)
	{
		func(fn);
	}
}

namespace fx
{
	auto ResourceStreamComponent::GetStreamingList() -> const decltype(m_resourcePairs)&
	{
		return m_resourcePairs;
	}

	bool ResourceStreamComponent::ShouldUpdateSet()
	{
		auto fileName = fmt::sprintf("cache:/files/%s/resource.sfl", m_resource->GetName());
		auto device = vfs::GetDevice(fileName);

		if (!device.GetRef())
		{
			return true;
		}

		// open the resource database
		auto dbName = fileName + ".db";
		auto setDatabase = std::make_shared<ResourceFileDatabase>();

		if (!setDatabase->Load(dbName))
		{
			return true;
		}

		// collect a set of file names
		std::vector<std::string> files;

		IterateRecursively(fmt::sprintf("%s/stream/", m_resource->GetPath()), [&](const std::string& fullPath)
		{
			auto device = vfs::GetDevice(fullPath);

			if (device.GetRef())
			{
				files.push_back(fullPath);
			}
		});

		if (setDatabase->Check(files))
		{
			return true;
		}

		// load the list and verify if all files still exist
		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(fmt::sprintf("cache:/files/%s/resource.sfl", m_resource->GetName()));

		if (!stream.GetRef())
		{
			return true;
		}

		size_t numEntries = stream->GetLength() / sizeof(Entry);

		std::vector<Entry> entries(numEntries);
		stream->Read(&entries[0], entries.size() * sizeof(Entry));

		for (auto& entry : entries)
		{
			if (!vfs::OpenRead(entry.onDiskPath).GetRef())
			{
				return true;
			}
		}

		return false;
	}

	bool ResourceStreamComponent::UpdateSet()
	{
		std::vector<std::string> files;

		IterateRecursively(fmt::sprintf("%s/stream/", m_resource->GetPath()), [&](const std::string& fullPath)
		{
			files.push_back(fullPath);

			auto file = AddStreamingFile(fullPath);

			if (file)
			{
				file->isAutoScan = true;
			}
		});

		std::string outFileName = fmt::sprintf("cache:/files/%s/resource.sfl", m_resource->GetName());

		fwRefContainer<vfs::Device> device = vfs::GetDevice(outFileName);
		device->CreateDirectory(outFileName.substr(0, outFileName.find_last_of('/')));

		// first, save the resource database
		{
			auto dbName = outFileName + ".db";
			auto setDatabase = std::make_shared<ResourceFileDatabase>();

			setDatabase->Snapshot(files);
			setDatabase->Save(dbName);
		}

		// then, save the actual SFL
		auto handle = device->Create(outFileName);

		if (handle == INVALID_DEVICE_HANDLE)
		{
			return false;
		}

		fwRefContainer<vfs::Stream> stream(new vfs::Stream(device, handle));

		std::vector<Entry> entries(m_resourcePairs.size());

		size_t i = 0;
		for (auto& value : m_resourcePairs)
		{
			entries[i] = value.second;

			++i;
		}

		stream->Write(&entries[0], entries.size() * sizeof(Entry));

		return true;
	}

	auto ResourceStreamComponent::AddStreamingFile(const std::string& fullPath) -> StorageEntry*
	{
		if (fullPath.find(".stream_raw") != std::string::npos)
		{
			return nullptr;
		}

		Entry entry = { 0 };

		auto stream = vfs::OpenRead(fullPath);
		auto rawStream = vfs::OpenRead(fullPath + ".stream_raw");

		if (!stream.GetRef())
		{
			return nullptr;
		}

		entry.size = stream->GetLength();

		strncpy(entry.entryName, fullPath.substr(fullPath.find_last_of("/\\") + 1).c_str(), sizeof(entry.entryName));
		strncpy(entry.onDiskPath, fullPath.c_str(), sizeof(entry.onDiskPath));

		entry.rscFlags = entry.size;

		{
			auto rscStream = (rawStream.GetRef()) ? rawStream : stream;

			struct
			{
				uint32_t magic;
				uint32_t version;
				uint32_t virtPages;
				uint32_t physPages;
			} rsc7Header;

			rscStream->Read(&rsc7Header, sizeof(rsc7Header));

			if (rsc7Header.magic == 0x37435352) // RSC7
			{
				entry.rscVersion = rsc7Header.version;
				entry.rscPagesPhysical = rsc7Header.physPages;
				entry.rscPagesVirtual = rsc7Header.virtPages;
				entry.isResource = true;
			}

			rscStream->Seek(0, SEEK_SET);
		}

		{
			// calculate the file hash
			std::vector<uint8_t> data(8192);

			auto sha1 = std::make_unique<Botan::SHA_160>();
			size_t numRead;

			// read from the stream
			while ((numRead = stream->Read(data)) > 0)
			{
				sha1->update(&data[0], numRead);
			}

			// get the hash result and convert it to a string
			auto hash = sha1->final();

			strcpy(entry.hashString, fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
				hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]).c_str());
		}

		return AddStreamingFile(entry);
	}

	std::string ResourceStreamComponent::Entry::GetCacheString()
	{
		auto json = nlohmann::json::object({
			{ "hash", this->hashString },
			{"isResource", this->isResource },
			{ "rscPagesPhysical", this->rscPagesPhysical},
			{ "rscPagesVirtual", this->rscPagesVirtual},
			{ "rscVersion", this->rscVersion},
			{ "size", this->size}
		});

		return json.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
	}

	auto ResourceStreamComponent::AddStreamingFile(const std::string& entryName, const std::string& diskPath, const std::string& cacheString) -> StorageEntry*
	{
		try
		{
			auto json = nlohmann::json::parse(cacheString);
			
			if (json.is_object())
			{
				Entry entry = { 0 };
				strncpy(entry.entryName, entryName.c_str(), sizeof(entry.entryName) - 1);
				strncpy(entry.hashString, json.value("hash", "").c_str(), sizeof(entry.hashString) - 1);
				entry.isResource = json.value("isResource", false);
				strncpy(entry.onDiskPath, diskPath.c_str(), sizeof(entry.onDiskPath) - 1);
				entry.rscPagesPhysical = json.value("rscPagesPhysical", -1);
				entry.rscPagesVirtual = json.value("rscPagesVirtual", -1);
				entry.rscVersion = json.value("rscVersion", -1);
				entry.size = json.value("size", -1);

				return AddStreamingFile(entry);
			}
		}
		catch (std::exception&)
		{
		}

		return nullptr;
	}

	auto ResourceStreamComponent::AddStreamingFile(const Entry& entry) -> StorageEntry*
	{
		StorageEntry se(entry);
		se.isAutoScan = false;

		auto it = m_resourcePairs.insert({ entry.entryName, se }).first;
		return &it->second;
	}

	void ResourceStreamComponent::AttachToObject(fx::Resource* object)
	{
		m_resource = object;

		object->OnStart.Connect([=]()
		{
			if (ShouldUpdateSet())
			{
				UpdateSet();
			}
			else
			{
				fwRefContainer<vfs::Stream> stream = vfs::OpenRead(fmt::sprintf("cache:/files/%s/resource.sfl", m_resource->GetName()));

				if (stream.GetRef())
				{
					size_t numEntries = stream->GetLength() / sizeof(Entry);

					std::vector<Entry> entries(numEntries);
					stream->Read(&entries[0], entries.size() * sizeof(Entry));

					for (auto& entry : entries)
					{
						auto file = AddStreamingFile(entry);

						if (file)
						{
							file->isAutoScan = true;
						}
					}
				}
			}
		}, 500);

		object->OnStop.Connect([=]()
		{
			m_resourcePairs.clear();
		}, -500);
	}
}

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new fx::ResourceStreamComponent());
	});
});
