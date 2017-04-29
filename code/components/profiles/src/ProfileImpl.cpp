/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ProfileManagerImpl.h"

ProfileImpl::ProfileImpl()
	: m_isSuggestion(false), m_internalIdentifier(0), m_signedIn(false)
{

}

bool ProfileImpl::IsSignedIn()
{
	return m_signedIn;
}

concurrency::task<ProfileTaskResult> ProfileImpl::MergeProfile(fwRefContainer<Profile> fromProfile)
{
	ProfileTaskResult result;

	return concurrency::task_from_result<ProfileTaskResult>(result);
}

const char* ProfileImpl::GetDisplayName()
{
	return m_name.c_str();
}

void ProfileImpl::SetDisplayName(const std::string& name)
{
	m_name = name;
}

const char* ProfileImpl::GetTileURI()
{
	return m_uri.c_str();
}

void ProfileImpl::SetTileURI(const std::string& uri)
{
	m_uri = uri;
}

uint32_t ProfileImpl::GetInternalIdentifier()
{
	return m_internalIdentifier;
}

int ProfileImpl::GetNumIdentifiers()
{
	return m_identifiers.size();
}

ProfileIdentifier ProfileImpl::GetIdentifierInternal(int index)
{
	return m_identifiers[index];
}

const char* ProfileImpl::GetIdentifier(int index)
{
	auto identifier = GetIdentifierInternal(index);

	return va("%s:%s", identifier.first.c_str(), identifier.second.c_str());
}

void ProfileImpl::SetIdentifiers(const std::vector<ProfileIdentifier>& identifiers)
{
	m_identifiers = identifiers;
}

const std::map<std::string, std::string>& ProfileImpl::GetParameters()
{
	return m_parameters;
}

void ProfileImpl::SetParameters(const std::map<std::string, std::string>& parameters)
{
	m_parameters = parameters;
}