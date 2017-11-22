/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "ResourceMetaDataComponent.h"

#include <ManifestVersion.h>

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

static size_t FindManifestVersionIndex(const guid_t& guid)
{
	auto begin = g_manifestVersionOrder;
	auto end = g_manifestVersionOrder + _countof(g_manifestVersionOrder);
	auto found = std::find(begin, end, guid);

	if (found == end)
	{
		return -1;
	}

	return (found - g_manifestVersionOrder);
}

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
	auto resourceVersion = FindManifestVersionIndex(manifestVersion);

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
		auto lowerVersion = FindManifestVersionIndex(lowerBound);

		if (resourceVersion < lowerVersion)
		{
			matches = false;
		}
	}

	if (matches && upperBound != nullGuid)
	{
		auto upperVersion = FindManifestVersionIndex(upperBound);

		if (resourceVersion >= upperVersion)
		{
			matches = false;
		}
	}

	return matches;
}
}
