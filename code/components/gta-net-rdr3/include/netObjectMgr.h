#pragma once

#include <netObject.h>

class CNetGamePlayer;

namespace rage
{
class netObjectMgr
{
public:
	virtual ~netObjectMgr() = 0;

	virtual void m_8() = 0; // GetObjectIDManagerForPlayer
	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void Update() = 0;

	virtual void AddEntity() = 0; // RegisterObject
	virtual void m_28() = 0; // UnregisterObject
	virtual void UnregisterNetworkObject(rage::netObject* object, int reason, bool force1, bool force2) = 0;
	virtual void ChangeOwner(rage::netObject* object, CNetGamePlayer* player, int migrationType) = 0;
	virtual void m_40() = 0; // PlayerHasJoined
	virtual void m_48() = 0; // PlayerHasLeft
	virtual void m_50_1311() = 0; // added in 1311
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void RegisterNetworkObject(rage::netObject* entity) = 0;

private:
	struct atDNetObjectNode
	{
		virtual ~atDNetObjectNode();

		netObject* object;
		atDNetObjectNode* next;
	};

	struct ObjectHolder
	{
		atDNetObjectNode* objects;
		netObject** unk; // might not just be a netObject**
	};

private:
	ObjectHolder m_objects[32];

public:
	template<typename T>
	inline void ForAllNetObjects(int playerId, const T& callback)
	{
		for (auto node = m_objects[playerId].objects; node; node = node->next)
		{
			if (node->object)
			{
				callback(node->object);
			}
		}
	}

	netObject* GetNetworkObject(uint16_t id, bool a3);

	static netObjectMgr* GetInstance();
};
}
