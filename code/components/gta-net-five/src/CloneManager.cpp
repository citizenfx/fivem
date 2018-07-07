#include "StdInc.h"
#include <CloneManager.h>

#include <state/RlMessageBuffer.h>

#include <NetBuffer.h>
#include <NetLibrary.h>

#include <rlNetBuffer.h>

#include <netBlender.h>
#include <netInterface.h>
#include <netObjectMgr.h>
#include <netSyncTree.h>

#include <lz4.h>

#include <boost/range/adaptor/map.hpp>

#include <ICoreGameInit.h>

#include <chrono>

#include <Hooking.h>

void ObjectIds_AddObjectId(int objectId);

void AssociateSyncTree(int objectId, rage::netSyncTree* syncTree);

using namespace std::chrono_literals;

inline std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static hook::cdecl_stub<uint32_t()> _getNetAckTimestamp([]()
{
	return hook::get_pattern("3B CA 76 02 FF", -0x31);
});

CNetGamePlayer* GetLocalPlayer();

CNetGamePlayer* GetPlayerByNetId(uint16_t);

void UpdateTime(uint64_t serverTime, bool isInit = false);

static bool g_doShit;

#include <CoreConsole.h>

static InitFunction initFunction([]()
{
	static ConsoleCommand cmd("doshit", []()
	{
		g_doShit = true;
	});
});

namespace sync
{
class msgClone;

class CloneManagerLocal : public CloneManager
{
public:
	virtual void Update() override;

	virtual void BindNetLibrary(NetLibrary* netLibrary) override;

	virtual uint16_t GetClientId(rage::netObject* netObject) override;

	virtual void GiveObjectToClient(rage::netObject* object, uint16_t clientId) override;

	virtual void OnObjectDeletion(rage::netObject* object) override;

	virtual void DeleteObjectId(uint16_t objectId) override;

private:
	void WriteUpdates();

	void SendUpdates();

	void AttemptFlushNetBuffer();

private:
	void HandleCloneAcks(const char* data, size_t len);

	void HandleCloneSync(const char* data, size_t len);

	void HandleCloneRemove(const char* data, size_t len);

	void HandleCloneCreate(const msgClone& msg);

	bool HandleCloneUpdate(const msgClone& msg);

private:
	NetLibrary* m_netLibrary;

private:
	std::chrono::milliseconds m_lastSend;
	rl::MessageBuffer m_sendBuffer{ 16384 };

private:
	struct ObjectData
	{
		std::chrono::milliseconds lastSyncTime;
		std::chrono::milliseconds lastSyncAck;

		ObjectData()
		{
			lastSyncTime = 0ms;
			lastSyncAck = 0ms;
		}
	};

	struct ExtendedCloneData
	{
		uint16_t clientId;

		inline ExtendedCloneData()
		{
		}

		inline ExtendedCloneData(uint16_t clientId)
			: clientId(clientId)
		{

		}
	};

private:
	std::unordered_map<int, ObjectData> m_trackedObjects;

	std::unordered_map<uint32_t, rage::netObject*> m_savedEntities;

	std::unordered_map<uint16_t, ExtendedCloneData> m_extendedData;
};

uint16_t CloneManagerLocal::GetClientId(rage::netObject* netObject)
{
	return m_extendedData[netObject->objectId].clientId;
}

void CloneManagerLocal::OnObjectDeletion(rage::netObject* netObject)
{
	auto& netBuffer = m_sendBuffer;

	netBuffer.Write(3, 3);
	//netBuffer.Write<uint8_t>(0); // player ID (byte)
	netBuffer.Write(13, netObject->objectId); // object ID (short)

	AttemptFlushNetBuffer();

	m_trackedObjects.erase(netObject->objectId);
	m_extendedData.erase(netObject->objectId);
	m_savedEntities.erase(netObject->objectId);
}

void CloneManagerLocal::BindNetLibrary(NetLibrary* netLibrary)
{
	// set the net library
	m_netLibrary = netLibrary;

	// add message handlers
	m_netLibrary->AddReliableHandler("msgCloneAcks", [this](const char* data, size_t len)
	{
		HandleCloneAcks(data, len);
	});

	m_netLibrary->AddReliableHandler("msgPackedClones", [this](const char* data, size_t len)
	{
		HandleCloneSync(data, len);
	});

	m_netLibrary->AddReliableHandler("msgCloneRemove", [this](const char* data, size_t len)
	{
		HandleCloneRemove(data, len);
	});
}

void CloneManagerLocal::HandleCloneAcks(const char* data, size_t len)
{
	net::Buffer buf(reinterpret_cast<const uint8_t*>(data), len);

	while (!buf.IsAtEnd())
	{
		auto type = buf.Read<uint8_t>();

		switch (type)
		{
			// create ack?
			case 1:
			{
				auto objId = buf.Read<uint16_t>();
				m_trackedObjects[objId].lastSyncAck = msec();

				break;
			}
			// sync ack?
			case 2:
			{
				auto objId = buf.Read<uint16_t>();
				auto timestamp = buf.Read<uint32_t>();

				auto netObjIt = m_savedEntities.find(objId);

				if (netObjIt != m_savedEntities.end())
				{
					auto netObj = netObjIt->second;

					if (netObj)
					{
						auto syncTree = netObj->GetSyncTree();

						if (netObj->m_20())
						{
							// 1290
							((void(*)(rage::netSyncTree*, rage::netObject*, uint8_t, uint16_t, uint32_t, int))0x1415D94F0)(syncTree, netObj, 31, 0 /* seq? */, timestamp, 0xFFFFFFFF);
						}
					}
				}
				break;
			}
			// timestamp ack?
			case 5:
			{
				auto timestamp = buf.Read<uint32_t>();

				for (auto& object : m_savedEntities)
				{
					rage::netObject* netObj = object.second;

					if (netObj && !netObj->syncData.isRemote)
					{
						auto syncTree = netObj->GetSyncTree();

						if (netObj->m_20())
						{
							// 1290
							((void(*)(rage::netSyncTree*, rage::netObject*, uint8_t, uint16_t, uint32_t, int))0x1415D94F0)(syncTree, netObj, 31, 0 /* seq? */, timestamp, 0xFFFFFFFF);
						}
					}
				}

				break;
			}
			// remove ack?
			case 3:
			{
				// this is now done the same time we send a remove
				/*auto objId = buf.Read<uint16_t>();
				m_trackedObjects.erase(objId);*/

				break;
			}
			default:
				return;
		}
	}
}

class msgClone
{
public:
	msgClone();

	void Read(net::Buffer& buffer);

	void Read(int syncType, rl::MessageBuffer& buffer);

	inline uint8_t GetSyncType() const
	{
		return m_syncType;
	}

	inline uint16_t GetClientId() const
	{
		return m_clientId;
	}

	inline uint16_t GetObjectId() const
	{
		return m_objectId;
	}

	inline NetObjEntityType GetEntityType() const
	{
		return m_entityType;
	}

	inline uint32_t GetTimestamp() const
	{
		return m_timestamp;
	}

	inline const std::vector<uint8_t>& GetCloneData() const
	{
		return m_cloneData;
	}

//private:
	uint32_t m_handle;
	uint16_t m_clientId;
	uint16_t m_objectId;
	NetObjEntityType m_entityType;
	uint8_t m_syncType;
	uint32_t m_timestamp;
	std::vector<uint8_t> m_cloneData;
};

msgClone::msgClone()
{
}

void msgClone::Read(net::Buffer& netBuffer)
{
	m_handle = netBuffer.Read<uint32_t>();
	m_clientId = netBuffer.Read<uint16_t>();
	m_objectId = netBuffer.Read<uint16_t>();
	m_entityType = (NetObjEntityType)netBuffer.Read<uint8_t>();
	m_syncType = netBuffer.Read<uint8_t>();

	m_timestamp = netBuffer.Read<uint32_t>();

	auto length = netBuffer.Read<uint16_t>();

	m_cloneData.resize(length);
	netBuffer.Read(m_cloneData.data(), m_cloneData.size());
}

void msgClone::Read(int syncType, rl::MessageBuffer& buffer)
{
	m_syncType = syncType;
	m_objectId = buffer.Read<uint16_t>(13);
	m_clientId = buffer.Read<uint16_t>(16);

	if (syncType == 1)
	{
		m_entityType = (NetObjEntityType)buffer.Read<uint8_t>(4);
	}

	m_timestamp = buffer.Read<uint32_t>(32);

	auto length = buffer.Read<uint16_t>(12);

	m_cloneData.resize(length);
	buffer.ReadBits(m_cloneData.data(), m_cloneData.size() * 8);
}

class msgPackedClones
{
public:
	msgPackedClones();

	void Read(net::Buffer& buffer);

	inline std::list<msgClone>& GetClones()
	{
		return m_clones;
	}

	inline uint64_t GetFrameIndex()
	{
		return m_frameIndex;
	}

private:
	uint64_t m_frameIndex;

	std::list<msgClone> m_clones;
};

msgPackedClones::msgPackedClones()
{
}

void msgPackedClones::Read(net::Buffer& buffer)
{
	m_frameIndex = buffer.Read<uint64_t>();

	uint8_t bufferData[16384] = { 0 };
	int bufferLength = LZ4_decompress_safe(reinterpret_cast<const char*>(&buffer.GetData()[buffer.GetCurOffset()]), reinterpret_cast<char*>(bufferData), buffer.GetRemainingBytes(), sizeof(bufferData));

	if (bufferLength > 0)
	{
		rl::MessageBuffer msgBuf(bufferData, bufferLength);

		bool end = false;

		while (!msgBuf.IsAtEnd() && !end)
		{
			auto dataType = msgBuf.Read<uint8_t>(3);

			switch (dataType)
			{
			case 1:
			case 2:
			{
				msgClone clone;
				clone.Read(dataType, msgBuf);

				m_clones.push_back(std::move(clone));
				break;
			}
			case 5:
			{
				uint32_t msecLow = msgBuf.Read<uint32_t>(32);
				uint32_t msecHigh = msgBuf.Read<uint32_t>(32);

				uint64_t serverTime = ((uint64_t(msecHigh) << 32) | msecLow);
				UpdateTime(serverTime);

				break;
			}
			case 7:
				end = true;
				break;
			}
		}
	}
}

void TempHackMakePhysicalPlayer(uint16_t clientId, int slotId = -1);

void CloneManagerLocal::HandleCloneCreate(const msgClone& msg)
{
	auto ackPacket = [&]()
	{
		// send ack
		net::Buffer outBuffer;
		outBuffer.Write<uint16_t>(msg.GetObjectId());

		m_netLibrary->SendReliableCommand("ccack", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());
	};

	// create buffer
	rage::netBuffer rlBuffer(const_cast<uint8_t*>(msg.GetCloneData().data()), msg.GetCloneData().size());
	rlBuffer.m_f1C = 1;

	if (msg.GetClientId() == m_netLibrary->GetServerNetID())
	{
		ackPacket();

		return;
	}

	// skip if the player hasn't been created yet
	if (GetPlayerByNetId(msg.GetClientId()) == nullptr)
	{
		return;
	}

	// get sync tree and read data
	auto syncTree = rage::netSyncTree::GetForType(msg.GetEntityType());
	syncTree->ReadFromBuffer(1, 0, &rlBuffer, nullptr);

	// find existence
	bool exists = false;

	rage::netObjectMgr::GetInstance()->ForAllNetObjects(31, [&](rage::netObject* object)
	{
		if (object->objectId == msg.GetObjectId())
		{
			exists = true;
		}
	});

	// already exists! bail out
	if (exists)
	{
		return;
	}

	m_extendedData[msg.GetObjectId()] = { msg.GetClientId() };

	// create the object
	auto obj = rage::CreateCloneObject(msg.GetEntityType(), msg.GetObjectId(), 31, 0, 32);
	obj->syncData.isRemote = true;

	// check if we can apply
	if (!syncTree->CanApplyToObject(obj))
	{
		//trace("Couldn't apply object.\n");

		// delete the unapplied object
		delete obj;
		return;
	}

	AssociateSyncTree(obj->objectId, syncTree);

	// apply object creation
	syncTree->ApplyToObject(obj, nullptr);

	// register with object mgr
	rage::netObjectMgr::GetInstance()->RegisterObject(obj);

	// initialize blend
	if (obj->GetBlender())
	{
		obj->GetBlender()->SetTimestamp(msg.GetTimestamp());

		obj->m_1D0();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();
	}

	obj->m_1C0();

	m_savedEntities[msg.GetObjectId()] = obj;

	ackPacket();
}

bool CloneManagerLocal::HandleCloneUpdate(const msgClone& msg)
{
	// create buffer
	rage::netBuffer rlBuffer(const_cast<uint8_t*>(msg.GetCloneData().data()), msg.GetCloneData().size());
	rlBuffer.m_f1C = 1;

	auto ackPacket = [&]()
	{
// 		// send ack
// 		net::Buffer outBuffer;
// 		outBuffer.Write<uint16_t>(msg.GetObjectId());
// 
// 		m_netLibrary->SendReliableCommand("csack", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());
	};

	// get saved object
	auto obj = m_savedEntities[msg.GetObjectId()];

	if (!obj)
	{
		ackPacket();

		// pretend it acked, we don't want the server to spam us with even more nodes we can't handle
		return true;
	}

	// update client id if changed
	auto& extData = m_extendedData[msg.GetObjectId()];

	if (extData.clientId != msg.GetClientId())
	{
		trace("reassigning!\n");

		auto clientId = msg.GetClientId();

		if (clientId == m_netLibrary->GetServerNetID())
		{
			auto player = GetLocalPlayer();

			// add the object
			rage::netObjectMgr::GetInstance()->AddObjectForPlayer(obj, player, 0);

			// this isn't remote anymore
			obj->syncData.isRemote = false;

			// give us the object ID
			ObjectIds_AddObjectId(msg.GetObjectId());
		}
		else
		{
			auto player = GetPlayerByNetId(clientId);

			if (player)
			{
				auto lastId = player->physicalPlayerIndex;
				player->physicalPlayerIndex = 31;

				rage::netObjectMgr::GetInstance()->AddObjectForPlayer(obj, player, 0);

				player->physicalPlayerIndex = lastId;

				obj->syncData.isRemote = true;
			}
		}

		// this should happen AFTER AddObjectForPlayer, it verifies the object owner
		extData.clientId = clientId;
	}

	if (msg.GetClientId() == m_netLibrary->GetServerNetID())
	{
		m_savedEntities[msg.GetObjectId()] = obj;

		ackPacket();

		// our object, it's fine
		return true;
	}

	// get sync tree and read data
	auto syncTree = obj->GetSyncTree();
	syncTree->ReadFromBuffer(2, 0, &rlBuffer, nullptr);

	// TODO: call m1E0

	if (!syncTree->CanApplyToObject(obj))
	{
		//trace("Couldn't apply object.\n");
		// couldn't apply, ignore ack
		return false;
	}

	// apply pre-blend
	if (obj->GetBlender())
	{
		obj->GetBlender()->SetTimestamp(msg.GetTimestamp());

		if (!obj->syncData.isRemote)
		{
			obj->GetBlender()->m_28();
		}
	}

	AssociateSyncTree(obj->objectId, syncTree);

	// apply to object
	syncTree->ApplyToObject(obj, nullptr);

	// call post-apply
	obj->m_1D0();

	m_savedEntities[msg.GetObjectId()] = obj;

	ackPacket();

	return true;
}

void CloneManagerLocal::HandleCloneSync(const char* data, size_t len)
{
	if (!rage::netObjectMgr::GetInstance())
	{
		return;
	}

	net::Buffer netBuffer(reinterpret_cast<const uint8_t*>(data), len);

	// read the packet
	msgPackedClones msg;
	msg.Read(netBuffer);

	std::vector<uint16_t> ignoreList;

	for (auto& clone : msg.GetClones())
	{
		switch (clone.GetSyncType())
		{
		case 1:
			HandleCloneCreate(clone);
			break;
		case 2:
		{
			bool acked = HandleCloneUpdate(clone);

			if (!acked)
			{
				ignoreList.push_back(clone.GetObjectId());
			}

			break;
		}
		}
	}

	{
		net::Buffer outBuffer;
		outBuffer.Write<uint64_t>(msg.GetFrameIndex());
		outBuffer.Write<uint8_t>(uint8_t(ignoreList.size()));

		for (uint16_t entry : ignoreList)
		{
			outBuffer.Write<uint16_t>(entry);
		}

		m_netLibrary->SendReliableCommand("gameStateAck", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());
	}
}

void CloneManagerLocal::HandleCloneRemove(const char* data, size_t len)
{
	net::Buffer netBuffer(reinterpret_cast<const uint8_t*>(data), len);
	auto objectId = netBuffer.Read<uint16_t>();

	DeleteObjectId(objectId);
}

void CloneManagerLocal::DeleteObjectId(uint16_t objectId)
{
	auto object = m_savedEntities[objectId];

	if (object)
	{
		// set flags
		object->syncData.wantsToDelete = true;
		object->syncData.shouldNotBeDeleted = false;

		// unack the create to unburden the game
		object->syncData.creationAckedPlayers &= ~(1 << 31);

		// call object manager clone removal
		rage::netObjectMgr::GetInstance()->RemoveClone(object, 8, 0, 1);
	}
}

void CloneManagerLocal::GiveObjectToClient(rage::netObject* object, uint16_t clientId)
{
	m_sendBuffer.Write(3, 4);
	m_sendBuffer.Write(16, clientId); // client ID
	//m_sendBuffer.Write<uint8_t>(0); // player ID (byte)
	m_sendBuffer.Write(13, object->objectId);

	AttemptFlushNetBuffer();
}

void CloneManagerLocal::Update()
{
	WriteUpdates();

	SendUpdates();

	// temp? run Update() on all remote clones
	for (auto& clone : m_savedEntities)
	{
		if (clone.second && clone.second->syncData.isRemote)
		{
			clone.second->Update();
		}
	}
}

void CloneManagerLocal::WriteUpdates()
{
	auto objectMgr = rage::netObjectMgr::GetInstance();

	if (!objectMgr)
	{
		return;
	}

	int syncCount1 = 0, syncCount2 = 0, syncCount3 = 0, syncCount4 = 0;

	{
		static bool didShit;

		if (!didShit)
		{
			if (Instance<ICoreGameInit>::Get()->HasVariable("networkInited"))
			{
				if (g_doShit)
				{
					for (int i = 0; i < 48; i++)
					{
						TempHackMakePhysicalPlayer(1);

						msgClone msg;
						msg.m_handle = i + 256;
						msg.m_entityType = NetObjEntityType::Player;
						msg.m_syncType = 1;
						msg.m_timestamp = rage::netInterface_queryFunctions::GetInstance()->GetTimestamp();
						msg.m_objectId = i + 256;

						FILE* f = fopen("L:/tmp/player_1.bin", "rb");
						fseek(f, 0, SEEK_END);
						int len = ftell(f);
						fseek(f, 0, SEEK_SET);

						msg.m_cloneData.resize(len);
						fread(msg.m_cloneData.data(), 1, len, f);

						fclose(f);

						HandleCloneCreate(msg);
					}

					didShit = true;
				}
			}
		}
	}

	{
		uint32_t timestamp = rage::netInterface_queryFunctions::GetInstance()->GetTimestamp();
		uint32_t ackTimestamp = _getNetAckTimestamp();

		m_sendBuffer.Write(3, 5);
		m_sendBuffer.Write(32, timestamp);
		m_sendBuffer.Write(32, ackTimestamp);
	}

	// collect object IDs that we have seen this time
	std::set<int> seenObjects;

	// on each object...
	auto objectCb = [&](rage::netObject* object)
	{
		// skip remote objects
		if (object->syncData.isRemote)
		{
			return;
		}

		// get basic object data
		auto objectType = object->objectType;
		auto objectId = object->objectId;

		// store a reference to the object tracking data
		auto& objectData = m_trackedObjects[objectId];

		// allocate a RAGE buffer
		uint8_t packetStub[1200] = { 0 };
		rage::netBuffer rlBuffer(packetStub, sizeof(packetStub));

		// if we want to delete this object
		if (object->syncData.wantsToDelete)
		{
			// has this been acked by client 31?
			if (object->syncData.IsCreationAckedByPlayer(31))
			{
				auto& netBuffer = m_sendBuffer;

				++syncCount3;

				netBuffer.Write(3, 3);
				//netBuffer.Write<uint8_t>(0); // player ID (byte)
				netBuffer.Write(13, object->objectId); // object ID (short)

				AttemptFlushNetBuffer();

				// unack the create to unburden the game
				object->syncData.creationAckedPlayers &= ~(1 << 31);

				// pretend to ack the remove to process removal
				// 1103
				// 1290
				((void(*)(rage::netObjectMgr*, rage::netObject*))0x1415C8CC8)(objectMgr, object);
			}

			// don't actually continue sync
			return;
		}

		// get the sync tree
		auto syncTree = object->GetSyncTree();

		// determine sync type from sync data
		int syncType = 0;

		if (objectData.lastSyncTime == 0ms)
		{
			// clone create
			syncType = 1;

			// TEMP: SYNC ACK
			// REMOVE THIS!!
			//objectData.lastSyncAck = msec();
		}
		else if ((msec() - objectData.lastSyncTime) > 100ms)
		{
			if (objectData.lastSyncAck == 0ms)
			{
				// create resend
				syncType = 1;
			}
			else
			{
				// clone sync
				syncType = 2;
			}
		}

		// if this is clone sync, perform some actions to set up the object
		if (syncType == 2)
		{
			// ack'd create on player 31
			object->syncData.creationAckedPlayers |= (1 << 31);

			// done by netObject::m108
			object->syncData.m6C |= (1 << 31);

			// set sync priority
			object->syncData.SetCloningFrequency(31, object->GetSyncFrequency());

			char* syncData = (char*)object->m_20();

			if (syncData)
			{
				*(uint32_t*)(syncData + 8) |= (1 << 31);
				*(uint32_t*)(syncData + 176 + 8) |= (1 << 31);
			}
		}

		// if we should sync
		if (syncType != 0)
		{
			auto& netBuffer = m_sendBuffer;

			if (syncType == 1)
			{
				++syncCount1;
			}
			else if (syncType == 2)
			{
				++syncCount2;
			}

			// write tree
			syncTree->WriteTreeCfx(syncType, 0, object, &rlBuffer, rage::netInterface_queryFunctions::GetInstance()->GetTimestamp(), nullptr, 31, nullptr);

			AssociateSyncTree(object->objectId, syncTree);

			// instantly mark player 31 as acked
			if (object->m_20())
			{
				// 1290
				//((void(*)(rage::netSyncTree*, rage::netObject*, uint8_t, uint16_t, uint32_t, int))0x1415D94F0)(syncTree, object, 31, 0 /* seq? */, 0x7FFFFFFF, 0xFFFFFFFF);
			}

			// write header to send buffer
			netBuffer.Write(3, syncType);

			// write data
			//netBuffer.Write<uint8_t>(getPlayerId()); // player ID (byte)
			//netBuffer.Write<uint8_t>(0); // player ID (byte)
			netBuffer.Write(13, objectId); // object ID (short)

			if (syncType == 1)
			{
				netBuffer.Write(4, objectType);
			}

			//netBuffer.Write<uint32_t>(rage::netInterface_queryFunctions::GetInstance()->GetTimestamp()); // timestamp?

			uint32_t len = rlBuffer.GetDataLength();
			netBuffer.Write(12, len); // length (short)
			netBuffer.WriteBits(rlBuffer.m_data, len * 8); // data

			//trace("uncompressed clone sync for %d: %d bytes\n", objectId, len);

			AttemptFlushNetBuffer();

			objectData.lastSyncTime = msec();
		}

		if (object->syncData.nextOwnerId != 0xFF)
		{
			if (object->syncData.nextOwnerId == 0)
			{
				m_sendBuffer.Write(3, 4);
				m_sendBuffer.Write(16, 0); // client ID
				//m_sendBuffer.Write<uint8_t>(0); // player ID (byte)
				m_sendBuffer.Write(13, objectId);

				++syncCount4;

				AttemptFlushNetBuffer();
			}
			else
			{
				trace("Tried to migrate an object to %d - but we can't map them yet.\n", object->syncData.nextOwnerId);
				object->syncData.nextOwnerId = -1;
			}
		}

		seenObjects.insert(objectId);
		m_savedEntities[objectId] = object;
		m_extendedData[objectId].clientId = m_netLibrary->GetServerNetID();
	};

	for (int i = 0; i < 64; i++)
	{
		objectMgr->ForAllNetObjects(i, objectCb);
	}

	// process removals
	/*{
		auto& netBuffer = m_sendBuffer;

		// anyone gone?
		auto iter = m_trackedObjects | boost::adaptors::map_keys;

		std::vector<int> removedObjects;
		std::set_difference(iter.begin(), iter.end(), seenObjects.begin(), seenObjects.end(), std::back_inserter(removedObjects));

		for (auto obj : removedObjects)
		{
			auto& syncData = m_trackedObjects[obj];

			if ((msec() - syncData.lastSyncTime) > 100ms)
			{
				// write clone remove header
				netBuffer.Write(3, 3);
				//netBuffer.Write<uint8_t>(0); // player ID (byte)
				netBuffer.Write(13, obj); // object ID (short)

				AttemptFlushNetBuffer();

				++syncCount3;

				syncData.lastSyncTime = msec();
			}
		}
	}*/

	//trace("sync: got %d creates, %d syncs, %d removes and %d migrates\n", syncCount1, syncCount2, syncCount3, syncCount4);
}

void CloneManagerLocal::AttemptFlushNetBuffer()
{
	// flush the send buffer in case it could compress to >1100 bytes
	if (LZ4_compressBound(m_sendBuffer.GetDataLength()) > 1100)
	{
		SendUpdates();
	}
}

void CloneManagerLocal::SendUpdates()
{
	// if ((timeGetTime() - lastSend) > 100 || netBuffer.GetCurOffset() >= 1200)

	if (m_sendBuffer.GetDataLength() > 0)
	{
		m_sendBuffer.Write(3, 7);

		// compress and send data
		std::vector<char> outData(LZ4_compressBound(m_sendBuffer.GetDataLength()) + 4);
		int len = LZ4_compress_default(reinterpret_cast<const char*>(m_sendBuffer.GetBuffer().data()), outData.data() + 4, m_sendBuffer.GetDataLength(), outData.size() - 4);

		//trace("compressed %d bytes to %d bytes\n", m_sendBuffer.GetDataLength(), len);

		*(uint32_t*)(outData.data()) = HashString("netClones");
		m_netLibrary->RoutePacket(outData.data(), len + 4, 0xFFFF);

#if _DEBUG && WRITE_BYTEHUNK
		static int byteHunkId;

		FILE* f = _wfopen(MakeRelativeCitPath(fmt::sprintf(L"cache/byteHunk/bytehunk-%d.bin", byteHunkId++)).c_str(), L"wb");

		if (f)
		{
			fwrite(outData.data(), 1, len + 4, f);
			fclose(f);
		}
#endif
	}

	m_sendBuffer.SetCurrentBit(0);
	m_lastSend = msec();
}

CloneManagerLocal g_cloneMgr;
}

sync::CloneManager* TheClones = &sync::g_cloneMgr;
