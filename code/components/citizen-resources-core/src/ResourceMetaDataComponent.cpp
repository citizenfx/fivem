/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceMetaDataComponent.h"

#include <Resource.h>
#include <VFSManager.h>

#include <ManifestVersion.h>

#include <regex>
#include <boost/algorithm/string/replace.hpp>

std::string path_normalize(const std::string& pathRef);

namespace fx
{
ResourceMetaDataComponent::ResourceMetaDataComponent(Resource* resourceRef)
	: m_resource(resourceRef), m_metaDataLoader(nullptr)
{

}

boost::optional<std::string> ResourceMetaDataComponent::LoadMetaData(const std::string& resourcePath)
{
	assert(m_metaDataLoader.GetRef());

	m_metaDataEntries.clear();

	return m_metaDataLoader->LoadMetaData(this, resourcePath);
}

static constexpr const ManifestVersion g_manifestVersionOrder[] = {
	guid_t{ 0 },
#include <ManifestVersions.h>
};

static constexpr const std::string_view g_manifestVersionOrderV2[] = {
	"",
#include <ManifestVersionsV2.h>
};

template<typename TSearch, typename T, unsigned int N>
static size_t FindManifestVersionIndex(const T(&list)[N], const TSearch& guid)
{
	auto begin = list;
	auto end = list + std::size(list);
	auto found = std::find(begin, end, guid);

	if (found == end)
	{
		return -1;
	}

	return (found - list);
}

// TODO: clean up to be a templated func
std::optional<bool> ResourceMetaDataComponent::IsManifestVersionBetween(const guid_t& lowerBound, const guid_t& upperBound)
{
	auto entries = this->GetEntries("resource_manifest_version");

	guid_t manifestVersion = { 0 };

	// if there's a manifest version
	if (entries.begin() != entries.end())
	{
		// parse it
		manifestVersion = ParseGuid(entries.begin()->second);
	}

	// find the manifest version in the manifest version stack
	auto resourceVersion = FindManifestVersionIndex(g_manifestVersionOrder, manifestVersion);

	// if not found, return failure
	if (resourceVersion == -1)
	{
		return {};
	}

	// test lower/upper bound
	static const guid_t nullGuid = { 0 };
	bool matches = true;

	if (lowerBound != nullGuid)
	{
		auto lowerVersion = FindManifestVersionIndex(g_manifestVersionOrder, lowerBound);

		if (resourceVersion < lowerVersion)
		{
			matches = false;
		}
	}

	if (matches && upperBound != nullGuid)
	{
		auto upperVersion = FindManifestVersionIndex(g_manifestVersionOrder, upperBound);

		if (resourceVersion >= upperVersion)
		{
			matches = false;
		}
	}

	return matches;
}

std::optional<bool> ResourceMetaDataComponent::IsManifestVersionBetween(const std::string& lowerBound, const std::string& upperBound)
{
	auto entries = this->GetEntries("fx_version");

	std::string manifestVersion;

	// if there's a manifest version
	if (entries.begin() != entries.end())
	{
		// parse it
		manifestVersion = entries.begin()->second;
	}

	// find the manifest version in the manifest version stack
	auto resourceVersion = FindManifestVersionIndex(g_manifestVersionOrderV2, manifestVersion);

	// if not found, return failure
	if (resourceVersion == -1)
	{
		return {};
	}

	// test lower/upper bound
	static const std::string emptyString = "";
	bool matches = true;

	if (lowerBound != emptyString)
	{
		auto lowerVersion = FindManifestVersionIndex(g_manifestVersionOrderV2, lowerBound);

		if (resourceVersion < lowerVersion)
		{
			matches = false;
		}
	}

	if (matches && upperBound != emptyString)
	{
		auto upperVersion = FindManifestVersionIndex(g_manifestVersionOrderV2, upperBound);

		if (resourceVersion >= upperVersion)
		{
			matches = false;
		}
	}

	return matches;
}

static std::string getDirectory(const std::string& in)
{
	auto i = in.find_last_of('/');

	if (i != std::string::npos)
	{
		return in.substr(0, i);
	}
	else
	{
		return ".";
	}
}

struct Match
{
	Match(const fwRefContainer<vfs::Device>& device, const std::string& pattern)
	{
		auto slashPos = pattern.find_last_of('/');
		auto root = pattern.substr(0, slashPos) + "/";
		auto after = pattern.substr(slashPos + 1);

		this->findHandle = device->FindFirst(root, &findData);

		this->device = device;
		this->pattern = after;
		this->root = root;
		this->end = (this->findHandle == INVALID_DEVICE_HANDLE);

		auto patternCopy = after;

		boost::replace_all(patternCopy, "\\", "\\\\");
		boost::replace_all(patternCopy, "^", "\\^");
		boost::replace_all(patternCopy, ".", "\\.");
		boost::replace_all(patternCopy, "$", "\\$");
		boost::replace_all(patternCopy, "|", "\\|");
		boost::replace_all(patternCopy, "(", "\\(");
		boost::replace_all(patternCopy, ")", "\\)");
		boost::replace_all(patternCopy, "[", "\\[");
		boost::replace_all(patternCopy, "]", "\\]");
		boost::replace_all(patternCopy, "*", "\\*");
		boost::replace_all(patternCopy, "+", "\\+");
		boost::replace_all(patternCopy, "?", "\\?");
		boost::replace_all(patternCopy, "/", "\\/");
		boost::replace_all(patternCopy, "\\?", ".");
		boost::replace_all(patternCopy, "\\*", ".*");

		this->re = std::regex{ "^" + patternCopy + "$" };

		while (!Matches() && !end)
		{
			FindNext();
		}

		this->has = false;

		if (Matches())
		{
			this->has = true;
		}
	}

	const vfs::FindData& Get()
	{
		return findData;
	}

	bool Matches()
	{
		if (findData.name != "." && findData.name != "..")
		{
			return std::regex_match(findData.name, re);
		}

		return false;
	}

	void Next()
	{
		if (!end)
		{
			do
			{
				FindNext();
			} while (!Matches() && !end);

			has = !end && Matches();
		}
		else
		{
			has = false;
		}
	}

	operator bool()
	{
		return findHandle != INVALID_DEVICE_HANDLE && has;
	}

	~Match()
	{
		if (findHandle != INVALID_DEVICE_HANDLE)
		{
			device->FindClose(findHandle);
		}

		findHandle = INVALID_DEVICE_HANDLE;
	}

private:
	void FindNext()
	{
		end = !device->FindNext(findHandle, &findData);
	}

private:
	fwRefContainer<vfs::Device> device;
	std::string root;
	std::string pattern;
	vfs::Device::THandle findHandle;
	vfs::FindData findData;
	std::regex re;
	bool end;
	bool has;
};

static std::vector<std::string> MatchFiles(const fwRefContainer<vfs::Device>& device, const std::string& pattern)
{
	auto patternNorm = path_normalize(pattern);

	auto starPos = patternNorm.find('*');
	auto before = getDirectory((starPos != std::string::npos) ? patternNorm.substr(0, starPos) : patternNorm);
	auto slashPos = (starPos != std::string::npos) ? patternNorm.find('/', starPos) : std::string::npos;
	auto after = (slashPos != std::string::npos) ? patternNorm.substr(slashPos + 1) : "";

	bool recurse = (starPos != std::string::npos &&
		patternNorm.substr(starPos + 1, 1) == "*" &&
		(starPos == 0 || patternNorm.substr(starPos - 1, 1) == "/"));

	std::vector<std::string> results;

	if (recurse)
	{
		auto submask = patternNorm.substr(0, starPos) + patternNorm.substr(starPos + 2);
		results = MatchFiles(device, submask);

		auto findPattern = patternNorm.substr(0, starPos + 1);

		for (Match match{ device, findPattern }; match; match.Next())
		{
			if (match.Get().attributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				auto matchPath = before + "/" + match.Get().name + "/" + patternNorm.substr(starPos);
				auto resultsSecond = MatchFiles(device, matchPath);

				for (auto& result : resultsSecond)
				{
					results.push_back(std::move(result));
				}
			}
		}
	}
	else
	{
		auto findPattern = patternNorm.substr(0, slashPos != std::string::npos ? slashPos - 1 : std::string::npos);

		for (Match match{ device, findPattern }; match; match.Next())
		{
			bool isfile = !(match.Get().attributes & FILE_ATTRIBUTE_DIRECTORY);
			bool hasSlashPos = slashPos != std::string::npos;

			if (!(hasSlashPos && isfile))
			{
				auto matchPath = before + "/" + match.Get().name;

				if (!after.empty())
				{
					auto resultsSecond = MatchFiles(device, matchPath + "/" + after);

					for (auto& result : resultsSecond)
					{
						results.push_back(std::move(result));
					}
				}
				else
				{
					results.push_back(matchPath);
				}
			}
		}
	}

	return results;
}

void ResourceMetaDataComponent::GlobEntries(const std::string& key, const std::function<void(const std::string&)>& entryCallback)
{
	for (auto& entry : GetEntries(key))
	{
		GlobValue(entry.second, entryCallback);
	}
}

void ResourceMetaDataComponent::GlobValue(const std::string& value, const std::function<void(const std::string&)>& entryCallback)
{
	// why... would anyone pass an empty value?!
	// this makes the VFS all odd so let's ignore it
	if (value.empty())
	{
		return;
	}

	const auto& rootPath = m_resource->GetPath() + "/";
	fwRefContainer<vfs::Device> device = vfs::GetDevice(rootPath);

	if (!device.GetRef())
	{
		return;
	}

	auto relRoot = path_normalize(rootPath);

	std::string pattern = value;

	// @ prefixes for files are special and handled later on
	if (pattern.length() >= 1 && pattern[0] == '@')
	{
		entryCallback(pattern);
		return;
	}

	auto mf = MatchFiles(device, rootPath + pattern);

	for (auto& file : mf)
	{
		if (file.length() < (relRoot.length() + 1))
		{
			continue;
		}

		entryCallback(file.substr(relRoot.length() + 1));
	}
}
}
