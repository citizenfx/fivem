#pragma once

class NetLibrary;

namespace rage
{
class netObject;
}

namespace sync
{
class CloneManager
{
public:
	virtual ~CloneManager() = default;

	virtual void Update() = 0;

	virtual void BindNetLibrary(NetLibrary* netLibrary) = 0;

	virtual uint16_t GetClientId(rage::netObject* netObject) = 0;

	virtual void GiveObjectToClient(rage::netObject* object, uint16_t clientId) = 0;

	virtual void OnObjectDeletion(rage::netObject* object) = 0;

	// TEMP: for temporary use during player deletion
	virtual void DeleteObjectId(uint16_t objectId) = 0;
};
}

extern sync::CloneManager* TheClones;
