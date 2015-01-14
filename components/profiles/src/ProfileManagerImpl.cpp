/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ProfileManagerImpl.h"

#include <dpapi.h>
#include <ShlObj.h>
#include <rapidjson/document.h>

#pragma comment(lib, "crypt32.lib")

void ProfileManagerImpl::Initialize()
{
	// load stored profiles
	LoadStoredProfiles();

	// initialize profiles from profile suggestion providers
	for (auto&& provider : m_suggestionProviders)
	{
		provider->GetProfiles([=] (fwRefContainer<Profile> profile)
		{
			fwRefContainer<ProfileImpl> profileImpl = profile;

			// find a matching profile - only add it to the list if none match any existing profile
			bool matchesExistingProfile = false;

			size_t hashKey = 0;

			for (int i = 0; i < profileImpl->GetNumIdentifiers(); i++)
			{
				ProfileIdentifier identifier = profileImpl->GetIdentifierInternal(i);
				
				for (auto&& profile : m_profiles)
				{
					auto& thatProfile = profile.second;

					for (int j = 0; j < thatProfile->GetNumIdentifiers(); j++)
					{
						ProfileIdentifier thatIdentifier = thatProfile->GetIdentifierInternal(j);

						if (identifier == thatIdentifier)
						{
							matchesExistingProfile = true;
							break;
						}
					}
				}

				hashKey ^= 3 * std::hash<ProfileIdentifier>()(identifier);
			}

			// clip the hash key to 32 bits
			hashKey &= UINT32_MAX;

			// mark the profile as suggestion
			profileImpl->SetIsSuggestion(true);

			// set the internal identifier
			profileImpl->SetInternalIdentifier(hashKey);

			if (!matchesExistingProfile)
			{
				m_profiles[hashKey] = profileImpl;
				m_profileIndices.push_back(hashKey);
			}
		});
	}
}

void ProfileManagerImpl::LoadStoredProfiles()
{
	PWSTR appDataPath;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataPath)))
	{
		// create the directory if not existent
		std::wstring cfxPath = std::wstring(appDataPath) + L"\\CitizenFX";
		CreateDirectory(cfxPath.c_str(), nullptr);

		// open and read the profile file
		std::wstring profilePath = cfxPath + L"\\profiles.json";

		if (FILE* profileFile = _wfopen(profilePath.c_str(), L"rb"))
		{
			std::vector<uint8_t> profileFileData;
			int pos;

			// get the file length
			fseek(profileFile, 0, SEEK_END);
			pos = ftell(profileFile);
			fseek(profileFile, 0, SEEK_SET);

			// resize the buffer
			profileFileData.resize(pos);

			// read the file and close it
			fread(&profileFileData[0], 1, pos, profileFile);

			fclose(profileFile);

			// decrypt the stored data - setup blob
			DATA_BLOB cryptBlob;
			cryptBlob.pbData = &profileFileData[0];
			cryptBlob.cbData = profileFileData.size();

			DATA_BLOB outBlob;

			// call DPAPI
			if (CryptUnprotectData(&cryptBlob, nullptr, nullptr, nullptr, nullptr, 0, &outBlob))
			{
				// parse the profile list
				ParseStoredProfiles(std::string(reinterpret_cast<char*>(outBlob.pbData), outBlob.cbData));

				// free the out data
				LocalFree(outBlob.pbData);
			}
		}
		

		CoTaskMemFree(appDataPath);
	}
}

void ProfileManagerImpl::ParseStoredProfiles(const std::string& profileList)
{
	rapidjson::Document document;
	document.Parse(profileList.c_str());

	if (!document.HasParseError())
	{
		if (document.IsObject())
		{
			if (document.HasMember("profiles") && document["profiles"].IsArray())
			{
				// loop through each profile
				auto& profiles = document["profiles"];

				std::for_each(profiles.Begin(), profiles.End(), [&] (rapidjson::Value& value)
				{
					if (!value.IsObject())
					{
						return;
					}

					// get display name
					if (!value.HasMember("displayName"))
					{
						return;
					}

					auto& displayNameMember = value["displayName"];

					if (!displayNameMember.IsString())
					{
						return;
					}

					// get tile URI
					if (!value.HasMember("tileUri"))
					{
						return;
					}

					auto& tileUriMember = value["tileUri"];

					if (!tileUriMember.IsString())
					{
						return;
					}

					// get identifier pairs
					if (!value.HasMember("identifiers"))
					{
						return;
					}

					auto& identifiersMember = value["identifiers"];

					if (!identifiersMember.IsArray())
					{
						return;
					}

					// enumerate identifier pairs
					std::vector<ProfileIdentifier> identifiers;
					size_t hashKey = 0;

					std::for_each(identifiersMember.Begin(), identifiersMember.End(), [&] (rapidjson::Value& identifierMember)
					{
						if (!identifierMember.IsArray())
						{
							return;
						}

						if (identifierMember.Size() != 2)
						{
							return;
						}

						auto& left = identifierMember[(rapidjson::SizeType)0]; // regular 0 is ambiguous as it could be a pointer
						auto& right = identifierMember[1];

						if (!left.IsString())
						{
							return;
						}

						if (!right.IsString())
						{
							return;
						}

						// add the identifier
						ProfileIdentifier identifier = std::make_pair(left.GetString(), right.GetString());

						identifiers.push_back(identifier);
						hashKey ^= 3 * std::hash<ProfileIdentifier>()(identifier);
					});

					// create an implemented profile
					fwRefContainer<ProfileImpl> profile = new ProfileImpl();
					profile->SetDisplayName(std::string(displayNameMember.GetString(), displayNameMember.GetStringLength()));
					profile->SetTileURI(std::string(tileUriMember.GetString(), tileUriMember.GetStringLength()));

					profile->SetIdentifiers(identifiers);

					m_profiles[hashKey] = profile;
				});
			}
		}
	}
}

void ProfileManagerImpl::AddSuggestionProvider(fwRefContainer<ProfileSuggestionProvider> provider)
{
	m_suggestionProviders.push_back(provider);
}

int ProfileManagerImpl::GetNumProfiles()
{
	return m_profileIndices.size();
}

fwRefContainer<Profile> ProfileManagerImpl::GetProfile(int index)
{
	return m_profiles[m_profileIndices[index]];
}

concurrency::task<ProfileTaskResult> ProfileManagerImpl::AddProfile(fwRefContainer<Profile> profile)
{
	ProfileTaskResult result;

	return concurrency::task_from_result<ProfileTaskResult>(result);
}

concurrency::task<ProfileTaskResult> ProfileManagerImpl::SetPrimaryProfile(fwRefContainer<Profile> profile)
{
	ProfileTaskResult result;

	return concurrency::task_from_result<ProfileTaskResult>(result);
}

concurrency::task<ProfileTaskResult> ProfileManagerImpl::SignIn(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters)
{
	ProfileTaskResult result;

	return concurrency::task_from_result<ProfileTaskResult>(result);
}

static ProfileManagerImpl* g_profileManager;

static InitFunction initFunction([] ()
{
	g_profileManager = new ProfileManagerImpl();
	Instance<ProfileManager>::Set(g_profileManager);
}, -500);

static InitFunction initFunctionPost([] ()
{
	g_profileManager->Initialize();
}, 500);