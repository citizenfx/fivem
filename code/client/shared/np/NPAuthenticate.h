// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: definitions for the NP authentication service
//
// Initial author: NTAuthority
// Started: 2011-06-28
// ==========================================================

enum EAuthenticateResult
{
	AuthenticateResultOK = 0,
	AuthenticateResultBadDetails = 1,
	AuthenticateResultServiceUnavailable = 2,
	AuthenticateResultBanned = 3,
	AuthenticateResultAlreadyLoggedIn = 4,
	AuthenticateResultUnknown = 9999
};

enum EExternalAuthState
{
	ExternalAuthStatePassed = 0,		// owns the game, cleared for playing
	ExternalAuthStateUnverified = 1,	// unverified account, needs further verification
	ExternalAuthStatePirate = 2,		// yarrrrrrrr, the user's a pirate!
	ExternalAuthStateError = 3,			// an error occurred during authentication; pass on as usual
	ExternalAuthStatePrivate = 4		// the external authentication profile is set as 'private'.
};

class NPRegisterServerResult
{
public:
	EAuthenticateResult result;
	char licenseKey[32];
	uint32_t serverID;
};

class NPAuthenticateResult
{
public:
	EAuthenticateResult result;
	NPID id;
	uint8_t sessionToken[32];
};

enum EValidateUserTicketResult
{
	ValidateUserTicketResultOK = 0,
	ValidateUserTicketResultInvalid = 1
};

class NPValidateUserTicketResult
{
public:
	EValidateUserTicketResult result;
	NPID id;
	int32_t groupID;
};

#pragma pack(push, 1)
class NPAuthenticateTicket
{
public:
	int32_t version;
	NPID clientID;
	NPID serverID;
	uint32_t time;
};
#pragma pack(pop)