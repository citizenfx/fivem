/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <om/core.h>

#include <optional>
#include "IteratorView.h"

#ifdef __linux__
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
class Resource;

class ResourceMetaDataComponent;

class ResourceMetaDataLoader : public fwRefCountable
{
public:
	virtual std::optional<std::string> LoadMetaData(ResourceMetaDataComponent* component, const std::string& resourcePath) = 0;
};

class RESOURCES_CORE_EXPORT ResourceMetaDataComponent : public fwRefCountable
{
public:
	struct Location
	{
		std::string file;
		int line = 0;

		// dummy ctor is required for building with Clang
		// ("default member initializer for 'line' needed within definition of enclosing class 
		//   'ResourceMetaDataComponent' outside of member functions")
		inline Location()
		{
		}
	};

private:
	Resource* m_resource;

	std::multimap<std::string, std::string> m_metaDataEntries;
	std::multimap<std::string, Location> m_metaDataLocations;

	fwRefContainer<ResourceMetaDataLoader> m_metaDataLoader;

public:
	ResourceMetaDataComponent(Resource* resourceRef);

	std::optional<std::string> LoadMetaData(const std::string& resourcePath);

	std::optional<bool> IsManifestVersionBetween(const guid_t& lowerBound, const guid_t& upperBound);

	std::optional<bool> IsManifestVersionBetween(const std::string& lowerBound, const std::string& upperBound);

	inline Resource* GetResource()
	{
		return m_resource;
	}

	inline void SetMetaDataLoader(fwRefContainer<ResourceMetaDataLoader> loader)
	{
		m_metaDataLoader = loader;
	}

	inline std::map<std::string, std::vector<std::string>> GetAllEntries()
	{
		std::map<std::string, std::vector<std::string>> newMap;

		for (auto it = m_metaDataEntries.begin(), end = m_metaDataEntries.end(); it != end; it++)
		{
			newMap[it->first].push_back(it->second);
		}

		return newMap;
	}

	inline auto GetEntries(const std::string& key)
	{
		return GetIteratorView(m_metaDataEntries.equal_range(key));
	}

	void GlobEntries(const std::string& key, const std::function<void(const std::string&)>& entryCallback);

	// MissingEntry contains data for a missing entry from GlobMissingEntries
	struct MissingEntry
	{
		std::string value;

		// wasPrefix is true if this was a prefix (i.e. explicitly-specified) part being missing
		bool wasPrefix = false;

		Location source;
	};

	// GlobMissingEntries behaves like GlobEntries, but it will return diagnostics for *missing* entries
	void GlobMissingEntries(const std::string& key, const std::function<void(const MissingEntry&)>& entryCallback);

private:
	template<typename TFn, typename OutputIterator>
	inline void CallIterator(TFn fn, const std::string& key, OutputIterator out)
	{
		(this->*fn)(key, [&out](const auto& entry) mutable
		{
			*out = entry;
			++out;
		});
	}

public:
	template<typename OutputIterator>
	inline void GlobEntriesIterator(const std::string& key, OutputIterator out)
	{
		CallIterator(&ResourceMetaDataComponent::GlobEntries, key, out);
	}

	inline std::vector<std::string> GlobEntriesVector(const std::string& key)
	{
		std::vector<std::string> retval;
		CallIterator(&ResourceMetaDataComponent::GlobEntries, key, std::back_inserter(retval));

		return retval;
	}

	inline std::vector<MissingEntry> GlobMissingEntriesVector(const std::string& key)
	{
		std::vector<MissingEntry> retval;
		CallIterator(&ResourceMetaDataComponent::GlobMissingEntries, key, std::back_inserter(retval));

		return retval;
	}

	void GlobValue(const std::string& value, const std::function<void(const std::string&)>& entryCallback);

private:
	template<typename TFn>
	bool GlobValueInternal(const std::string& value, const TFn& entryCallback);

public:
	template<typename OutputIterator>
	inline void GlobValueIterator(const std::string& value, OutputIterator out)
	{
		GlobValue(value, [&out](const std::string& entry) mutable
		{
			*out = entry;
			++out;
		});
	}

	inline std::vector<std::string> GlobValueVector(const std::string& value)
	{
		std::vector<std::string> retval;
		GlobValueIterator(value, std::back_inserter(retval));

		return retval;
	}

	void AddMetaData(const std::string& key, const std::string& value, const Location& location = {});
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceMetaDataComponent);
