/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ResourceCache.h>

#include <SHA1.h>
#include <VFSManager.h>

#include <msgpack.hpp>

#include <Error.h>

ResourceCache::ResourceCache(const std::string& cachePath, const std::string& physCachePath)
	: m_cachePath(cachePath), m_physCachePath(physCachePath)
{
	OpenDatabase();

	vfs::GetDevice(cachePath)->CreateDirectory(cachePath + "/unconfirmed/");
}

leveldb::Env* GetVFSEnvironment();

void ResourceCache::OpenDatabase()
{
	leveldb::DB* dbPointer;

	leveldb::Options options;
	options.env = GetVFSEnvironment();
	options.create_if_missing = true;

	auto status = leveldb::DB::Open(options, m_cachePath + "/db/", &dbPointer);

	if (status.IsCorruption() || status.IsIOError())
	{
		leveldb::Options repairOptions;
		repairOptions.reuse_logs = false;
		repairOptions.create_if_missing = true;
		repairOptions.env = GetVFSEnvironment();

		status = leveldb::RepairDB(m_cachePath + "/db/", repairOptions);

		if (status.ok())
		{
			status = leveldb::DB::Open(options, m_cachePath + "/db/", &dbPointer);
		}

		if (!status.ok())
		{
			status = leveldb::DestroyDB(m_cachePath + "/db/", options);

			status = leveldb::DB::Open(options, m_cachePath + "/db/", &dbPointer);
		}
	}

	if (!status.ok())
	{
		FatalError("Opening database (%s) failed: %s", m_cachePath, status.ToString());
	}

	m_indexDatabase = std::unique_ptr<leveldb::DB>(dbPointer);
	dbPointer = nullptr; // as the unique_ptr 'owns' it now
}

// TODO: should be shared
template<int Size>
static std::array<uint8_t, Size> ParseHexString(const char* string)
{
	std::array<uint8_t, Size> retval;

	assert(strlen(string) == Size * 2);

	for (int i = 0; i < Size; i++)
	{
		const char* startIndex = &string[i * 2];
		char byte[3] = { startIndex[0], startIndex[1], 0 };

		retval[i] = strtoul(byte, nullptr, 16);
	}

	return retval;
}

ResourceCache::Entry::Entry(const std::string& entryData)
{
	// deserialize the entry data
	msgpack::unpacked msg = msgpack::unpack(entryData.c_str(), entryData.size());
	const msgpack::object& object = msg.get();

	// convert to a map of msgpack objects
	std::map<std::string, msgpack::object> data;
	object.convert(data);

	// fill out the main fields
	m_localPath = data["fn"].as<std::string>();
	m_hash = ParseHexString<20>(data["h"].as<std::string>().c_str());
	m_metaData = data["m"].as<std::map<std::string, std::string>>();
}

std::string ResourceCache::Entry::GetHashString() const
{
	const auto& hash = m_hash;

	return va("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
			  hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
			  hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]);
}

void ResourceCache::AddEntry(const std::string& localFileName, const std::map<std::string, std::string>& metaData)
{
	// attempt to open the local file (for hashing, mainly)
	fwRefContainer<vfs::Stream> stream = vfs::OpenRead(localFileName);

	if (stream.GetRef())
	{
		// calculate a hash of the file
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

		std::array<uint8_t, 20> h;
		memcpy(h.data(), hash, 20);

		stream = {};

		return AddEntry(localFileName, h, metaData);
	}
}

void ResourceCache::AddEntry(const std::string& localFileName, const std::array<uint8_t, 20>& hash, const std::map<std::string, std::string>& metaData)
{
	std::string hashString = va("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		hash[0], hash[1], hash[2], hash[3], hash[4], hash[5], hash[6], hash[7], hash[8], hash[9],
		hash[10], hash[11], hash[12], hash[13], hash[14], hash[15], hash[16], hash[17], hash[18], hash[19]);

	{
		auto refIt = metaData.find("reference");

		if (refIt != metaData.end())
		{
			if (refIt->second != hashString && !refIt->second.empty())
			{
				trace(__FUNCTION__ ": corruption uncovered - %s should hash to %s, but saved as %s.\n", localFileName, refIt->second, hashString);
			}
		}
	}

	std::string targetFileName = m_cachePath + "cache_" + hashString;

	fwRefContainer<vfs::Device> device = vfs::GetDevice(localFileName);
	device->RemoveFile(targetFileName);
	device->RenameFile(localFileName, targetFileName);

	// serialize the data for placement in the database
	msgpack::sbuffer buffer;
	msgpack::packer<msgpack::sbuffer> packer(buffer);

	// we want to pack a map with 3 entries - filename, hash and metadata
	packer.pack_map(3);

	packer.pack("fn");
	packer.pack(targetFileName);

	packer.pack("h");
	packer.pack(hashString);

	packer.pack("m");
	packer.pack(metaData);


	leveldb::WriteOptions options;
	options.sync = true;

	{
		// add an entry to the database
		std::string key = "cache:v1:" + std::string(reinterpret_cast<const char*>(hash.data()), 20);

		m_indexDatabase->Put(options, key, leveldb::Slice(buffer.data(), buffer.size()));
		m_entryCache[key] = {};
	}

	{
		// add a URL entry as well
		auto fromIt = metaData.find("from");

		std::string key = "cache:v1:url:" + fromIt->second;

		m_indexDatabase->Put(options, key, leveldb::Slice(buffer.data(), buffer.size()));
		m_entryCache[key] = {};
	}

	trace("ResourceCache::AddEntry: Saved cache:v1:%s to the index cache.\n", hashString);
}

std::optional<ResourceCache::Entry> ResourceCache::GetEntryFor(const std::array<uint8_t, 20>& hash)
{
	// attempt a database get
	std::string key = "cache:v1:" + std::string(reinterpret_cast<const char*>(&hash[0]), hash.size());
	std::string value;

	auto fetchIt = m_entryCache.find(key);

	if (fetchIt != m_entryCache.end() && fetchIt->second)
	{
		return *fetchIt->second;
	}

	leveldb::Status status = m_indexDatabase->Get(leveldb::ReadOptions{}, key, &value);

	if (status.ok())
	{
		Entry e{ value };
		m_entryCache[key] = std::optional<Entry>(e);

		return std::optional<Entry>(e);
	}

	if (!status.IsNotFound())
	{
#if _DEBUG
		FatalError("Failed to fetch ResourceCache entry: %s", status.ToString());
#else
		trace("Failed to fetch ResourceCache entry: %s\n", status.ToString());
#endif
	}

	m_entryCache[key] = { std::optional<Entry>{} };
	return std::optional<Entry>();
}

std::optional<ResourceCache::Entry> ResourceCache::GetEntryFor(const std::string& hashString)
{
	return GetEntryFor(ParseHexString<20>(hashString.c_str()));
}

std::optional<ResourceCache::Entry> ResourceCache::GetEntryFor(const ResourceCacheEntryList::Entry& entry)
{
	if (entry.referenceHash.empty())
	{
		// attempt a database get
		std::string key = "cache:v1:url:" + entry.remoteUrl;
		std::string value;

		leveldb::Status status = m_indexDatabase->Get(leveldb::ReadOptions{}, key, &value);

		if (status.ok())
		{
			return std::optional<Entry>(Entry(value));
		}

		if (!status.IsNotFound())
		{
#if _DEBUG
			FatalError("Failed to fetch ResourceCache entry: %s", status.ToString());
#else
			trace("Failed to fetch ResourceCache entry: %s\n", status.ToString());
#endif
		}

		return std::optional<Entry>();
	}

	return GetEntryFor(entry.referenceHash);
}
