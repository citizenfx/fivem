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

#include <array>
#include <chrono>

#include <EntitySystem.h>

#include <Hooking.h>

#include <tbb/concurrent_queue.h>

#include <CrossBuildRuntime.h>

#include <ResourceManager.h>
#include <StateBagComponent.h>

#include <CoreConsole.h>

extern rage::netObject* g_curNetObjectSelection;
rage::netObject* g_curNetObject;

static std::set<uint16_t> g_dontParrotDeletionAcks;

void ObjectIds_AddObjectId(int objectId);
void ObjectIds_StealObjectId(int objectId);
void ObjectIds_ConfirmObjectId(int objectId);

void AssociateSyncTree(int objectId, rage::netSyncTree* syncTree);

rage::netObject* GetLocalPlayerPedNetObject();

using namespace std::chrono_literals;

inline std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static ICoreGameInit* icgi;

static hook::cdecl_stub<uint32_t()> _getNetAckTimestamp([]()
{
	return hook::get_pattern("3B CA 76 02 FF", -0x31);
});

extern CNetGamePlayer* g_players[256];
extern std::unordered_map<uint16_t, CNetGamePlayer*> g_playersByNetId;
extern std::unordered_map<CNetGamePlayer*, uint16_t> g_netIdsByPlayer;

std::string GetType(void* d);

CNetGamePlayer* GetLocalPlayer();

CNetGamePlayer* GetPlayerByNetId(uint16_t);

void UpdateTime(uint64_t serverTime, bool isInit = false);

bool IsWaitingForTimeSync();

extern uint32_t* rage__s_NetworkTimeThisFrameStart;
extern uint32_t* rage__s_NetworkTimeLastFrameStart;

namespace sync
{
class msgClone;

enum class AckResult
{
	OK,
	ResendClone,
	ResendCreate
};

class CloneManagerLocal : public CloneManager, public INetObjMgrAbstraction, public fx::StateBagGameInterface
{
public:
	virtual void Update() override;

	virtual void BindNetLibrary(NetLibrary* netLibrary) override;

	virtual uint16_t GetClientId(rage::netObject* netObject) override;

	virtual void GiveObjectToClient(rage::netObject* object, uint16_t clientId) override;

	virtual uint16_t GetPendingClientId(rage::netObject* netObject) override;

	virtual void SetTargetOwner(rage::netObject* object, uint16_t clientId) override;

	virtual void OnObjectDeletion(rage::netObject* object) override;

	virtual rage::netObject* GetNetObject(uint16_t objectId) override;

	virtual void DeleteObjectId(uint16_t objectId, bool force) override;

	virtual void Logv(const char* format, fmt::printf_args argumentList) override;

	virtual const std::vector<rage::netObject*>& GetObjectList() override;

	virtual bool IsRemovingObjectId(uint16_t objectId) override;

	// netobjmgr abstraction
	virtual bool RegisterNetworkObject(rage::netObject* object) override;

	virtual void DestroyNetworkObject(rage::netObject* object) override;

	virtual void ChangeOwner(rage::netObject* object, CNetGamePlayer* player, int migrationType) override;

	virtual rage::netObject* GetNetworkObject(uint16_t id) override;

	virtual void ForAllNetObjects(int playerId, const std::function<void(rage::netObject*)>& callback, bool safe) override
	{
		if (safe)
		{
			auto nol = m_netObjects[playerId]; // copy the list, we want to remove from it

			for (auto& entry : nol)
			{
				callback(entry.second);
			}
		}
		else
		{
			for (auto& entry : m_netObjects[playerId])
			{
				callback(entry.second);
			}
		}
	}

private:
	void WriteUpdates();

	void SendUpdates(rl::MessageBuffer& buffer, uint32_t msgType);

	void AttemptFlushNetBuffer(rl::MessageBuffer& buffer, uint32_t msgType);

	void AttemptFlushCloneBuffer();

	void AttemptFlushAckBuffer();

private:
	void HandleCloneAcks(const char* data, size_t len);

	void HandleCloneAcksNew(const char* data, size_t len);

	void HandleCloneSync(const char* data, size_t len);

	void HandleCloneRemove(const char* data, size_t len);

	bool HandleCloneCreate(const msgClone& msg);

	AckResult HandleCloneUpdate(const msgClone& msg);

	void CheckMigration(const msgClone& msg);

	void AddCreateAck(uint16_t objectId);

	void AddRemoveAck(uint16_t objectId);

	void ProcessCreateAck(uint16_t objectId, uint16_t uniqifier = 0);

	void ProcessSyncAck(uint16_t objectId, uint16_t uniqifier = 0);

	void ProcessRemoveAck(uint16_t objectId, uint16_t uniqifier = 0);

	void ProcessTimestampAck(uint32_t timestamp);

	virtual void SendPacket(int peer, std::string_view data) override;

private:
	NetLibrary* m_netLibrary;

private:
	std::chrono::milliseconds m_lastSend;
	std::chrono::milliseconds m_lastAck;
	rl::MessageBuffer m_sendBuffer{ 16384 };
	rl::MessageBuffer m_ackBuffer{ 16384 };

	uint32_t m_ackTimestamp{ 0 };

private:
	struct ObjectData
	{
		std::chrono::milliseconds lastSyncTime;
		std::chrono::milliseconds lastSyncAck;
		uint32_t lastChangeTime;
		uint32_t lastResendTime;
		uint32_t nextKeepaliveSync;
		uint16_t uniqifier;
		std::shared_ptr<fx::StateBag> stateBag;
		bool hi = false;

		ObjectData()
		{
			lastSyncTime = 0ms;
			lastSyncAck = 0ms;
			nextKeepaliveSync = 0;
			lastChangeTime = 0;
			lastResendTime = 0;
			uniqifier = rand();
		}
	};

	struct ExtendedCloneData
	{
		uint16_t clientId;
		uint16_t pendingClientId;
		uint32_t dontSyncBefore;

		inline ExtendedCloneData()
			: clientId(0), pendingClientId(-1), dontSyncBefore(0)
		{
		}

		inline ExtendedCloneData(uint16_t clientId)
			: clientId(clientId), pendingClientId(-1), dontSyncBefore(0)
		{
		}
	};

private:
	std::unordered_map<int, ObjectData> m_trackedObjects;

	std::unordered_map<uint32_t, rage::netObject*> m_savedEntities;

	std::unordered_set<rage::netObject*> m_savedEntitySet;

	std::vector<rage::netObject*> m_savedEntityVec;

	std::unordered_map<uint16_t, ExtendedCloneData> m_extendedData;

	std::map<std::tuple<int, int>, std::chrono::milliseconds> m_pendingRemoveAcks;

	std::set<int> m_pendingConfirmObjectIds;

	tbb::concurrent_queue<std::string> m_logQueue;

	std::condition_variable m_consoleCondVar;
	std::mutex m_consoleMutex;

	std::string m_logFile;

	std::array<std::map<int, rage::netObject*>, 256> m_netObjects;

	std::shared_ptr<fx::StateBag> m_globalBag;

	fwRefContainer<fx::StateBagComponent> m_sbac;
};

uint16_t CloneManagerLocal::GetClientId(rage::netObject* netObject)
{
	return m_extendedData[netObject->objectId].clientId;
}

uint16_t CloneManagerLocal::GetPendingClientId(rage::netObject* netObject)
{
	return m_extendedData[netObject->objectId].pendingClientId;
}

void CloneManagerLocal::Logv(const char* format, fmt::printf_args argumentList)
{
	if (!m_logFile.empty())
	{
		m_logQueue.push(fmt::sprintf("[% 10d] ", (!IsWaitingForTimeSync()) ? rage::netInterface_queryFunctions::GetInstance()->GetTimestamp() : 0));
		m_logQueue.push(fmt::vsprintf(format, argumentList));

		m_consoleCondVar.notify_all();
	}
}

void CloneManagerLocal::OnObjectDeletion(rage::netObject* netObject)
{
	Log("%s: %s\n", __func__, netObject->ToString());

	if (!netObject->syncData.isRemote)
	{
		if (m_trackedObjects[netObject->objectId].lastSyncTime != 0ms)
		{
			m_pendingRemoveAcks.insert({ { netObject->objectId, m_trackedObjects[netObject->objectId].uniqifier }, msec() });
		}
	}

	m_trackedObjects.erase(netObject->objectId);
	m_extendedData.erase(netObject->objectId);
	m_savedEntities.erase(netObject->objectId);
	m_savedEntitySet.erase(netObject);

	m_savedEntityVec.erase(std::remove(m_savedEntityVec.begin(), m_savedEntityVec.end(), netObject), m_savedEntityVec.end());
}

void CloneManagerLocal::SendPacket(int peer, std::string_view data)
{
	m_netLibrary->SendReliableCommand("msgStateBag", data.data(), data.size());
}

void CloneManagerLocal::BindNetLibrary(NetLibrary* netLibrary)
{
	// set the net library
	m_netLibrary = netLibrary;

	// add message handlers
	m_netLibrary->AddReliableHandler(
	"msgCloneAcks", [this](const char* data, size_t len)
	{
		HandleCloneAcks(data, len);
	},
	true);

	m_netLibrary->AddReliableHandler(
	"msgPackedClones", [this](const char* data, size_t len)
	{
		HandleCloneSync(data, len);
	},
	true);

	m_netLibrary->AddReliableHandler(
	"msgPackedAcks", [this](const char* data, size_t len)
	{
		HandleCloneAcksNew(data, len);
	},
	true);

	m_netLibrary->AddReliableHandler(
	"msgCloneRemove", [this](const char* data, size_t len)
	{
		HandleCloneRemove(data, len);
	},
	true);

	std::thread([this]()
	{
		while (true)
		{
			{
				std::unique_lock<std::mutex> lock(m_consoleMutex);
				m_consoleCondVar.wait(lock);
			}

			static std::string lastLogFile;
			static FILE* file;

			if (lastLogFile != m_logFile)
			{
				if (file)
				{
					fclose(file);
					file = nullptr;
				}

				if (!m_logFile.empty())
				{
					file = _pfopen(MakeRelativeCitPath(m_logFile).c_str(), _P("w"));
				}

				lastLogFile = m_logFile;
			}

			std::string str;

			while (m_logQueue.try_pop(str))
			{
				if (file)
				{
					fprintf(file, "%s", str.c_str());
				}
			}
		}
	})
	.detach();

	static ConVar<std::string> logFile("onesync_logFile", ConVar_None, "", &m_logFile);

	static ConsoleCommand printObj("net_printOwner", [this](int objectId)
	{
		auto it = m_savedEntities.find(objectId);

		if (it == m_savedEntities.end() || !it->second)
		{
			console::PrintError("CloneManager", "Couldn't find object by ID %d\n", objectId);
			return;
		}

		rage::netObject* obj = it->second;
		auto& extData = m_extendedData[obj->objectId];

		console::Printf("CloneManager", "-- NETWORK OBJECT %d (%s) --\n", obj->objectId, GetType(obj));
		console::Printf("CloneManager", "Owner: %s (%d)\n", g_playersByNetId[extData.clientId] ? g_playersByNetId[extData.clientId]->GetName() : "null?", extData.clientId);
		console::Printf("CloneManager", "Is remote: %s\n", obj->syncData.isRemote ? "yes" : "no");
		console::Printf("CloneManager", "Game client ID: %d\n", obj->syncData.ownerId);
		console::Printf("CloneManager", "\n");
	});

	icgi = Instance<ICoreGameInit>::Get();

	// #TODO: shutdown session logic!!
	auto sbac = fx::StateBagComponent::Create(fx::StateBagRole::Client);
	m_globalBag = sbac->RegisterStateBag("global");

	sbac->RegisterTarget(0);
	sbac->SetGameInterface(this);

	m_netLibrary->AddReliableHandler(
	"msgStateBag", [sbac](const char* data, size_t len)
	{
		sbac->HandlePacket(0, std::string_view{ data, len });
	},
	true);

	m_sbac = sbac;

	fx::ResourceManager::OnInitializeInstance.Connect([sbac](fx::ResourceManager* rm)
	{
		rm->SetComponent(sbac);
	});
}

void CloneManagerLocal::ProcessCreateAck(uint16_t objId, uint16_t uniqifier)
{
	if (icgi->NetProtoVersion >= 0x201912301309 && m_trackedObjects[objId].uniqifier != uniqifier)
	{
		Log("%s: invalid uniqifier for %d\n", __func__, objId);
		return;
	}

	m_trackedObjects[objId].lastSyncAck = msec();

	Log("%s: create ack %d\n", __func__, objId);
}

static hook::cdecl_stub<void(rage::netSyncTree*, rage::netObject*, uint8_t, uint16_t, uint32_t, int)> _processAck([]()
{
	return hook::get_pattern("45 32 ED FF 50 20 8B CB 41", -0x34);
});

void CloneManagerLocal::ProcessSyncAck(uint16_t objId, uint16_t uniqifier)
{
	if (icgi->NetProtoVersion >= 0x201912301309 && m_trackedObjects[objId].uniqifier != uniqifier)
	{
		Log("%s: invalid uniqifier for %d\n", __func__, objId);
		return;
	}

	auto netObjIt = m_savedEntities.find(objId);

	Log("%s: sync ack %d\n", __func__, objId);

	if (netObjIt != m_savedEntities.end())
	{
		auto netObj = netObjIt->second;

		if (netObj)
		{
			auto syncTree = netObj->GetSyncTree();
			syncTree->AckCfx(netObj, m_ackTimestamp);

			if (netObj->m_20())
			{
				_processAck(syncTree, netObj, 31, 0 /* seq? */, m_ackTimestamp, 0xFFFFFFFF);
			}
		}
	}
}

void CloneManagerLocal::ProcessRemoveAck(uint16_t objId, uint16_t uniqifier)
{
	auto sure = [this, objId]()
	{
		if (m_pendingConfirmObjectIds.find(objId) != m_pendingConfirmObjectIds.end())
		{
			ObjectIds_ConfirmObjectId(objId);

			m_pendingConfirmObjectIds.erase(objId);
		}
	};

	if (icgi->NetProtoVersion >= 0x202002271209)
	{
		if (uniqifier == 0)
		{
			auto s = m_pendingRemoveAcks.lower_bound({ objId, 0 });
			auto e = m_pendingRemoveAcks.upper_bound({ objId, INT32_MAX });

			m_pendingRemoveAcks.erase(s, e);

			sure();
		}
		else
		{
			m_pendingRemoveAcks.erase({ objId, uniqifier });

			sure();
		}

		return;
	}

	if (icgi->NetProtoVersion >= 0x201912301309 && m_trackedObjects[objId].uniqifier != uniqifier && uniqifier != 0)
	{
		Log("%s: invalid uniqifier for %d\n", __func__, objId);
		return;
	}

	// #NETVER: resend removes and handle acks here
	if (icgi->NetProtoVersion >= 0x201905190829)
	{
		auto s = m_pendingRemoveAcks.lower_bound({ objId, 0 });
		auto e = m_pendingRemoveAcks.upper_bound({ objId, INT32_MAX });

		m_pendingRemoveAcks.erase(s, e);
	}

	sure();
}

bool CloneManagerLocal::IsRemovingObjectId(uint16_t objectId)
{
	auto first = m_pendingRemoveAcks.lower_bound({ objectId, 0 });
	auto second = m_pendingRemoveAcks.lower_bound({ objectId, INT32_MAX });

	bool indeed = first != second;

	if (indeed)
	{
		m_pendingConfirmObjectIds.insert(objectId);
	}

	return indeed;
}

void CloneManagerLocal::ProcessTimestampAck(uint32_t timestamp)
{
	m_ackTimestamp = timestamp;

	Log("%s: ts ack %d\n", __func__, timestamp);
}

void CloneManagerLocal::HandleCloneAcks(const char* data, size_t len)
{
	net::Buffer buf(reinterpret_cast<const uint8_t*>(data), len);

	while (!buf.IsAtEnd())
	{
		auto type = buf.Read<uint8_t>();

		Log("%s: read ack type %d\n", __func__, type);

		switch (type)
		{
			// create ack?
			case 1:
			{
				auto objId = buf.Read<uint16_t>();
				ProcessCreateAck(objId);

				break;
			}
			// sync ack?
			case 2:
			{
				auto objId = buf.Read<uint16_t>();
				ProcessSyncAck(objId);

				break;
			}
			// timestamp ack?
			case 5:
			{
				auto timestamp = buf.Read<uint32_t>();
				ProcessTimestampAck(timestamp);

				break;
			}
			// remove ack?
			case 3:
			{
				auto objId = buf.Read<uint16_t>();
				ProcessRemoveAck(objId);

				break;
			}
			default:
				return;
		}
	}
}

void CloneManagerLocal::HandleCloneAcksNew(const char* data, size_t len)
{
	net::Buffer buffer(reinterpret_cast<const uint8_t*>(data), len);

	// dummy frame index
	buffer.Read<uint64_t>();

	uint8_t bufferData[16384] = { 0 };
	int bufferLength = LZ4_decompress_safe(reinterpret_cast<const char*>(&buffer.GetData()[buffer.GetCurOffset()]), reinterpret_cast<char*>(bufferData), buffer.GetRemainingBytes(), sizeof(bufferData));

	if (bufferLength > 0)
	{
		rl::MessageBuffer msgBuf(bufferData, bufferLength);

		bool end = false;

		while (!msgBuf.IsAtEnd() && !end)
		{
			auto type = msgBuf.Read<uint8_t>(3);

			Log("%s: read ack type %d\n", __func__, type);

			switch (type)
			{
				// create ack?
				case 1:
				{
					auto objId = msgBuf.Read<uint16_t>(13);
					auto uniqifier = 0;

					if (icgi->NetProtoVersion >= 0x201912301309)
					{
						uniqifier = msgBuf.Read<uint16_t>(16);
					}

					ProcessCreateAck(objId, uniqifier);

					break;
				}
				// sync ack?
				case 2:
				{
					auto objId = msgBuf.Read<uint16_t>(13);
					auto uniqifier = 0;

					if (icgi->NetProtoVersion >= 0x201912301309)
					{
						uniqifier = msgBuf.Read<uint16_t>(16);
					}

					ProcessSyncAck(objId, uniqifier);

					break;
				}
				// remove ack?
				case 3:
				{
					auto objId = msgBuf.Read<uint16_t>(13);
					auto uniqifier = 0;

					if (icgi->NetProtoVersion >= 0x201912301309)
					{
						uniqifier = msgBuf.Read<uint16_t>(16);
					}

					ProcessRemoveAck(objId, uniqifier);

					break;
				}
				// timestamp ack?
				case 5:
				{
					auto timestamp = msgBuf.Read<uint32_t>(32);
					ProcessTimestampAck(timestamp);

					break;
				}
				case 7:
				default:
					end = true;
					break;
			}
		}
	}
}

void CloneManagerLocal::AddCreateAck(uint16_t objectId)
{
	if (icgi->NetProtoVersion >= 0x202007022353)
	{
		return;
	}

	m_ackBuffer.Write(3, 1);
	m_ackBuffer.Write(13, objectId);

	AttemptFlushAckBuffer();
}

void CloneManagerLocal::AddRemoveAck(uint16_t objectId)
{
	if (icgi->NetProtoVersion >= 0x202007022353)
	{
		return;
	}

	m_ackBuffer.Write(3, 3);
	m_ackBuffer.Write(13, objectId);

	AttemptFlushAckBuffer();
}

class msgClone
{
public:
	msgClone();

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

	inline uint16_t GetUniqifier() const
	{
		return m_uniqifier;
	}

	//private:
	uint32_t m_handle;
	uint16_t m_clientId;
	uint16_t m_objectId;
	uint16_t m_uniqifier;
	NetObjEntityType m_entityType;
	uint8_t m_syncType;
	uint32_t m_timestamp;
	uint32_t m_creationToken;
	std::vector<uint8_t> m_cloneData;
};

msgClone::msgClone()
{
}

void msgClone::Read(int syncType, rl::MessageBuffer& buffer)
{
	m_syncType = syncType;
	m_objectId = buffer.Read<uint16_t>(13);
	m_clientId = buffer.Read<uint16_t>(16);

	if (syncType == 1)
	{
		m_entityType = (NetObjEntityType)buffer.Read<uint8_t>(4);
		m_creationToken = 0;

		if (icgi->NetProtoVersion >= 0x202002271209)
		{
			m_creationToken = buffer.Read<uint32_t>(32);
		}
	}

	if (icgi->NetProtoVersion >= 0x201912301309)
	{
		m_uniqifier = buffer.Read<uint16_t>(16);
	}
	else
	{
		m_uniqifier = 0;
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

	inline const std::vector<std::tuple<uint16_t, uint16_t, bool>>& GetRemoves() const
	{
		return m_removes;
	}

	inline uint64_t GetFrameIndex()
	{
		return m_frameIndex;
	}

private:
	uint64_t m_frameIndex;

	std::list<msgClone> m_clones;

	std::vector<std::tuple<uint16_t, uint16_t, bool>> m_removes;
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
				case 3: // clone remove
				{
					auto stillAlive = false;
					uint16_t uniqifier = 0;

					if (icgi->NetProtoVersion >= 0x202007120951)
					{
						stillAlive = msgBuf.ReadBit();
					}

					auto remove = msgBuf.Read<uint16_t>(13);

					if (icgi->NetProtoVersion >= 0x202007151853)
					{
						uniqifier = msgBuf.Read<uint16_t>(16);
					}

					m_removes.push_back({ remove, uniqifier, stillAlive });
					break;
				}
				case 5:
				{
					uint32_t msecLow = msgBuf.Read<uint32_t>(32);
					uint32_t msecHigh = msgBuf.Read<uint32_t>(32);

					uint64_t serverTime = ((uint64_t(msecHigh) << 32) | msecLow);
					//UpdateTime(serverTime);

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

extern std::map<int, int> g_creationTokenToObjectId;
extern std::map<int, uint32_t> g_objectIdToCreationToken;

rage::netObject* CloneManagerLocal::GetNetObject(uint16_t objectId)
{
	auto it = m_savedEntities.find(objectId);

	return (it != m_savedEntities.end()) ? it->second : nullptr;
}

rage::netObject* CloneManagerLocal::GetNetworkObject(uint16_t id)
{
	return GetNetObject(id);
}

bool CloneManagerLocal::HandleCloneCreate(const msgClone& msg)
{
	auto ackPacket = [&]()
	{
		// #NETVER: refactored ACKs
		if (icgi->NetProtoVersion >= 0x201905310838)
		{
			AddCreateAck(msg.GetObjectId());
		}
		else
		{
			// send ack
			net::Buffer outBuffer;
			outBuffer.Write<uint16_t>(msg.GetObjectId());

			m_netLibrary->SendReliableCommand("ccack", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());
		}
	};

	Log("%s: id %d obj [obj:%d] ts %d\n", __func__, msg.GetClientId(), msg.GetObjectId(), msg.GetTimestamp());

	if (m_pendingRemoveAcks.find({ msg.GetObjectId(), msg.GetUniqifier() }) != m_pendingRemoveAcks.end() || 
		m_pendingRemoveAcks.find({ msg.GetObjectId(), uint16_t(~msg.GetUniqifier()) }) != m_pendingRemoveAcks.end())
	{
		ackPacket();

		return true;
	}

	// create buffer
	rage::datBitBuffer rlBuffer(const_cast<uint8_t*>(msg.GetCloneData().data()), msg.GetCloneData().size());
	rlBuffer.m_f1C = 1;

	// skip if the player hasn't been created yet
	if (GetPlayerByNetId(msg.GetClientId()) == nullptr)
	{
		return false;
	}

	// get sync tree and read data
	auto syncTree = rage::netSyncTree::GetForType(msg.GetEntityType());
	syncTree->ReadFromBuffer(1, 0, &rlBuffer, nullptr);

	// find existence
	bool exists = false;

	CloneObjectMgr->ForAllNetObjects(31, [&](rage::netObject* object)
	{
		if (object->objectId == msg.GetObjectId())
		{
			exists = true;
		}
	});

	// already exists! bail out
	if (exists)
	{
		if (auto objectDataIt = m_trackedObjects.find(msg.GetObjectId()); objectDataIt == m_trackedObjects.end() || objectDataIt->second.uniqifier != msg.GetUniqifier())
		{
			Log("%s: duplicate remote object, undoing sync\n", __func__);

			ObjectIds_StealObjectId(msg.GetObjectId());
			g_dontParrotDeletionAcks.insert(msg.GetObjectId());
			DeleteObjectId(msg.GetObjectId(), true);

			return false;
		}

		// update client id if changed
		CheckMigration(msg);

		// hm
		Log("%s: tried to create a duplicate (remote) object\n", __func__);

		ackPacket();

		return true;
	}

	// check if the object already exists *locally*
	if (rage::netObjectMgr::GetInstance()->GetNetworkObject(msg.GetObjectId(), true) != nullptr)
	{
		if (auto objectDataIt = m_trackedObjects.find(msg.GetObjectId()); objectDataIt == m_trackedObjects.end() || objectDataIt->second.uniqifier != msg.GetUniqifier())
		{
			Log("%s: duplicate local object, undoing sync\n", __func__);

			ObjectIds_StealObjectId(msg.GetObjectId());
			g_dontParrotDeletionAcks.insert(msg.GetObjectId());
			DeleteObjectId(msg.GetObjectId(), true);

			return false;
		}

		// update client id if changed
		CheckMigration(msg);

		// continue
		Log("%s: tried to create a duplicate (local) object - [obj:%d]\n", __func__, msg.GetObjectId());

		ackPacket();

		return true;
	}

	m_extendedData[msg.GetObjectId()] = { msg.GetClientId() };

	auto& objectData = m_trackedObjects[msg.GetObjectId()];

	if (!objectData.stateBag)
	{
		objectData.stateBag = m_sbac->RegisterStateBag(fmt::sprintf("entity:%d", msg.GetObjectId()));
	}

	objectData.uniqifier = msg.GetUniqifier();

	// owner ID
	auto isRemote = (msg.GetClientId() != m_netLibrary->GetServerNetID());
	auto owner = isRemote ? 31 : GetLocalPlayer()->physicalPlayerIndex();

	// create the object
	auto obj = rage::CreateCloneObject(msg.GetEntityType(), msg.GetObjectId(), owner, 0, 32);

	if (!obj)
	{
		Log("%s: couldn't create object\n", __func__);

		return false;
	}

	obj->syncData.isRemote = isRemote;

	// check if we can apply
	if (!syncTree->CanApplyToObject(obj))
	{
		Log("%s: couldn't apply object\n", __func__);

		// delete the unapplied object
		delete obj;
		return false;
	}

	AssociateSyncTree(obj->objectId, syncTree);

	// apply object creation
	syncTree->ApplyToObject(obj, nullptr);

	// again, ensure it's not local
	if (obj->syncData.isRemote != isRemote || obj->syncData.ownerId != owner)
	{
		console::DPrintf("onesync", "Treason! Owner ID changed to %d.\n", obj->syncData.ownerId);
		Log("%s: Treason! Owner ID changed to %d.\n", __func__, obj->syncData.ownerId);
	}

	obj->syncData.isRemote = isRemote;
	obj->syncData.ownerId = owner;

	// register with object mgr
	rage::netObjectMgr::GetInstance()->RegisterNetworkObject(obj);

	// initialize blend
	if (obj->GetBlender())
	{
		obj->GetBlender()->SetTimestamp(msg.GetTimestamp());

		obj->m_1D0();

		obj->GetBlender()->ApplyBlend();
		obj->GetBlender()->m_38();
	}

	obj->m_1C0();

	// for the last time, ensure it's not local
	if (obj->syncData.isRemote != isRemote || obj->syncData.ownerId != owner)
	{
		console::DPrintf("onesync", "Treason (2)! Owner ID changed to %d.\n", obj->syncData.ownerId);
		Log("%s: Treason (2)! Owner ID changed to %d.\n", __func__, obj->syncData.ownerId);
	}

	obj->syncData.isRemote = isRemote;
	obj->syncData.ownerId = owner;

	// if this is owned by us, actually own the object now
	// (this is done late to make sure the logic is safe)
	if (msg.GetClientId() == m_netLibrary->GetServerNetID())
	{
		Log("%s: making obj %s our own\n", __func__, obj->ToString());

		// give us the object ID
		ObjectIds_AddObjectId(msg.GetObjectId());
	}

	if (msg.m_creationToken != 0)
	{
		g_creationTokenToObjectId[msg.m_creationToken] = msg.GetObjectId();
	}

	ackPacket();

	return true;
}

AckResult CloneManagerLocal::HandleCloneUpdate(const msgClone& msg)
{
	// create buffer
	rage::datBitBuffer rlBuffer(const_cast<uint8_t*>(msg.GetCloneData().data()), msg.GetCloneData().size());
	rlBuffer.m_f1C = 1;

	auto ackPacket = [&]()
	{
		// 		// send ack
		// 		net::Buffer outBuffer;
		// 		outBuffer.Write<uint16_t>(msg.GetObjectId());
		//
		// 		m_netLibrary->SendReliableCommand("csack", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());
	};

	Log("%s: id %d obj [obj:%d] ts %d\n", __func__, msg.GetClientId(), msg.GetObjectId(), msg.GetTimestamp());

	// get saved object
	auto objIt = m_savedEntities.find(msg.GetObjectId());

	if (objIt == m_savedEntities.end())
	{
		ackPacket();

		Log("%s: unknown obj?\n", __func__);

		if (m_pendingRemoveAcks.find({ msg.GetObjectId(), msg.GetUniqifier() }) != m_pendingRemoveAcks.end() || 
			m_pendingRemoveAcks.find({ msg.GetObjectId(), uint16_t(~msg.GetUniqifier()) }) != m_pendingRemoveAcks.end())
		{
			// hey, we're deleting this object, you don't know it yet, so you're giving it back to us as 'new'
			// we don't want it though, so we'll pretend it's all right :)
			return AckResult::OK;
		}

		return AckResult::ResendCreate;
	}

	// check uniqifier
	auto& objectData = m_trackedObjects[msg.GetObjectId()];

	if ((objectData.uniqifier != msg.GetUniqifier() && uint16_t(~objectData.uniqifier) != msg.GetUniqifier()) && icgi->NetProtoVersion >= 0x201912301309)
	{
		ackPacket();

		Log("%s: invalid object instance?\n", __func__);

		if (m_pendingRemoveAcks.find({ msg.GetObjectId(), msg.GetUniqifier() }) != m_pendingRemoveAcks.end() || m_pendingRemoveAcks.find({ msg.GetObjectId(), uint16_t(~msg.GetUniqifier()) }) != m_pendingRemoveAcks.end())
		{
			// yeah, yeah, we recreated this object by now, but you're trying to give us the old one
			return AckResult::OK;
		}

		return AckResult::ResendCreate;
	}

	auto& extData = m_extendedData[msg.GetObjectId()];
	auto obj = objIt->second;

	// if owned locally
	if (msg.GetClientId() == m_netLibrary->GetServerNetID()
		&& extData.clientId == msg.GetClientId() /* and this is not a migration */
		&& uint16_t(~objectData.uniqifier) != msg.GetUniqifier() /* and also not the first update for mA0 nodes */)
	{
		Log("%s: our object, bailing out\n", __func__);

		ackPacket();

		// our object, it's fine
		return AckResult::OK;
	}

	bool canApply = true;

	// is this a migration? if so, don't apply, aaaa!
	if (extData.clientId != msg.GetClientId())
	{
		canApply = false;
	}

	g_curNetObject = obj;

	if (msg.GetCloneData().size() && canApply)
	{
		// get sync tree and read data
		auto syncTree = obj->GetSyncTree();
		syncTree->ReadFromBuffer(2, 0, &rlBuffer, nullptr);

		// TODO: call m1E0

		if (!syncTree->CanApplyToObject(obj))
		{
			// couldn't apply, ignore ack

			Log("%s: couldn't apply object\n", __func__);

			return AckResult::ResendClone;
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
	}

	// update client id if changed
	// this has to be done AFTER apply since the sync update might contain critical state we didn't have yet!
	CheckMigration(msg);

	ackPacket();

	return AckResult::OK;
}

void CloneManagerLocal::CheckMigration(const msgClone& msg)
{
	auto objIt = m_savedEntities.find(msg.GetObjectId());
	rage::netObject* obj = nullptr;

	if (objIt != m_savedEntities.end())
	{
		obj = objIt->second;
	}

	auto& extData = m_extendedData[msg.GetObjectId()];

	if (extData.clientId != msg.GetClientId())
	{
		if (!obj)
		{
			Log("%s: No object by id [obj:%d] for migration :/\n", __func__, msg.GetObjectId());
			return;
		}

		Log("%s: Remote-migrating object %s (of type %s) from %s to %s.\n", __func__, obj->ToString(), GetType(obj),
		(g_playersByNetId[extData.clientId]) ? g_playersByNetId[extData.clientId]->GetName() : "(null)",
		(g_playersByNetId[msg.GetClientId()]) ? g_playersByNetId[msg.GetClientId()]->GetName() : "(null)");

		// reset next-owner ID as we've just migrated it
		obj->syncData.nextOwnerId = -1;
		extData.pendingClientId = -1;

		auto clientId = msg.GetClientId();

		if (clientId == m_netLibrary->GetServerNetID())
		{
			auto player = GetLocalPlayer();

			// add the object
			rage::netObjectMgr::GetInstance()->ChangeOwner(obj, player, 0);

			// if the game wants to delete it right away, let it.
			m_trackedObjects[obj->objectId].lastSyncTime = 1ms;
		}
		else
		{
			auto player = GetPlayerByNetId(clientId);

			if (player)
			{
				auto lastId = player->physicalPlayerIndex();
				player->physicalPlayerIndex() = 31;

				rage::netObjectMgr::GetInstance()->ChangeOwner(obj, player, 0);

				player->physicalPlayerIndex() = lastId;

				obj->syncData.isRemote = true;
				obj->syncData.nextOwnerId = -1;
			}
		}

		// this should happen AFTER AddObjectForPlayer, it verifies the object owner
		extData.clientId = clientId;
	}
}

net::Buffer g_cloneMsgPacket;
std::vector<uint8_t> g_cloneMsgData;

void CloneManagerLocal::HandleCloneSync(const char* data, size_t len)
{
	if (!rage::netObjectMgr::GetInstance())
	{
		return;
	}

	net::Buffer netBuffer(reinterpret_cast<const uint8_t*>(data), len);
	g_cloneMsgPacket = netBuffer.Clone();

	// read the packet
	msgPackedClones msg;
	msg.Read(netBuffer);

	static std::vector<uint16_t> ignoreList;
	static std::vector<uint16_t> recreateList;

	for (auto& clone : msg.GetClones())
	{
		g_cloneMsgData = clone.GetCloneData();

		switch (clone.GetSyncType())
		{
			case 1:
			{
				bool acked = HandleCloneCreate(clone);

				if (!acked && icgi->NetProtoVersion >= 0x202007022353)
				{
					ignoreList.push_back(clone.GetObjectId());
				}

				break;
			}
			case 2:
			{
				AckResult acked = HandleCloneUpdate(clone);

				if (acked != AckResult::OK)
				{
					if (acked == AckResult::ResendCreate && icgi->NetProtoVersion >= 0x202007022353)
					{
						recreateList.push_back(clone.GetObjectId());
					}
					else
					{
						ignoreList.push_back(clone.GetObjectId());
					}
				}

				break;
			}
		}
	}

	for (auto [ remove, uniqifier, stillAlive ] : msg.GetRemoves())
	{
		auto localUniq = -1;

		auto objectIt = m_trackedObjects.find(remove);

		if (objectIt != m_trackedObjects.end())
		{
			localUniq = objectIt->second.uniqifier;
		}

		Log("Trying to remove entity %d (sent uniq: %d, local uniq: %d, should steal: %s)\n", remove, uniqifier, localUniq, stillAlive ? "yes" : "no");

		// even if the server is referring to a different entity, they'll have stolen the object ID
		// excessive steals are not really a big issue, it just means that upon next *deletion* the object will go back
		// into the global pool
		if (stillAlive)
		{
			m_pendingConfirmObjectIds.erase(remove);
			ObjectIds_StealObjectId(remove);
		}

		{
			auto objectIt = m_trackedObjects.find(remove);

			if (objectIt != m_trackedObjects.end())
			{
				if (uniqifier && objectIt->second.uniqifier != uniqifier)
				{
					Log("Invalid uniqifier!\n");
					continue;
				}
			}
		}

		if (stillAlive)
		{
			auto objectIt = m_savedEntities.find(remove);

			if (objectIt != m_savedEntities.end())
			{
				auto object = objectIt->second;

				g_dontParrotDeletionAcks.insert(remove);
			}
		}

		DeleteObjectId(remove, false);
	}

	if (msg.GetFrameIndex() & (uint64_t(1) << 63) || icgi->NetProtoVersion < 0x202007022353)
	{
		net::Buffer outBuffer;
		outBuffer.Write<uint64_t>(msg.GetFrameIndex() & ~(uint64_t(1) << 63));
		outBuffer.Write<uint8_t>(uint8_t(ignoreList.size()));

		for (uint16_t entry : ignoreList)
		{
			outBuffer.Write<uint16_t>(entry);
		}

		if (icgi->NetProtoVersion >= 0x202007022353)
		{
			outBuffer.Write<uint8_t>(uint8_t(recreateList.size()));

			for (uint16_t entry : recreateList)
			{
				outBuffer.Write<uint16_t>(entry);
			}
		}

		Log("GSAck for frame index %d w/ %d ignore and %d rec\n", msg.GetFrameIndex() & ~(uint64_t(1) << 63), ignoreList.size(), recreateList.size());

		m_netLibrary->SendUnreliableCommand("gameStateAck", (const char*)outBuffer.GetData().data(), outBuffer.GetCurOffset());

		ignoreList.clear();
		recreateList.clear();
	}
}

void CloneManagerLocal::HandleCloneRemove(const char* data, size_t len)
{
	net::Buffer netBuffer(reinterpret_cast<const uint8_t*>(data), len);
	auto objectId = netBuffer.Read<uint16_t>();

	Log("%s: deleting [obj:%d]\n", __func__, objectId);

	DeleteObjectId(objectId, false);
}

void CloneManagerLocal::DeleteObjectId(uint16_t objectId, bool force)
{
	// find object and remove
	auto objectIt = m_savedEntities.find(objectId);

	if (objectIt != m_savedEntities.end())
	{
		auto object = objectIt->second;

		// don't allow removing the local player ped, that'll lead to a few issues
		if (object == GetLocalPlayerPedNetObject())
		{
			return;
		}

		// set flags
		object->syncData.wantsToDelete = true;
		object->syncData.shouldNotBeDeleted = false;

		// unack the create to unburden the game
		object->syncData.creationAckedPlayers &= ~(1 << 31);

		// call object manager clone removal
		rage::netObjectMgr::GetInstance()->UnregisterNetworkObject(object, 8, 0, 1);

		Log("%s: object ID [obj:%d]\n", __func__, objectId);
	}

	// #NETVER: refactored ACKs
	if (icgi->NetProtoVersion >= 0x201905310838 && !force)
	{
		AddRemoveAck(objectId);
	}
}

void CloneManagerLocal::SetTargetOwner(rage::netObject* object, uint16_t clientId)
{
	m_extendedData[object->objectId].pendingClientId = clientId;
}

void CloneManagerLocal::GiveObjectToClient(rage::netObject* object, uint16_t clientId)
{
	bool wasLocal = !object->syncData.isRemote;

	if (clientId == m_netLibrary->GetServerNetID() && !wasLocal)
	{
		// give us the object ID
		ObjectIds_AddObjectId(object->objectId);

		// store object data as being synced (so we don't have to send creation to the server)
		auto& objectData = m_trackedObjects[object->objectId];

		objectData.lastSyncTime = msec();
		objectData.lastSyncAck = msec();

		// this isn't remote anymore
		object->syncData.isRemote = false;
		object->syncData.nextOwnerId = -1;
	}

	// TODO: rate-limit resends (in case pending ownership is taking really long)

	m_sendBuffer.Write(3, 4);
	m_sendBuffer.Write(16, clientId); // client ID
	//m_sendBuffer.Write<uint8_t>(0); // player ID (byte)
	m_sendBuffer.Write(13, object->objectId);

	AttemptFlushCloneBuffer();

	Log("%s: Migrating object %s (of type %s) from %s to %s (%s).\n", __func__, object->ToString(), GetType(object),
	wasLocal ? "us" : "a remote player",
	(g_playersByNetId[clientId]) ? g_playersByNetId[clientId]->GetName() : "(null)",
	(clientId == m_netLibrary->GetServerNetID()) ? "us!" : "remote player");
}

const std::vector<rage::netObject*>& CloneManagerLocal::GetObjectList()
{
	return m_savedEntityVec;
}

void* origin;
float* (*getCoordsFromOrigin)(void*, float*);

static HookFunction hookFunctionOrigin([]()
{
	auto loc = hook::get_call(hook::get_pattern<char>("C6 45 0B 80 89 5D 0F", 0x1B));
	origin = hook::get_address<void*>(loc + 0xC);
	hook::set_call(&getCoordsFromOrigin, loc + 0x10);
});

void CloneManagerLocal::Update()
{
	WriteUpdates();

	SendUpdates(m_sendBuffer, HashString("netClones"));

	if (icgi->NetProtoVersion < 0x202007022353)
	{
		SendUpdates(m_ackBuffer, HashString("netAcks"));
	}

	alignas(16) float centerOfWorld[4];
	getCoordsFromOrigin(origin, centerOfWorld);

	auto origin = DirectX::XMVectorSet(centerOfWorld[0], centerOfWorld[1], centerOfWorld[2], 1.0f);
	static uint32_t frameCount = 0;

	// run Update() on all clones
	for (auto& clone : m_savedEntities)
	{
		if (clone.second)
		{
			clone.second->Update();

			if (clone.second->GetGameObject())
			{
				if (clone.second->syncData.isRemote)
				{
					auto ent = (fwEntity*)(clone.second->GetGameObject());
					auto vtbl = *(char**)ent;

					if (!Is2060())
					{
						auto posData = ent->GetPosition();
						auto pos = DirectX::XMLoadFloat3(&posData);

						static std::map<fwEntity*, uint32_t> removedFlags;
						auto it = removedFlags.find(ent);

						if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(pos, origin))) < (300.f * 300.f))
						{
							if (it != removedFlags.end())
							{
								// 1604
								//((void(*)(void*))(*(uintptr_t*)(vtbl + 0x260)))(ent);

								// 1604, rage::fwSceneUpdate::AddToSceneUpdate
								((void (*)(void*, uint32_t))0x1415F2EDC)(ent, it->second);

								removedFlags.erase(it);
							}
						}
						else
						{
							if (it == removedFlags.end())
							{
								uint32_t flags = 0;

								if (auto ext = ent->GetExtension<fwSceneUpdateExtension>())
								{
									flags = ext->GetUpdateFlags();
								}

								it = removedFlags.emplace(ent, flags).first;

								// 1604
								//((void(*)(void*))(*(uintptr_t*)(vtbl + 0x268)))(ent);

								// 1604, rage::fwSceneUpdate::RemoveFromSceneUpdate
								((void (*)(void*, uint32_t, bool))0x1415F64EC)(ent, -1, true);
							}

							if ((frameCount % 50) < 2)
							{
								// 1604
								//((void(*)(void*))(*(uintptr_t*)(vtbl + 0x260)))(ent);

								// 1604, rage::fwSceneUpdate::AddToSceneUpdate
								((void (*)(void*, uint32_t))0x1415F2EDC)(ent, it->second);

								removedFlags.erase(it);
							}
						}
					}
				}

				clone.second->UpdatePendingVisibilityChanges();
			}
		}
	}

	frameCount++;
}

bool CloneManagerLocal::RegisterNetworkObject(rage::netObject* object)
{
	if (m_savedEntities.find(object->objectId) != m_savedEntities.end())
	{
		// TODO: delete it somewhen?
		Log("%s: duplicate object ID %s\n", __func__, object->ToString());
		console::DPrintf("onesync", "%s: duplicate object ID %s\n", __func__, object->ToString());

		return false;
	}

	m_trackedObjects[object->objectId].hi = true;

	if (!m_trackedObjects[object->objectId].stateBag)
	{
		m_trackedObjects[object->objectId].stateBag = m_sbac->RegisterStateBag(fmt::sprintf("entity:%d", object->objectId));
	}

	Log("%s: registering %s (uniqifier: %d)\n", __func__, object->ToString(), m_trackedObjects[object->objectId].uniqifier);

	if (object->syncData.ownerId != 0xFF)
	{
		m_netObjects[object->syncData.ownerId][object->objectId] = object;

		if (object->syncData.ownerId != 31)
		{
			m_extendedData[object->objectId].clientId = m_netLibrary->GetServerNetID();

			int delay = 0;

			// don't send peds unless they're staying
			if (object->objectType == (int)NetObjEntityType::Ped)
			{
				delay = 75;
			}

			m_extendedData[object->objectId].dontSyncBefore = (*rage__s_NetworkTimeThisFrameStart) + delay;
		}
	}

	m_savedEntities[object->objectId] = object;
	m_savedEntitySet.insert(object);
	m_savedEntityVec.push_back(object);

	return true;
}

void CloneManagerLocal::DestroyNetworkObject(rage::netObject* object)
{
	Log("%s: unregistering %s\n", __func__, object->ToString());

	if (g_curNetObjectSelection == object)
	{
		g_curNetObjectSelection = nullptr;
	}

	for (auto& objectList : m_netObjects)
	{
		objectList.erase(object->objectId);
	}

	// these are not actually to be deleted, don't ask the server to delete them
	if (g_dontParrotDeletionAcks.find(object->objectId) == g_dontParrotDeletionAcks.end())
	{
		if (m_trackedObjects[object->objectId].lastSyncTime != 0ms)
		{
			m_pendingRemoveAcks.insert({ { object->objectId, m_trackedObjects[object->objectId].uniqifier }, msec() });
		}
	}

	g_dontParrotDeletionAcks.erase(object->objectId);
	m_savedEntities.erase(object->objectId);
	m_savedEntitySet.erase(object);
	m_trackedObjects.erase(object->objectId);
	m_extendedData.erase(object->objectId);

	m_savedEntityVec.erase(std::remove(m_savedEntityVec.begin(), m_savedEntityVec.end(), object), m_savedEntityVec.end());
}

void CloneManagerLocal::ChangeOwner(rage::netObject* object, CNetGamePlayer* player, int migrationType)
{
	if (object->syncData.ownerId != player->physicalPlayerIndex())
	{
		GiveObjectToClient(object, g_netIdsByPlayer[player]);
	}

	m_netObjects[object->syncData.ownerId].erase(object->objectId);
	m_netObjects[player->physicalPlayerIndex()][object->objectId] = object;
}

static hook::cdecl_stub<bool(const Vector3* position, float radius, float maxDistance, CNetGamePlayer** firstPlayer)> _isSphereVisibleForAnyPlayer([]()
{
	return hook::get_call(hook::get_pattern("0F 29 4C 24 30 0F 28 C8 E8", 8));
});

static hook::cdecl_stub<void(rage::netObjectMgr*, rage::netObject*)> _processRemoveAck([]()
{
	return hook::get_pattern("39 42 74 75 12 39 42 70 75 0D", -0x11);
});

static hook::thiscall_stub<bool(void*)> fwEntity_IsInScene([]()
{
	return hook::get_pattern("74 12 F6 41 40 01 75 0A 48", -5);
});

void CloneManagerLocal::WriteUpdates()
{
	auto objectMgr = rage::netObjectMgr::GetInstance();

	if (!objectMgr)
	{
		return;
	}

	int syncCount1 = 0, syncCount2 = 0, syncCount3 = 0, syncCount4 = 0;

	bool hitTimestamp = false;

	auto ts = *rage__s_NetworkTimeLastFrameStart;

	auto touchTimestamp = [&hitTimestamp, ts, this]()
	{
		if (hitTimestamp)
		{
			return;
		}

		uint32_t timestamp = ts;

		m_sendBuffer.Write(3, 5);
		m_sendBuffer.Write(32, timestamp);

		hitTimestamp = true;
	};

	// on each object...
	auto objectCb = [&](rage::netObject* object)
	{
		// skip remote objects
		if (object->syncData.isRemote)
		{
			if (m_extendedData[object->objectId].clientId == m_netLibrary->GetServerNetID())
			{
				console::DPrintf("onesync", "%s: got a remote object (%s) that's meant to be ours. telling the server so again.\n", __func__, object->ToString());
				Log("%s: got a remote object (%s) that's meant to be ours. telling the server so again.\n", __func__, object->ToString());

				GiveObjectToClient(object, m_netLibrary->GetServerNetID());

				m_extendedData[object->objectId].clientId = -1;
			}

			return;
		}

		if (object->syncData.ownerId == 31)
		{
			return;
		}

		if (object->syncData.nextOwnerId != 0xFF)
		{
			GiveObjectToClient(object, m_extendedData[object->objectId].pendingClientId);
		}

		// don't sync created entities for the initial part of their life
		if (*rage__s_NetworkTimeThisFrameStart < m_extendedData[object->objectId].dontSyncBefore)
		{
			return;
		}

		// don't sync netobjs that aren't in the scene
		// #TODO1S: remove netobjs from the server if they're removed from the scene?
		if (object->GetGameObject())
		{
			if (!fwEntity_IsInScene(object->GetGameObject()))
			{
				return;
			}
		}

		// get basic object data
		auto objectType = object->objectType;
		auto objectId = object->objectId;

		// store a reference to the object tracking data
		auto& objectData = m_trackedObjects[objectId];

		// allocate a RAGE buffer
		uint8_t packetStub[1200] = { 0 };
		rage::datBitBuffer rlBuffer(packetStub, sizeof(packetStub));

		// if we want to delete this object
		if (object->syncData.wantsToDelete)
		{
			// has this been acked by client 31?
			if (object->syncData.IsCreationAckedByPlayer(31))
			{
				/*auto& netBuffer = m_sendBuffer;

				++syncCount3;

				netBuffer.Write(3, 3);
				//netBuffer.Write<uint8_t>(0); // player ID (byte)
				netBuffer.Write(13, object->objectId); // object ID (short)

				AttemptFlushNetBuffer();

				Log("%s: telling server %d is deleted\n", __func__, object->objectId);*/

				// unack the create to unburden the game
				object->syncData.creationAckedPlayers &= ~(1 << 31);

				// pretend to ack the remove to process removal
				_processRemoveAck(objectMgr, object);
			}

			// don't actually continue sync
			return;
		}

		// if this object doesn't have a game object, but it should, ignore it
		if (object->objectType != (uint16_t)NetObjEntityType::PickupPlacement)
		{
			if (object->GetGameObject() == nullptr)
			{
				return;
			}
		}

		// get the sync tree
		auto syncTree = object->GetSyncTree();

		// get latency stuff
		auto syncLatency = 50ms;

		if (object->GetGameObject())
		{
			auto entity = (fwEntity*)object->GetGameObject();
			auto entityPos = entity->GetPosition();

			if (!_isSphereVisibleForAnyPlayer(&entityPos, entity->GetRadius(), 250.0f, nullptr))
			{
				syncLatency = 250ms;
			}
		}

		// players get instant sync
		if (object->objectType == (uint16_t)NetObjEntityType::Player)
		{
			syncLatency = 0ms;
		}
		// player-occupied vehicles do as well
		else if (object->GetGameObject() && ((fwEntity*)object->GetGameObject())->IsOfType(HashString("CVehicle")))
		{
			auto vehicle = (CVehicle*)object->GetGameObject();
			auto seatManager = vehicle->GetSeatManager();

			for (int i = 0; i < seatManager->GetNumSeats(); i++)
			{
				auto occupant = seatManager->GetOccupant(i);

				if (occupant)
				{
					auto netObject = reinterpret_cast<rage::netObject*>(occupant->GetNetObject());

					if (netObject && netObject->objectType == (uint16_t)NetObjEntityType::Player)
					{
						syncLatency = 0ms;
						break;
					}
				}
			}
		}

		// #TODO1S: make this dynamic based on net_targetRate
		//syncLatency = std::max(syncLatency, 20ms);

		// determine sync type from sync data
		int syncType = 0;

		if (objectData.lastSyncTime == 0ms)
		{
			// clone create
			syncType = 1;
		}
		else if ((msec() - objectData.lastSyncTime) >= syncLatency)
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
			g_curNetObject = object;

			uint32_t lastChangeTime;

			bool shouldTrySend = syncTree->WriteTreeCfx(syncType, (syncType == 2 || syncType == 4) ? 1 : 0, object, &rlBuffer, ts, nullptr, 31, nullptr, &lastChangeTime);

			if (!shouldTrySend && icgi->NetProtoVersion >= 0x202007022353)
			{
				if (ts >= objectData.nextKeepaliveSync)
				{
					rlBuffer = { packetStub, sizeof(packetStub) };
					shouldTrySend = true;
				}
			}

			if (shouldTrySend)
			{
				// #TODO1S: dynamic resend time based on latency
				bool shouldWrite = true;

				if (lastChangeTime == objectData.lastChangeTime && ts < (objectData.lastResendTime + 100))
				{
					Log("%s: no early resend of object [obj:%d]\n", __func__, objectId);
					shouldWrite = false;
				}

				if (shouldWrite)
				{
					objectData.nextKeepaliveSync = ts + 1000;
					objectData.lastChangeTime = lastChangeTime;

					AssociateSyncTree(object->objectId, syncTree);

					// instantly mark player 31 as acked
					if (object->m_20())
					{
						// 1290
						//((void(*)(rage::netSyncTree*, rage::netObject*, uint8_t, uint16_t, uint32_t, int))0x1415D94F0)(syncTree, object, 31, 0 /* seq? */, 0x7FFFFFFF, 0xFFFFFFFF);
					}

					// touch the timestamp
					touchTimestamp();

					// write header to send buffer
					netBuffer.Write(3, syncType);

					if (icgi->NetProtoVersion >= 0x201912301309)
					{
						netBuffer.Write(16, objectData.uniqifier);
					}

					// write data
					//netBuffer.Write<uint8_t>(getPlayerId()); // player ID (byte)
					//netBuffer.Write<uint8_t>(0); // player ID (byte)
					netBuffer.Write(13, objectId); // object ID (short)

					if (syncType == 1)
					{
						if (icgi->NetProtoVersion >= 0x202002271209)
						{
							netBuffer.Write(32, g_objectIdToCreationToken[objectId]);
						}

						netBuffer.Write(4, objectType);
					}

					//netBuffer.Write<uint32_t>(rage::netInterface_queryFunctions::GetInstance()->GetTimestamp()); // timestamp?

					uint32_t len = rlBuffer.GetDataLength();
					netBuffer.Write(12, len); // length (short)

					if (len > 0)
					{
						netBuffer.WriteBits(rlBuffer.m_data, len * 8); // data
					}

					Log("uncompressed clone sync for [obj:%d]: %d bytes\n", objectId, len);

					AttemptFlushCloneBuffer();

					objectData.lastResendTime = ts;
					objectData.lastSyncTime = msec();
				}
			}
		}

		/*		m_savedEntities[objectId] = object;
		m_savedEntitySet.insert(object);

		if (m_extendedData[objectId].clientId != m_netLibrary->GetServerNetID())
		{
			Log("changing object %d netid from %d to %d%s\n",
				objectId,
				m_extendedData[objectId].clientId,
				m_netLibrary->GetServerNetID(),
				m_extendedData[objectId].clientId != 0 ? "... already initialized?" : "");

			m_extendedData[objectId].clientId = m_netLibrary->GetServerNetID();

			objectData.lastSyncTime = msec();
			objectData.lastSyncAck = msec();
		}*/
	};

	for (auto& list : m_netObjects)
	{
		// since the list may get mutated, store it temporarily
		static rage::netObject* objects[8192];
		int objIdx = 0;

		for (auto& object : list)
		{
			objects[objIdx++] = object.second;
		}

		for (int i = 0; i < objIdx; i++)
		{
			objectCb(objects[i]);
		}
	}

	auto t = msec();

	for (auto& pair : m_pendingRemoveAcks)
	{
		auto [objectIdRef, nextTime] = pair;

		auto [objectId, uniqifier] = objectIdRef;

		if (t < nextTime)
		{
			continue;
		}

		auto& netBuffer = m_sendBuffer;

		netBuffer.Write(3, 3);
		netBuffer.Write(13, objectId); // object ID (short)

		if (icgi->NetProtoVersion >= 0x202002271209)
		{
			netBuffer.Write(16, uniqifier);
		}

		AttemptFlushCloneBuffer();

		pair.second = t + 150ms;
	}

	// #NETVER: older servers won't ack removes, so we don't try resending removals ever
	if (icgi->NetProtoVersion < 0x201905190829)
	{
		m_pendingRemoveAcks.clear();
	}

	Log("sync: got %d creates, %d syncs, %d removes and %d migrates\n", syncCount1, syncCount2, syncCount3, syncCount4);
}

void CloneManagerLocal::AttemptFlushCloneBuffer()
{
	AttemptFlushNetBuffer(m_sendBuffer, HashString("netClones"));
}

void CloneManagerLocal::AttemptFlushAckBuffer()
{
	if (icgi->NetProtoVersion < 0x202007022353)
	{
		AttemptFlushNetBuffer(m_ackBuffer, HashString("netAcks"));
	}
}

void CloneManagerLocal::AttemptFlushNetBuffer(rl::MessageBuffer& buffer, uint32_t msgType)
{
	// flush the send buffer in case it could compress to >1100 bytes
	if (LZ4_compressBound(buffer.GetDataLength()) > 1100)
	{
		SendUpdates(buffer, msgType);
	}
}

void CloneManagerLocal::SendUpdates(rl::MessageBuffer& buffer, uint32_t msgType)
{
	auto lastSendVar = (msgType == HashString("netClones")) ? &m_lastSend : &m_lastAck;

	if (buffer.GetDataLength() > 0)
	{
		buffer.Write(3, 7);

		// compress and send data
		std::vector<char> outData(LZ4_compressBound(buffer.GetDataLength()) + 4);
		int len = LZ4_compress_default(reinterpret_cast<const char*>(buffer.GetBuffer().data()), outData.data() + 4, buffer.GetDataLength(), outData.size() - 4);

		Log("compressed %d bytes to %d bytes\n", buffer.GetDataLength(), len);

		*(uint32_t*)(outData.data()) = msgType;
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

		buffer.SetCurrentBit(0);
		*lastSendVar = msec();
	}
}

CloneManagerLocal g_cloneMgr;
}

sync::CloneManager* TheClones = &sync::g_cloneMgr;
sync::INetObjMgrAbstraction* CloneObjectMgr = &sync::g_cloneMgr;
