#include "StdInc.h"
#include <ResourceStreamComponent.h>

#include <VFSManager.h>
#include <VFSLinkExtension.h>

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

		if (numEntries > 0)
		{
			std::vector<Entry> entries(numEntries);
			stream->Read(&entries[0], entries.size() * sizeof(Entry));

			for (auto& entry : entries)
			{
				if (!vfs::OpenRead(entry.onDiskPath).GetRef())
				{
					return true;
				}
			}
		}

		return false;
	}

	static void MakeSymLink(const std::string& cacheRoot, ResourceStreamComponent::RuntimeEntry* file)
	{
		auto device = vfs::GetDevice(cacheRoot);
		device->CreateDirectory(cacheRoot);

		auto h = device->Create(cacheRoot + "these files are hardlinks - they do not take up any disk space by themselves.txt");

		if (h != vfs::Device::InvalidHandle)
		{
			device->Close(h);
		}

		vfs::MakeHardLinkExtension cd;
		cd.existingPath = file->onDiskPath;
		cd.newPath = cacheRoot + "z" + file->hashString;

		bool madeLink = device->ExtensionCtl(VFS_MAKE_HARDLINK, &cd, sizeof(cd));

		if (madeLink)
		{
			file->loadDiskPath = cd.newPath;
		}
		else
		{
			static bool warned = false;

			if (!warned)
			{
				trace("^3Could not make hard link for %s <-> %s.^7\n", cd.existingPath, cd.newPath);

				warned = true;
			}
		}
	}

	bool ResourceStreamComponent::UpdateSet()
	{
		// set up variables
		std::string outFileName = fmt::sprintf("cache:/files/%s/resource.sfl", m_resource->GetName());
		auto sflRoot = outFileName.substr(0, outFileName.find_last_of('/'));
		auto cacheRoot = sflRoot + "/stream_cache/";

		// create output directory
		fwRefContainer<vfs::Device> device = vfs::GetDevice(outFileName);
		device->CreateDirectory(sflRoot);

		// look at files
		std::vector<std::string> files;

		IterateRecursively(fmt::sprintf("%s/stream/", m_resource->GetPath()), [this, device, &cacheRoot, &files](const std::string& fullPath)
		{
			files.push_back(fullPath);

			auto file = AddStreamingFile(fullPath);

			if (file)
			{
				file->isAutoScan = true;

				MakeSymLink(cacheRoot, file);
			}
		});

		// remove hard links for missing files
		{
			vfs::FindData fd;
			auto handle = device->FindFirst(cacheRoot, &fd);

			if (handle != INVALID_DEVICE_HANDLE)
			{
				do 
				{
					if (!(fd.attributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (fd.name.length() == 41 && fd.name[0] == 'z')
						{
							auto hash = fd.name.substr(1);

							if (m_hashPairs.find(hash) == m_hashPairs.end())
							{
								device->RemoveFile(cacheRoot + fd.name);
							}
						}
					}
				} while (device->FindNext(handle, &fd));

				device->FindClose(handle);
			}
		}

		// save the resource database
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

	uint32_t ConvertRSC7Size(uint32_t flags);

	auto ResourceStreamComponent::AddStreamingFile(const std::string& fullPath) -> RuntimeEntry*
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

				ValidateSize(entry.entryName, ConvertRSC7Size(rsc7Header.physPages), ConvertRSC7Size(rsc7Header.virtPages));
			}
			else if (rsc7Header.magic == 0x05435352) // RSC\x05
			{
				entry.rscVersion = rsc7Header.version;
				entry.rscPagesVirtual = rsc7Header.virtPages;
				entry.rscPagesPhysical = rsc7Header.version;
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
			{ "isResource", this->isResource },
			{ "rscPagesPhysical", this->rscPagesPhysical},
			{ "rscPagesVirtual", this->rscPagesVirtual},
			{ "rscVersion", this->rscVersion},
			{ "size", this->size}
		});

		return json.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
	}

	auto ResourceStreamComponent::AddStreamingFile(const std::string& entryName, const std::string& diskPath, const std::string& cacheString) -> RuntimeEntry*
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

	uint32_t ConvertRSC7Size(uint32_t flags)
	{
		auto s0 = ((flags >> 27) & 0x1) << 0; // 1 bit  - 27        (*1)
		auto s1 = ((flags >> 26) & 0x1) << 1; // 1 bit  - 26        (*2)
		auto s2 = ((flags >> 25) & 0x1) << 2; // 1 bit  - 25        (*4)
		auto s3 = ((flags >> 24) & 0x1) << 3; // 1 bit  - 24        (*8)
		auto s4 = ((flags >> 17) & 0x7F) << 4; // 7 bits - 17 - 23   (*16)   (max 127 * 16)
		auto s5 = ((flags >> 11) & 0x3F) << 5; // 6 bits - 11 - 16   (*32)   (max 63  * 32)
		auto s6 = ((flags >> 7) & 0xF) << 6; // 4 bits - 7  - 10   (*64)   (max 15  * 64)
		auto s7 = ((flags >> 5) & 0x3) << 7; // 2 bits - 5  - 6    (*128)  (max 3   * 128)
		auto s8 = ((flags >> 4) & 0x1) << 8; // 1 bit  - 4         (*256)
		auto ss = ((flags >> 0) & 0xF); // 4 bits - 0  - 3
		auto baseSize = 0x200 << (uint32_t)ss;
		auto size = baseSize * (s0 + s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8);
		return (uint32_t)size;
	}

	void ResourceStreamComponent::ValidateSize(std::string_view name, uint32_t physSize, uint32_t virtSize)
	{
		auto checkSize = [&name, this](uint32_t size, std::string_view why)
		{
			auto divSize = fmt::sprintf("%.1f", size / 1024.0 / 1024.0);
			int warnColor = 0;

			if (size > (64 * 1024 * 1024))
			{
				warnColor = 1;
			}
			else if (size > (32 * 1024 * 1024))
			{
				warnColor = 3;
			}
			else if (size > (16 * 1024 * 1024))
			{
				warnColor = 4;
			}

			if (warnColor != 0)
			{
				trace("^%dAsset %s/%s uses %s MiB of %s memory.%s^7\n", warnColor, m_resource->GetName(), name, divSize, why,
					(size > (48 * 1024 * 1024))
						? " Oversized assets can and WILL lead to streaming issues (models not loading/rendering, commonly known as 'texture loss', 'city bug' or 'streaming isn't great')."
						: "");
			}
		};

		checkSize(physSize, "physical");
		checkSize(virtSize, "virtual");
	}

	auto ResourceStreamComponent::AddStreamingFile(const Entry& entry) -> RuntimeEntry*
	{
		RuntimeEntry se(entry);
		se.isAutoScan = false;

		if (entry.isResource /* && gamename == gta5? */)
		{
			ValidateSize(entry.entryName, ConvertRSC7Size(entry.rscPagesPhysical), ConvertRSC7Size(entry.rscPagesVirtual));
		}

		auto it = m_resourcePairs.insert({ entry.entryName, se }).first;
		m_hashPairs[se.hashString] = &it->second;

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
				auto cacheRoot = fmt::sprintf("cache:/files/%s/stream_cache/", m_resource->GetName());

				if (stream.GetRef())
				{
					size_t numEntries = stream->GetLength() / sizeof(Entry);

					if (numEntries)
					{
						std::vector<Entry> entries(numEntries);
						stream->Read(&entries[0], entries.size() * sizeof(Entry));

						for (auto& entry : entries)
						{
							auto file = AddStreamingFile(entry);

							if (file)
							{
								file->isAutoScan = true;

								MakeSymLink(cacheRoot, file);
							}
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
