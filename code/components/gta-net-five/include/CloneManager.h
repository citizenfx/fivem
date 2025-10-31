#pragma once

#include <unordered_set>

// uncomment this to enable cloning natives, remove this hack later.
//#define ONESYNC_CLONING_NATIVES

class NetLibrary;

namespace rage
{
class netObject;
}

class CNetGamePlayer;

extern CNetGamePlayer* g_playerList[256];
extern int g_playerListCount;

extern CNetGamePlayer* g_playerListRemote[256];
extern int g_playerListCountRemote;

namespace sync
{
struct FrameIndex
{
	union
	{
		struct
		{
			uint64_t lastFragment : 1;
			uint64_t currentFragment : 7;
			uint64_t frameIndex : 56;
		};

		uint64_t full;
	};

	FrameIndex()
		: full(0)
	{
	}

	FrameIndex(uint64_t idx)
		: full(idx)
	{
	}
};

class INetObjMgrAbstraction
{
public:
	virtual ~INetObjMgrAbstraction() = default;

	virtual bool RegisterNetworkObject(rage::netObject* object) = 0;

	virtual void DestroyNetworkObject(rage::netObject* object) = 0;

	virtual void ChangeOwner(rage::netObject* object, int oldOwnerId, CNetGamePlayer* player, int migrationType) = 0;

	virtual rage::netObject* GetNetworkObject(uint16_t id) = 0;

	virtual void ForAllNetObjects(int playerId, const std::function<void(rage::netObject*)>& callback, bool safe = false) = 0;
};

class CloneManager
{
public:
	virtual ~CloneManager() = default;

	virtual void Reset() = 0;

	virtual void Update() = 0;

	virtual void BindNetLibrary(NetLibrary* netLibrary) = 0;

	virtual uint16_t GetClientId(rage::netObject* netObject) = 0;

	virtual void GiveObjectToClient(rage::netObject* object, uint16_t clientId) = 0;

	virtual uint16_t GetPendingClientId(rage::netObject* netObject) = 0;

	virtual void SetTargetOwner(rage::netObject* object, uint16_t clientId) = 0;

	virtual void OnObjectDeletion(rage::netObject* object) = 0;

	virtual rage::netObject* GetNetObject(uint16_t objectId) = 0;

	virtual const std::vector<rage::netObject*>& GetObjectList() = 0;

	virtual bool IsRemovingObjectId(uint16_t objectId) = 0;

	// TEMP: for temporary use during player deletion
	virtual void DeleteObjectId(uint16_t objectId, uint16_t uniqifier, bool force = false) = 0;
	
	virtual void HandleCloneSync(const char* data, size_t len) = 0;

	virtual void HandleCloneAcks(const char* data, size_t len) = 0;

	virtual const std::map<int, rage::netObject*>& GetPlayerObjects(uint8_t playerId) = 0;

public:
	virtual void Logv(const char* format, fmt::printf_args argumentList) = 0;

	template<typename... TArgs>
	inline void Log(const char* format, const TArgs&... args)
	{
		Logv(format, fmt::make_printf_args(args...));
	}
};
}

extern sync::CloneManager* TheClones;
extern sync::INetObjMgrAbstraction* CloneObjectMgr;
