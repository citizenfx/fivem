/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <leveldb/db.h>
#include <array>

#include <boost/optional.hpp>

#include <Resource.h>

struct IgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#endif
ResourceCacheEntryList: public fwRefCountable, public fx::IAttached<fx::Resource>
{
public:
	struct Entry
	{
		std::string resourceName;
		std::string basename;
		std::string remoteUrl;
		std::string referenceHash;
		size_t size;
		std::map<std::string, std::string> extData;

		inline Entry()
		{

		}

		inline Entry(const std::string& resourceName, const std::string& basename, const std::string& remoteUrl, const std::string& referenceHash, size_t size, const std::map<std::string, std::string>& extData = {})
			: resourceName(resourceName), basename(basename), remoteUrl(remoteUrl), referenceHash(referenceHash), size(size), extData(extData)
		{

		}
	};

private:
	fx::Resource* m_parentResource;

	std::map<std::string, Entry, IgnoreCaseLess> m_entries;

public:
	virtual void AttachToObject(fx::Resource* resource) override;

	inline const std::map<std::string, Entry, IgnoreCaseLess>& GetEntries()
	{
		return m_entries;
	}

	inline boost::optional<Entry> GetEntry(const std::string& baseName)
	{
		auto it = m_entries.find(baseName);

		if (it == m_entries.end())
		{
			return boost::optional<Entry>();
		}

		return boost::optional<Entry>(it->second);
	}

	inline void AddEntry(const Entry& entry)
	{
		m_entries[entry.basename] = entry;
		m_entries[entry.basename].resourceName = m_parentResource->GetName();
	}
};

DECLARE_INSTANCE_TYPE(ResourceCacheEntryList);

class
#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	ResourceCache
{
private:
	std::unique_ptr<leveldb::DB> m_indexDatabase;

	std::string m_cachePath;

public:
	class Entry
	{
	private:
		std::map<std::string, std::string> m_metaData;

		std::array<uint8_t, 20> m_hash;

		std::string m_localPath;

	public:
		Entry(const std::string& entryData);

	public:
		inline const std::string& GetLocalPath() const
		{
			return m_localPath;
		}

		inline const std::map<std::string, std::string>& GetMetaData() const
		{
			return m_metaData;
		}

		inline const std::array<uint8_t, 20>& GetHash() const
		{
			return m_hash;
		}

		std::string GetHashString() const;
	};

public:
	ResourceCache(const std::string& cachePath);

	void AddEntry(const std::string& localFileName, const std::map<std::string, std::string>& metaData);

	void AddEntry(const std::string& localFileName, const std::array<uint8_t, 20>& hashData, const std::map<std::string, std::string>& metaData);

	boost::optional<Entry> GetEntryFor(const ResourceCacheEntryList::Entry& entry);

	boost::optional<Entry> GetEntryFor(const std::string& hashString);

	boost::optional<Entry> GetEntryFor(const std::array<uint8_t, 20>& hash);

	inline const std::string& GetCachePath()
	{
		return m_cachePath;
	}

private:
	void OpenDatabase();
};
