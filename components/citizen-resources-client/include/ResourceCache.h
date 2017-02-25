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

	boost::optional<Entry> GetEntryFor(const std::string& hashString);

	boost::optional<Entry> GetEntryFor(const std::array<uint8_t, 20>& hash);

	inline const std::string& GetCachePath()
	{
		return m_cachePath;
	}

private:
	void OpenDatabase();
};