/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "ProfileManager.h"

typedef std::pair<std::string, std::string> ProfileIdentifier;

namespace std
{
	template<typename T1, typename T2>
	struct hash<std::pair<T1, T2>>
	{
		size_t operator()(const std::pair<T1, T2>& obj)
		{
			return (3 * std::hash<T1>()(obj.first)) ^ std::hash<T2>()(obj.second);
		}
	};
}

class ProfileSuggestionProvider : public fwRefCountable
{
public:
	virtual void GetProfiles(std::function<void(fwRefContainer<Profile>)> receiver) = 0;
};

class ProfileIdentityProvider : public fwRefCountable
{
public:
	virtual const char* GetIdentifierKey() = 0;

	virtual bool RequiresCredentials() = 0;

	virtual concurrency::task<ProfileTaskResult> SignIn(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters) = 0;
};

class ProfileImpl : public Profile
{
private:
	bool m_isSuggestion;

	std::string m_name;

	std::string m_uri;

	std::vector<ProfileIdentifier> m_identifiers;

	uint32_t m_internalIdentifier;

public:
	ProfileImpl();

	inline bool IsSuggestion()
	{
		return m_isSuggestion;
	}

	inline void SetIsSuggestion(bool isSuggestion)
	{
		m_isSuggestion = isSuggestion;
	}

	inline void SetInternalIdentifier(uint32_t identifier)
	{
		m_internalIdentifier = identifier;
	}

	ProfileIdentifier GetIdentifierInternal(int index);

	void SetDisplayName(const std::string& name);

	void SetTileURI(const std::string& uri);

	void SetIdentifiers(const std::vector<ProfileIdentifier>& identifiers);

	// Profile implementation
	virtual const char* GetDisplayName() override;

	virtual int GetNumIdentifiers() override;

	virtual const char* GetIdentifier(int index) override;

	virtual const char* GetTileURI() override;

	virtual bool IsSignedIn() override;

	virtual uint32_t GetInternalIdentifier() override;

	virtual concurrency::task<ProfileTaskResult> MergeProfile(fwRefContainer<Profile> fromProfile) override;
};

class ProfileManagerImpl : public ProfileManager
{
private:
	std::map<size_t, fwRefContainer<ProfileImpl>> m_profiles;

	std::vector<size_t> m_profileIndices;

	std::vector<fwRefContainer<ProfileSuggestionProvider>> m_suggestionProviders;

	std::map<std::string, fwRefContainer<ProfileIdentityProvider>> m_identityProviders;

private:
	void LoadStoredProfiles();

	void ParseStoredProfiles(const std::string& profileList);

public:
	void Initialize();

	void AddSuggestionProvider(fwRefContainer<ProfileSuggestionProvider> provider);

	// ProfileManager implementation
	virtual int GetNumProfiles() override;

	virtual fwRefContainer<Profile> GetProfile(int index) override;

	virtual concurrency::task<ProfileTaskResult> AddProfile(fwRefContainer<Profile> profile) override;

	virtual concurrency::task<ProfileTaskResult> SetPrimaryProfile(fwRefContainer<Profile> profile) override;

	virtual concurrency::task<ProfileTaskResult> SignIn(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters) override;
};