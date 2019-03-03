#pragma once

#include <Client.h>

#include <ServerInstanceBase.h>

#include <state/Pool.h>

#include <state/RlMessageBuffer.h>

#include <variant>

#include <array>
#include <EASTL/bitset.h>

#include <tbb/concurrent_unordered_map.h>
#include <tbb/task_group.h>

namespace fx
{
struct ScriptGuid;
}

namespace fx::sync
{
struct SyncEntityState;

struct SyncParseState
{
	rl::MessageBuffer buffer;
	int syncType;
	int objType;

	std::shared_ptr<SyncEntityState> entity;

	uint64_t frameIndex;
};

struct SyncUnparseState
{
	rl::MessageBuffer& buffer;
	int syncType;
	int objType;

	std::shared_ptr<Client> client;

	SyncUnparseState(rl::MessageBuffer& buffer)
		: buffer(buffer)
	{

	}
};

struct NodeBase;

using SyncTreeVisitor = std::function<bool(NodeBase&)>;

struct NodeBase
{
public:
	eastl::bitset<256> ackedPlayers;

	uint64_t frameIndex;

	virtual bool Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual bool Visit(const SyncTreeVisitor& visitor) = 0;
};

struct CPlayerCameraNodeData
{
	int camMode;
	float freeCamPosX;
	float freeCamPosY;
	float freeCamPosZ;

	float cameraX;
	float cameraZ;

	float camOffX;
	float camOffY;
	float camOffZ;
};

struct CPedGameStateNodeData
{
	int curVehicle;
	int curVehicleSeat;

	int lastVehicle;
	int lastVehicleSeat;

	inline CPedGameStateNodeData()
		: lastVehicle(-1), lastVehicleSeat(-1)
	{

	}
};

struct CVehicleGameStateNodeData
{
	uint16_t occupants[32];
	eastl::bitset<32> playerOccupants;

	inline CVehicleGameStateNodeData()
	{
		memset(occupants, 0, sizeof(occupants));
	}
};

enum ePopType
{
	POPTYPE_UNKNOWN = 0,
	POPTYPE_RANDOM_PERMANENT,
	POPTYPE_RANDOM_PARKED,
	POPTYPE_RANDOM_PATROL,
	POPTYPE_RANDOM_SCENARIO,
	POPTYPE_RANDOM_AMBIENT,
	POPTYPE_PERMANENT,
	POPTYPE_MISSION,
	POPTYPE_REPLAY,
	POPTYPE_CACHE,
	POPTYPE_TOOL
};

struct SyncTreeBase
{
public:
	virtual ~SyncTreeBase() = default;

	virtual void Parse(SyncParseState& state) = 0;

	virtual bool Unparse(SyncUnparseState& state) = 0;

	virtual void Visit(const SyncTreeVisitor& visitor) = 0;

	// accessors
public:
	virtual void GetPosition(float* posOut) = 0;

	virtual CPlayerCameraNodeData* GetPlayerCamera() = 0;

	virtual CPedGameStateNodeData* GetPedGameState() = 0;

	virtual CVehicleGameStateNodeData* GetVehicleGameState() = 0;

	virtual bool GetPopulationType(ePopType* popType) = 0;

	virtual bool GetModelHash(uint32_t* modelHash) = 0;

	virtual bool GetScriptHash(uint32_t* scriptHash) = 0;
};

enum class NetObjEntityType
{
	Automobile = 0,
	Bike = 1,
	Boat = 2,
	Door = 3,
	Heli = 4,
	Object = 5,
	Ped = 6,
	Pickup = 7,
	PickupPlacement = 8,
	Plane = 9,
	Submarine = 10,
	Player = 11,
	Trailer = 12,
	Train = 13
};

struct SyncEntityState
{
	using TData = std::variant<int, float, bool, std::string>;

	tbb::concurrent_unordered_map<std::string, TData> data;
	std::weak_ptr<fx::Client> client;
	NetObjEntityType type;
	eastl::bitset<256> ackedCreation;
	eastl::bitset<256> didDeletion;
	uint32_t timestamp;
	uint64_t frameIndex;
	uint64_t lastFrameIndex;

	std::array<std::chrono::milliseconds, 256> lastResends{};
	std::array<std::chrono::milliseconds, 256> lastSyncs{};

	std::unique_ptr<SyncTreeBase> syncTree;

	template<typename T>
	inline T GetData(std::string_view key, T defaultVal)
	{
		auto it = data.find(std::string(key));

		try
		{
			return (it != data.end()) ? std::get<T>(it->second) : defaultVal;
		}
		catch (std::bad_variant_access&)
		{
			return defaultVal;
		}
	}

	inline void CalculatePosition()
	{
		auto sectorX = GetData<int>("sectorX", 512);
		auto sectorY = GetData<int>("sectorY", 512);
		auto sectorZ = GetData<int>("sectorZ", 24);

		auto sectorPosX = GetData<float>("sectorPosX", 0.0f);
		auto sectorPosY = GetData<float>("sectorPosY", 0.0f);
		auto sectorPosZ = GetData<float>("sectorPosZ", 44.0f);

		data["posX"] = ((sectorX - 512.0f) * 54.0f) + sectorPosX;
		data["posY"] = ((sectorY - 512.0f) * 54.0f) + sectorPosY;
		data["posZ"] = ((sectorZ * 69.0f) + sectorPosZ) - 1700.0f;
	}

	ScriptGuid* guid;
	uint32_t handle;

	virtual ~SyncEntityState();
};
}

namespace fx
{
struct ScriptGuid
{
	enum class Type
	{
		Entity,
		TempEntity
	};

	Type type;
	union
	{
		struct
		{
			uint32_t handle;
		} entity;

		struct
		{
			uint32_t creationToken;
		} tempEntity;
	};
};

class ServerGameState : public fwRefCountable, public fx::IAttached<fx::ServerInstanceBase>
{
public:
	ServerGameState();

	void Tick(fx::ServerInstanceBase* instance);

	void UpdateWorldGrid(fx::ServerInstanceBase* instance);

	void UpdateEntities();

	void ParseGameStatePacket(const std::shared_ptr<fx::Client>& client, const std::vector<uint8_t>& packetData);

	virtual void AttachToObject(fx::ServerInstanceBase* instance) override;

	void HandleClientDrop(const std::shared_ptr<fx::Client>& client);

	void SendObjectIds(const std::shared_ptr<fx::Client>& client, int numIds);

	void RemoveEntity(uint32_t entityHandle);

	void ReassignEntity(uint32_t entityHandle, const std::shared_ptr<fx::Client>& targetClient);

private:
	void ProcessCloneCreate(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneRemove(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneSync(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, net::Buffer& ackPacket);

	void ProcessCloneTakeover(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket);

	void ProcessClonePacket(const std::shared_ptr<fx::Client>& client, rl::MessageBuffer& inPacket, int parsingType, uint16_t* outObjectId);

	void OnCloneRemove(const std::shared_ptr<sync::SyncEntityState>& entity);

public:
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint8_t playerId, uint16_t objectId);
	std::shared_ptr<sync::SyncEntityState> GetEntity(uint32_t handle);

private:
	fx::ServerInstanceBase* m_instance;

	std::unique_ptr<tbb::task_group> m_tg;

	// as bitset is not thread-safe
	std::mutex m_objectIdsMutex;

	eastl::bitset<8192> m_objectIdsSent;
	eastl::bitset<8192> m_objectIdsUsed;
	eastl::bitset<8192> m_objectIdsStolen;

	uint64_t m_frameIndex;

	struct WorldGridEntry
	{
		uint8_t sectorX;
		uint8_t sectorY;
		uint8_t slotID;

		WorldGridEntry()
		{
			sectorX = 0;
			sectorY = 0;
			slotID = -1;
		}
	};

	struct WorldGridState
	{
		WorldGridEntry entries[12];
	};

	WorldGridState m_worldGrid[256];

	struct WorldGridOwnerIndexes
	{
		uint8_t slots[256][256];

		inline WorldGridOwnerIndexes()
		{
			memset(slots, 0xFF, sizeof(slots));
		}
	};

	WorldGridOwnerIndexes m_worldGridAccel;

	void SendWorldGrid(void* entry = nullptr, const std::shared_ptr<fx::Client>& client = {});

//private:
public:
	std::vector<std::weak_ptr<sync::SyncEntityState>> m_entitiesById;

	std::list<std::shared_ptr<sync::SyncEntityState>> m_entityList;
	std::shared_mutex m_entityListMutex;
};

std::unique_ptr<sync::SyncTreeBase> MakeSyncTree(sync::NetObjEntityType objectType);
}

DECLARE_INSTANCE_TYPE(fx::ServerGameState);

extern CPool<fx::ScriptGuid>* g_scriptHandlePool;
