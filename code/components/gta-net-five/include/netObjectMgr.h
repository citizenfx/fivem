#pragma once

#include <netObject.h>

class CNetGamePlayer;

namespace rage
{
class netObjectMgr
{
public:
	virtual ~netObjectMgr() = 0;

	virtual void Initialize() = 0;
	virtual void Shutdown() = 0;
	virtual void Update() = 0;

	virtual void AddEntity() = 0;
	virtual void m_28() = 0;
	virtual void RemoveClone(rage::netObject* object, int reason, bool force1, bool force2) = 0;
	virtual void AddObjectForPlayer(rage::netObject* object, CNetGamePlayer* player, int a4) = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void RegisterObject(rage::netObject* entity) = 0;

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

	static netObjectMgr* GetInstance();
};
}
