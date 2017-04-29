// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: definitions for the NP friends service
//
// Initial author: NTAuthority
// Started: 2012-01-16
// ==========================================================

enum EPresenceState
{
	PresenceStateUnknown,
	PresenceStateOnline,
	PresenceStateAway,
	PresenceStateExtendedAway,
	PresenceStateOffline
};


struct NPGetUserAvatarResult
{
	EGetFileResult result;
	int32_t guid;
	uint32_t fileSize;
	uint8_t* buffer;
};

struct NPProfileData
{
	NPID npID;
	int32_t experience;
	int32_t prestige;
};

struct NPGetProfileDataResult
{
	int32_t numResults;
	NPProfileData* results;
};

struct NPExtProfileData
{
	NPID npID;
	size_t bufLength;
	char buffer[65535];
};

struct NPGetExtProfileDataResult
{
	int32_t numResults;
	NPExtProfileData* results;
};