#include "StdInc.h"
#include <ResourceStreamComponent.h>

#include <VFSManager.h>

#include <SHA1.h>

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
		auto fileName = fmt::sprintf("cache:/files/%s.sfl", m_resource->GetName());
		auto device = vfs::GetDevice(fileName);

		if (!device.GetRef())
		{
			return true;
		}

		// check if the modification time differs
		time_t listTime = device->GetModifiedTime(fileName);
		time_t streamTime = 0;

		IterateRecursively(fmt::sprintf("%s/stream/", m_resource->GetPath()), [&](const std::string& fullPath)
		{
			auto device = vfs::GetDevice(fullPath);

			if (device.GetRef())
			{
				auto time = device->GetModifiedTime(fullPath);

				if (time != -1 && time >= streamTime)
				{
					streamTime = time;
				}
			}
		});

		if (streamTime > listTime)
		{
			return true;
		}

		// load the list and verify if all files still exist
		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(fmt::sprintf("cache:/files/%s.sfl", m_resource->GetName()));

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
		std::vector<Entry> entries;

		IterateRecursively(fmt::sprintf("%s/stream/", m_resource->GetPath()), [&](const std::string& fullPath)
		{
			Entry entry = { 0 };

			auto stream = vfs::OpenRead(fullPath);

			if (!stream.GetRef())
			{
				return;
			}

			entry.size = stream->GetLength();

			strncpy(entry.entryName, fullPath.substr(fullPath.find_last_of("/\\") + 1).c_str(), sizeof(entry.entryName));
			strncpy(entry.onDiskPath, fullPath.c_str(), sizeof(entry.onDiskPath));

			entry.rscFlags = entry.size;

			{
				struct  
				{
					uint32_t magic;
					uint32_t version;
					uint32_t virtPages;
					uint32_t physPages;
				} rsc7Header;

				stream->Read(&rsc7Header, sizeof(rsc7Header));

				if (rsc7Header.magic == 0x37435352) // RSC7
				{
					entry.rscVersion = rsc7Header.version;
					entry.rscPagesPhysical = rsc7Header.physPages;
					entry.rscPagesVirtual = rsc7Header.virtPages;
					entry.isResource = true;
				}

				stream->Seek(0, SEEK_SET);
			}

			{
				// calculate the file hash
				std::vector<uint8_t> data(8192);
				sha1nfo sha1;
				size_t numRead;

				// initialize context
				sha1_init(&sha1);

				// read from the stream
				while ((numRead = stream->Read(data)) > 0)
				{
					sha1_write(&sha1, reinterpret_cast<char*>(&data[0]), numRead);
				}

				// get the hash result and convert it to a string
				uint8_t* hash = sha1_result(&sha1);

				strcpy(entry.hashString, fmt::sprintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
					hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
					hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]).c_str());
			}

			entries.emplace_back(entry);
		});

		std::string outFileName = fmt::sprintf("cache:/files/%s.sfl", m_resource->GetName());

		fwRefContainer<vfs::Device> device = vfs::GetDevice(outFileName);
		device->CreateDirectory(outFileName.substr(0, outFileName.find_last_of('/')));

		auto handle = device->Create(outFileName);

		if (handle == INVALID_DEVICE_HANDLE)
		{
			return false;
		}

		fwRefContainer<vfs::Stream> stream(new vfs::Stream(device, handle));

		stream->Write(&entries[0], entries.size() * sizeof(Entry));

		return true;
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

			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(fmt::sprintf("cache:/files/%s.sfl", m_resource->GetName()));

			if (stream.GetRef())
			{
				size_t numEntries = stream->GetLength() / sizeof(Entry);

				std::vector<Entry> entries(numEntries);
				stream->Read(&entries[0], entries.size() * sizeof(Entry));

				for (auto& entry : entries)
				{
					m_resourcePairs.insert({ entry.entryName, entry });
				}
			}
		}, 500);
	}
}

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new fx::ResourceStreamComponent());
	});
});
