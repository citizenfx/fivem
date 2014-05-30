// ==========================================================
// alterIWnet project
// 
// Component: xnp
// Sub-component: libnp
// Purpose: definitions for the NP server list service
//
// Initial author: NTAuthority
// Started: 2012-05-10
// ==========================================================

enum EServersResult
{
	ServersResultOK = 0,
	ServersResultNotAllowed = 1,
	ServersResultNotFound = 2
};

typedef uint64_t NPSID;

class NPSessionInfo
{
public:
	NPSID sid;
	uint32_t address;
	uint16_t port;
	NPID npid;
	int16_t players;
	int16_t maxplayers;

	NPDictionary data;
};

class NPCreateSessionResult
{
public:
	EServersResult result;
	NPSID sid;
};
