#include <StdInc.h>
#include <ResourceFileDatabase.h>

#include <VFSManager.h>

#include <botan/base64.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

namespace fx
{
ResourceFileDatabase::ResourceFileDatabase()
{

}

bool ResourceFileDatabase::Load(const std::string& fileName)
{
	auto vfsStream = vfs::OpenRead(fileName);

	m_entries.clear();
	
	if (vfsStream.GetRef())
	{
		auto data = vfsStream->ReadToEnd();

		// add a null terminator
		data.resize(data.size() + 1);

		rapidjson::Document doc;
		doc.ParseInsitu(reinterpret_cast<char*>(data.data()));

		if (!doc.HasParseError() && doc.IsArray())
		{
			for (auto it = doc.Begin(); it != doc.End(); it++)
			{
				auto& elem = *it;

				if (elem.IsObject())
				{
					// assume the format here is going to be 'sane'
					std::string name = elem["n"].GetString();
					std::time_t mtime = elem["mt"].GetInt64();
					uint64_t size = elem["s"].GetUint64();
					auto id = Botan::base64_decode(elem["i"].GetString());

					Entry entry;
					memcpy(entry.fileId.data(), id.data(), std::min(id.size(), entry.fileId.size()));
					entry.mtime = mtime;
					entry.size = size;

					m_entries.emplace(name, std::move(entry));
				}
			}

			return true;
		}
	}

	return false;
}

bool ResourceFileDatabase::Save(const std::string& fileName)
{
	rapidjson::Document doc;
	doc.SetArray();

	for (const auto& entryPair : m_entries)
	{
		rapidjson::Value entryVal;
		entryVal.SetObject();

		entryVal.AddMember("n", rapidjson::Value(entryPair.first.c_str(), entryPair.first.size()), doc.GetAllocator());
		entryVal.AddMember("mt", rapidjson::Value(int64_t(entryPair.second.mtime)), doc.GetAllocator());
		entryVal.AddMember("s", rapidjson::Value(entryPair.second.size), doc.GetAllocator());

		auto id = Botan::base64_encode(entryPair.second.fileId.data(), entryPair.second.fileId.size());
		entryVal.AddMember("i", rapidjson::Value(id.c_str(), doc.GetAllocator()), doc.GetAllocator());

		doc.PushBack(entryVal, doc.GetAllocator());
	}

	rapidjson::StringBuffer sb;
	rapidjson::Writer writer(sb);
	
	doc.Accept(writer);

	{
		auto device = vfs::GetDevice(fileName);

		if (device.GetRef())
		{
			// as PackfileBuilder does this too and we run before it
			device->CreateDirectory(fileName.substr(0, fileName.find_last_of('/')));

			auto handle = device->Create(fileName);

			if (handle != INVALID_DEVICE_HANDLE)
			{
				device->Write(handle, sb.GetString(), sb.GetSize());
				device->Close(handle);

				return true;
			}
		}
	}

	return false;
}

bool ResourceFileDatabase::Check(const std::vector<std::string>& fileList)
{
	// first of all, check if the file names in the list are any different from what's stored in the database
	std::set<std::string> newSet(fileList.begin(), fileList.end());
	std::set<std::string> oldSet;

	for (auto& entry : m_entries)
	{
		oldSet.insert(entry.first);
	}

	// count different? good, everything's different
	if (oldSet.size() != newSet.size())
	{
		return true;
	}

	// difference
	std::vector<std::string> outs;
	std::set_difference(oldSet.begin(), oldSet.end(), newSet.begin(), newSet.end(), std::back_inserter(outs));

	// contains different names? ok, different
	if (!outs.empty())
	{
		return true;
	}

	// loop through every file in our data set to check if on-disk values are different
	for (auto& entry : m_entries)
	{
		auto newEntry = GetEntry(entry.first);

		// is different?
		if (newEntry != entry.second)
		{
			return true;
		}
	}

	// guess it all matches
	return false;
}

void ResourceFileDatabase::Snapshot(const std::vector<std::string>& fileList)
{
	m_entries.clear();

	for (const auto& file : fileList)
	{
		m_entries.emplace(file, GetEntry(file));
	}
}

auto ResourceFileDatabase::GetEntry(const std::string& file) -> Entry
{
	Entry entry;

	auto device = vfs::GetDevice(file);

	if (device.GetRef())
	{
		auto handle = device->Open(file, true);

		if (handle != INVALID_DEVICE_HANDLE)
		{
			entry.mtime = device->GetModifiedTime(file);
			entry.size = device->GetLength(handle);
			
			vfs::GetFileIdExtension idExt;
			idExt.handle = handle;

			device->ExtensionCtl(VFS_GET_FILE_ID, &idExt, sizeof(idExt));

			entry.fileId = idExt.fileId;

			// very important, close the file
			device->Close(handle);
		}
	}

	return entry;
}
}
