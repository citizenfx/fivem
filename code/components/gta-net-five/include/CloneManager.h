#pragma once

#include <unordered_set>

class NetLibrary;

namespace rage
{
class netObject;
}

class CNetGamePlayer;

namespace sync
{
class INetObjMgrAbstraction
{
public:
	virtual ~INetObjMgrAbstraction() = default;

	virtual bool RegisterNetworkObject(rage::netObject* object) = 0;

	virtual void DestroyNetworkObject(rage::netObject* object) = 0;

	virtual void ChangeOwner(rage::netObject* object, CNetGamePlayer* player, int migrationType) = 0;

	virtual rage::netObject* GetNetworkObject(uint16_t id) = 0;

	virtual void ForAllNetObjects(int playerId, const std::function<void(rage::netObject*)>& callback, bool safe = false) = 0;
};

class CloneManager
{
public:
	virtual ~CloneManager() = default;

	virtual void Update() = 0;

	virtual void BindNetLibrary(NetLibrary* netLibrary) = 0;

	virtual uint16_t GetClientId(rage::netObject* netObject) = 0;

	virtual void GiveObjectToClient(rage::netObject* object, uint16_t clientId) = 0;

	virtual uint16_t GetPendingClientId(rage::netObject* netObject) = 0;

	virtual void SetTargetOwner(rage::netObject* object, uint16_t clientId) = 0;

	virtual void OnObjectDeletion(rage::netObject* object) = 0;

	virtual rage::netObject* GetNetObject(uint16_t objectId) = 0;

	virtual const std::unordered_set<rage::netObject*>& GetObjectList() = 0;

	// TEMP: for temporary use during player deletion
	virtual void DeleteObjectId(uint16_t objectId, bool force = false) = 0;

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
