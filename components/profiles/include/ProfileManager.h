/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ppltasks.h>

struct ProfileTaskResult
{
	bool success;
	std::string error;
	uint32_t profileId;

	inline ProfileTaskResult()
		: success(false), error("")
	{

	}

	inline ProfileTaskResult(bool success)
		: success(success)
	{

	}

	inline ProfileTaskResult(bool success, const char* error)
		: success(success), error(error)
	{

	}

	inline ProfileTaskResult(bool success, uint32_t profileId)
		: success(success), profileId(profileId)
	{

	}
};

class Profile : public fwRefCountable
{
public:
	virtual const char* GetDisplayName() = 0;

	virtual int GetNumIdentifiers() = 0;

	virtual const char* GetIdentifier(int index) = 0;

	virtual const char* GetTileURI() = 0;

	virtual const std::map<std::string, std::string>& GetParameters() = 0;

	virtual uint32_t GetInternalIdentifier() = 0;

	virtual bool IsSignedIn() = 0;

	virtual concurrency::task<ProfileTaskResult> MergeProfile(fwRefContainer<Profile> fromProfile) = 0;
};

class ProfileManager
{
public:
	virtual int GetNumProfiles() = 0;

	virtual fwRefContainer<Profile> GetProfile(int index) = 0;

	virtual fwRefContainer<Profile> GetDummyProfile() = 0;

	virtual concurrency::task<ProfileTaskResult> SetPrimaryProfile(fwRefContainer<Profile> profile) = 0;

	virtual concurrency::task<ProfileTaskResult> SignIn(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters) = 0;

	virtual fwRefContainer<Profile> GetPrimaryProfile() = 0;
};

DECLARE_INSTANCE_TYPE(ProfileManager);